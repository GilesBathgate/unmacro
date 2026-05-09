#ifndef UNMACRO_TYPES_H
#define UNMACRO_TYPES_H

#include "clang/Basic/SourceLocation.h"
#include <vector>

struct ExpansionInfo {
    clang::SourceRange Range;
    bool InternalSemi;
    clang::SourceLocation ExternalSemiLoc;
};

#endif
