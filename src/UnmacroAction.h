#ifndef UNMACRO_ACTION_H
#define UNMACRO_ACTION_H

#include "clang/Frontend/FrontendActions.h"
#include "UnmacroTypes.h"
#include <vector>
#include <string>

class UnmacroAction : public clang::ASTFrontendAction {
    std::vector<ExpansionInfo> Expansions;
    std::vector<std::string> TargetMacroNames;
    bool Inplace;
    bool RemoveExtraStatements;

public:
    UnmacroAction(const std::vector<std::string> &MacroNames, bool Inplace, bool RemoveExtraStatements);
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef file) override;
    void EndSourceFileAction() override;
};

#endif
