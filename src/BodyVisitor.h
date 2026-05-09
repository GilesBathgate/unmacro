#include "clang/AST/RecursiveASTVisitor.h"
#include "UnmacroTypes.h"
#include <set>
#include <vector>

class BodyVisitor : public clang::RecursiveASTVisitor<BodyVisitor> {
    std::set<unsigned> &DangerousOffsets;
    std::vector<clang::SourceRange> &EmptyStmtRanges;
    const std::vector<ExpansionInfo> &Expansions;
    clang::SourceManager &SM;
    const clang::LangOptions &LangOpts;
    bool RemoveExtraStatements;

public:
    BodyVisitor(std::set<unsigned> &O, std::vector<clang::SourceRange> &ESR, const std::vector<ExpansionInfo> &Ex, clang::SourceManager &SM, const clang::LangOptions &LO, bool RemoveExtraStatements);

    bool VisitIfStmt(clang::IfStmt *S);
    bool VisitWhileStmt(clang::WhileStmt *S);
    bool VisitForStmt(clang::ForStmt *S);
    bool VisitDoStmt(clang::DoStmt *S);

private:
    void checkDangerous(clang::Stmt *S);
    bool isEffectivelyEmpty(const clang::Stmt *S);
    void markForRemoval(clang::Stmt *S);
    bool isRemovedExpansion(clang::SourceLocation Loc);
};
