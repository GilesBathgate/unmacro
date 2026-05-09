#include "BodyVisitor.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"

using namespace clang;

BodyVisitor::BodyVisitor(std::set<unsigned> &O, std::vector<SourceRange> &ESR, const std::vector<ExpansionInfo> &Ex, SourceManager &SM, const LangOptions &LO, bool RemoveExtraStatements)
    : DangerousOffsets(O), EmptyStmtRanges(ESR), Expansions(Ex), SM(SM), LangOpts(LO), RemoveExtraStatements(RemoveExtraStatements) {}

bool BodyVisitor::VisitIfStmt(IfStmt *S) {
    checkDangerous(S->getThen());
    checkDangerous(S->getElse());
    if (RemoveExtraStatements && isEffectivelyEmpty(S->getThen()) && isEffectivelyEmpty(S->getElse())) {
        markForRemoval(S);
    }
    return true;
}
bool BodyVisitor::VisitWhileStmt(WhileStmt *S) {
    checkDangerous(S->getBody());
    if (RemoveExtraStatements && isEffectivelyEmpty(S->getBody())) {
        markForRemoval(S);
    }
    return true;
}
bool BodyVisitor::VisitForStmt(ForStmt *S) {
    checkDangerous(S->getBody());
    if (RemoveExtraStatements && isEffectivelyEmpty(S->getBody())) {
        markForRemoval(S);
    }
    return true;
}
bool BodyVisitor::VisitDoStmt(DoStmt *S) {
    checkDangerous(S->getBody());
    if (RemoveExtraStatements && isEffectivelyEmpty(S->getBody())) {
        markForRemoval(S);
    }
    return true;
}

void BodyVisitor::checkDangerous(Stmt *S) {
    if (!S) return;
    if (!isa<CompoundStmt>(S)) {
        SourceLocation Loc = S->getBeginLoc();
        if (Loc.isValid()) {
            SourceLocation ExpLoc = Loc.isMacroID() ? SM.getExpansionLoc(Loc) : Loc;
            if (SM.isWrittenInMainFile(ExpLoc)) {
                 DangerousOffsets.insert(SM.getFileOffset(ExpLoc));
            }
        }
    }
}

bool BodyVisitor::isEffectivelyEmpty(const Stmt *S) {
    if (!S) return true;
    if (isa<NullStmt>(S)) return true;
    if (const CompoundStmt *CS = dyn_cast<CompoundStmt>(S)) {
        for (auto *Child : CS->body()) {
            if (!isEffectivelyEmpty(Child)) return false;
        }
        return true;
    }

    SourceLocation Loc = S->getBeginLoc();
    return isRemovedExpansion(Loc);
}

bool BodyVisitor::isRemovedExpansion(SourceLocation Loc) {
    if (Loc.isMacroID()) {
        SourceLocation ExpLoc = SM.getExpansionLoc(Loc);
        unsigned Offset = SM.getFileOffset(ExpLoc);
        for (const auto &Info : Expansions) {
            if (SM.getFileOffset(Info.Range.getBegin()) == Offset) {
                return true;
            }
        }
    }
    return false;
}

void BodyVisitor::markForRemoval(Stmt *S) {
    SourceLocation Start = SM.getExpansionLoc(S->getBeginLoc());
    SourceLocation End = SM.getExpansionLoc(S->getEndLoc());
    if (Start.isValid() && End.isValid() && SM.isWrittenInMainFile(Start)) {
        // If the body was not a CompoundStmt, it likely has a trailing semicolon
        // that belongs to the statement inside it, which we've already handled
        // or will handle. But for the control structure itself:
        if (isa<IfStmt>(S) || isa<ForStmt>(S) || isa<WhileStmt>(S)) {
             // These don't have trailing semicolons unless the body is a simple statement
             // which ALREADY has a semicolon.
        }

        // Extension to trailing semicolon for do-while
        if (isa<DoStmt>(S)) {
            std::optional<Token> SemiTok = Lexer::findNextToken(End, SM, LangOpts);
            if (SemiTok && SemiTok->is(tok::semi)) {
                 End = SemiTok->getLocation();
            }
        }

        EmptyStmtRanges.push_back(SourceRange(Start, End));
    }
}
