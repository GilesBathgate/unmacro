#ifndef UNMACRO_TYPES_H
#define UNMACRO_TYPES_H

#include "clang/Basic/SourceLocation.h"
#include <vector>
#include <string>

struct ExpansionInfo {
    clang::SourceRange Range;
    std::string MacroName;
    bool InternalSemi;
    clang::SourceLocation ExternalSemiLoc;
};

#endif
