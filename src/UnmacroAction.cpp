#include "UnmacroAction.h"
#include "UnmacroPPCallbacks.h"
#include "BodyVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include <iostream>

using namespace clang;

UnmacroAction::UnmacroAction(llvm::StringRef MacroName, bool Inplace, bool RemoveExtraStatements)
    : TargetMacroName(MacroName), Inplace(Inplace), RemoveExtraStatements(RemoveExtraStatements) {}

std::unique_ptr<ASTConsumer> UnmacroAction::CreateASTConsumer(CompilerInstance &CI, llvm::StringRef file) {
    CI.getPreprocessor().addPPCallbacks(std::make_unique<UnmacroPPCallbacks>(
        Expansions, TargetMacroName, CI.getPreprocessor()));
    return std::make_unique<ASTConsumer>();
}

void UnmacroAction::EndSourceFileAction() {
    CompilerInstance &CI = getCompilerInstance();
    if (!CI.hasASTContext()) return;

    ASTContext &Context = CI.getASTContext();
    SourceManager &SM = Context.getSourceManager();

    std::set<unsigned> DangerousOffsets;
    std::vector<SourceRange> EmptyStmtRanges;
    BodyVisitor Visitor(DangerousOffsets, EmptyStmtRanges, Expansions, SM, Context.getLangOpts(), RemoveExtraStatements);
    Visitor.TraverseAST(Context);

    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SM, Context.getLangOpts());

    std::set<unsigned> RemovedOffsets;

    if (RemoveExtraStatements) {
        for (const auto &Range : EmptyStmtRanges) {
            TheRewriter.RemoveText(Range);
            // Mark the start of the removed structure
            RemovedOffsets.insert(SM.getFileOffset(Range.getBegin()));
        }
    }

    for (const auto &Info : Expansions) {
        unsigned Offset = SM.getFileOffset(Info.Range.getBegin());

        // Check if this macro expansion was part of a removed empty structure
        bool alreadyRemoved = false;
        for (const auto &Range : EmptyStmtRanges) {
             if (SM.isBeforeInTranslationUnit(Range.getBegin(), Info.Range.getBegin()) &&
                 SM.isBeforeInTranslationUnit(Info.Range.getEnd(), Range.getEnd())) {
                 alreadyRemoved = true;
                 break;
             }
        }
        if (alreadyRemoved) continue;

        bool isDangerous = DangerousOffsets.count(Offset);

        if (isDangerous) {
            if (Info.ExternalSemiLoc.isValid()) {
                TheRewriter.RemoveText(Info.Range);
            } else {
                TheRewriter.ReplaceText(Info.Range, ";");
            }
        } else {
            if (!Info.InternalSemi && Info.ExternalSemiLoc.isValid()) {
                TheRewriter.RemoveText(SourceRange(Info.Range.getBegin(), Info.ExternalSemiLoc));
            } else {
                TheRewriter.RemoveText(Info.Range);
            }
        }
    }

    if (TheRewriter.buffer_begin() != TheRewriter.buffer_end()) {
        if (Inplace) {
            TheRewriter.overwriteChangedFiles();
        } else {
            TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
        }
    } else if (!Inplace) {
        bool Invalid = false;
        llvm::StringRef Data = SM.getBufferData(SM.getMainFileID(), &Invalid);
        if (!Invalid) llvm::outs() << Data;
    }
}
