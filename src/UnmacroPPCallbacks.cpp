#include "UnmacroPPCallbacks.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroArgs.h"

using namespace clang;

UnmacroPPCallbacks::UnmacroPPCallbacks(std::vector<ExpansionInfo> &E, llvm::StringRef Name, Preprocessor &PP)
    : Expansions(E), TargetMacroName(Name), PP(PP) {}

void UnmacroPPCallbacks::MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD,
                                     SourceRange Range, const MacroArgs *Args) {
    const IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
    if (!II || II->getName() != TargetMacroName)
        return;

    SourceManager &SM = PP.getSourceManager();
    SourceLocation StartLoc = SM.getExpansionLoc(Range.getBegin());
    SourceLocation EndLoc = SM.getExpansionLoc(Range.getEnd());

    if (!SM.isWrittenInMainFile(StartLoc)) return;

    bool internalSemicolon = false;
    if (Args) {
        for (unsigned i = 0; i < Args->getNumMacroArguments(); ++i) {
            const Token *ArgToks = Args->getUnexpArgument(i);
            if (!ArgToks) continue;
            for (const Token *T = ArgToks; T->isNot(tok::eof); ++T) {
                if (T->is(tok::semi)) {
                    internalSemicolon = true;
                    break;
                }
            }
            if (internalSemicolon) break;
        }
    }

    ExpansionInfo Info;
    Info.Range = SourceRange(StartLoc, EndLoc);
    Info.InternalSemi = internalSemicolon;
    Info.ExternalSemiLoc = SourceLocation();

    std::optional<Token> SemicolonTok = Lexer::findNextToken(EndLoc, SM, PP.getLangOpts());
    if (SemicolonTok && SemicolonTok->is(tok::semi)) {
        Info.ExternalSemiLoc = SemicolonTok->getLocation();
    }

    Expansions.push_back(Info);
}
