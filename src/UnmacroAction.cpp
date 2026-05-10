#include "UnmacroAction.h"
#include "UnmacroPPCallbacks.h"
#include "BodyVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include <iostream>

using namespace clang;

UnmacroAction::UnmacroAction(const std::vector<std::string> &MacroNames, bool Inplace, bool RemoveExtraStatements)
    : TargetMacroNames(MacroNames), Inplace(Inplace), RemoveExtraStatements(RemoveExtraStatements) {}

std::unique_ptr<ASTConsumer> UnmacroAction::CreateASTConsumer(CompilerInstance &CI, llvm::StringRef file) {
    CI.getPreprocessor().addPPCallbacks(std::make_unique<UnmacroPPCallbacks>(
        Expansions, TargetMacroNames, CI.getPreprocessor()));
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
            CharSourceRange CRange = CharSourceRange::getTokenRange(Range);
            // StringRef Text = Lexer::getSourceText(CRange, SM, Context.getLangOpts());
            // llvm::errs() << "[Action] Removing empty stmt: " << SM.getFileOffset(Range.getBegin()) << "-" << SM.getFileOffset(Range.getEnd()) << " text: \"" << Text << "\"\n";
            TheRewriter.RemoveText(CRange);
            // Mark the start of the removed structure
            RemovedOffsets.insert(SM.getFileOffset(Range.getBegin()));
        }
    }

    for (const auto &Info : Expansions) {
        unsigned Offset = SM.getFileOffset(Info.Range.getBegin());
        // unsigned EndOffset = SM.getFileOffset(Info.Range.getEnd());

        // Check if this macro expansion was part of a removed empty structure
        bool alreadyRemoved = false;
        for (const auto &Range : EmptyStmtRanges) {
             // SM.isBeforeInTranslationUnit(A, B) is true if A < B
             // Equality: !isBefore(A, B) && !isBefore(B, A)

             bool StartOk = !SM.isBeforeInTranslationUnit(Info.Range.getBegin(), Range.getBegin()); // Info.Start >= Range.Start
             bool EndOk = !SM.isBeforeInTranslationUnit(Range.getEnd(), Info.Range.getEnd());     // Info.End <= Range.End

             if (StartOk && EndOk) {
                 alreadyRemoved = true;
                 break;
             }
        }
        if (alreadyRemoved) continue;

        bool isDangerous = DangerousOffsets.count(Offset);

        if (isDangerous) {
            CharSourceRange CRange = CharSourceRange::getTokenRange(Info.Range);
            if (Info.ExternalSemiLoc.isValid()) {
                TheRewriter.RemoveText(CRange);
            } else {
                TheRewriter.ReplaceText(CRange, ";");
            }
        } else {
            if (!Info.InternalSemi && Info.ExternalSemiLoc.isValid()) {
                CharSourceRange CRange = CharSourceRange::getTokenRange(Info.Range.getBegin(), Info.ExternalSemiLoc);
                TheRewriter.RemoveText(CRange);
            } else {
                CharSourceRange CRange = CharSourceRange::getTokenRange(Info.Range);
                TheRewriter.RemoveText(CRange);
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
