#ifndef UNMACRO_PP_CALLBACKS_H
#define UNMACRO_PP_CALLBACKS_H

#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "UnmacroTypes.h"
#include <vector>
#include <string>

class UnmacroPPCallbacks : public clang::PPCallbacks {
    std::vector<ExpansionInfo> &Expansions;
    std::vector<std::string> TargetMacroNames;
    clang::Preprocessor &PP;

public:
    UnmacroPPCallbacks(std::vector<ExpansionInfo> &E, const std::vector<std::string> &Names, clang::Preprocessor &PP);

    void MacroExpands(const clang::Token &MacroNameTok, const clang::MacroDefinition &MD,
                      clang::SourceRange Range, const clang::MacroArgs *Args) override;
};

#endif
