#include "UnmacroAction.h"
#include "UnmacroPPCallbacks.h"
#include "BodyVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include <iostream>
#include <algorithm>

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
    const LangOptions &LangOpts = Context.getLangOpts();

    std::set<unsigned> DangerousOffsets;
    std::vector<SourceRange> EmptyStmtRanges;
    BodyVisitor Visitor(DangerousOffsets, EmptyStmtRanges, Expansions, SM, LangOpts, RemoveExtraStatements);
    Visitor.TraverseAST(Context);

    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SM, LangOpts);

    std::vector<std::pair<unsigned, unsigned>> RemovedIntervals;

    auto MarkRemoved = [&](unsigned Start, unsigned End) {
        RemovedIntervals.push_back({Start, End});
    };

    auto IsAlreadyRemoved = [&](unsigned Start, unsigned End) {
        for (const auto &Interval : RemovedIntervals) {
            if (Start >= Interval.first && End <= Interval.second) return true;
        }
        return false;
    };

    if (RemoveExtraStatements) {
        // Sort to process larger/earlier ranges first
        std::sort(EmptyStmtRanges.begin(), EmptyStmtRanges.end(), [&](const SourceRange &A, const SourceRange &B) {
            if (A.getBegin() != B.getBegin())
                return SM.isBeforeInTranslationUnit(A.getBegin(), B.getBegin());
            return SM.isBeforeInTranslationUnit(B.getEnd(), A.getEnd());
        });

        for (const auto &Range : EmptyStmtRanges) {
            CharSourceRange CRange = CharSourceRange::getTokenRange(Range);
            unsigned Start = SM.getFileOffset(CRange.getBegin());
            unsigned End = SM.getFileOffset(Lexer::getLocForEndOfToken(CRange.getEnd(), 0, SM, LangOpts));

            if (IsAlreadyRemoved(Start, End)) continue;

            TheRewriter.RemoveText(CRange);
            MarkRemoved(Start, End);
        }
    }

    std::vector<ExpansionInfo> SortedExpansions = Expansions;
    std::sort(SortedExpansions.begin(), SortedExpansions.end(), [&](const ExpansionInfo &A, const ExpansionInfo &B) {
        return SM.isBeforeInTranslationUnit(A.Range.getBegin(), B.Range.getBegin());
    });

    for (const auto &Info : SortedExpansions) {
        SourceLocation StartLoc = Info.Range.getBegin();
        SourceLocation EndLoc = Info.Range.getEnd();
        unsigned StartOffset = SM.getFileOffset(StartLoc);
        unsigned EndOffset = SM.getFileOffset(Lexer::getLocForEndOfToken(EndLoc, 0, SM, LangOpts));

        if (IsAlreadyRemoved(StartOffset, EndOffset)) continue;

        bool isDangerous = DangerousOffsets.count(StartOffset);

        if (isDangerous) {
            CharSourceRange CRange = CharSourceRange::getTokenRange(Info.Range);
            if (Info.ExternalSemiLoc.isValid()) {
                TheRewriter.RemoveText(CRange);
                MarkRemoved(StartOffset, EndOffset);
            } else {
                TheRewriter.ReplaceText(CRange, ";");
                MarkRemoved(StartOffset, EndOffset);
            }
        } else {
            if (!Info.InternalSemi && Info.ExternalSemiLoc.isValid()) {
                SourceLocation SemiEnd = Lexer::getLocForEndOfToken(Info.ExternalSemiLoc, 0, SM, LangOpts);
                CharSourceRange FullCRange = CharSourceRange::getCharRange(StartLoc, SemiEnd);
                TheRewriter.RemoveText(FullCRange);
                MarkRemoved(StartOffset, SM.getFileOffset(SemiEnd));
            } else {
                CharSourceRange CRange = CharSourceRange::getTokenRange(Info.Range);
                TheRewriter.RemoveText(CRange);
                MarkRemoved(StartOffset, EndOffset);
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
