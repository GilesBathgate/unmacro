#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/CommandLine.h"
#include <vector>
#include <set>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static cl::OptionCategory UnmacroCategory("unmacro options");
static cl::opt<std::string> MacroNameOpt("macro-name", cl::desc("The specific macro to remove"), cl::Required, cl::cat(UnmacroCategory));
static cl::opt<bool> InplaceOpt("inplace", cl::desc("Modify the file directly"), cl::init(false), cl::cat(UnmacroCategory));

struct ExpansionInfo {
    SourceRange Range;
    bool InternalSemi;
    SourceLocation ExternalSemiLoc;
};

class UnmacroPPCallbacks : public PPCallbacks {
    std::vector<ExpansionInfo> &Expansions;
    std::string TargetMacroName;
    Preprocessor &PP;

public:
    UnmacroPPCallbacks(std::vector<ExpansionInfo> &E, StringRef Name, Preprocessor &PP)
        : Expansions(E), TargetMacroName(Name), PP(PP) {}

    void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD,
                      SourceRange Range, const MacroArgs *Args) override {
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
};

class BodyVisitor : public RecursiveASTVisitor<BodyVisitor> {
    std::set<unsigned> &DangerousOffsets;
    SourceManager &SM;
public:
    BodyVisitor(std::set<unsigned> &O, SourceManager &SM) : DangerousOffsets(O), SM(SM) {}

    bool VisitIfStmt(IfStmt *S) {
        check(S->getThen());
        check(S->getElse());
        return true;
    }
    bool VisitWhileStmt(WhileStmt *S) {
        check(S->getBody());
        return true;
    }
    bool VisitForStmt(ForStmt *S) {
        check(S->getBody());
        return true;
    }
    bool VisitDoStmt(DoStmt *S) {
        check(S->getBody());
        return true;
    }

private:
    void check(Stmt *S) {
        if (S && !isa<CompoundStmt>(S)) {
            SourceLocation Loc = S->getBeginLoc();
            if (Loc.isValid() && SM.isWrittenInMainFile(SM.getExpansionLoc(Loc))) {
                DangerousOffsets.insert(SM.getFileOffset(SM.getExpansionLoc(Loc)));
            }
        }
    }
};

class UnmacroAction : public ASTFrontendAction {
    std::vector<ExpansionInfo> Expansions;
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        CI.getPreprocessor().addPPCallbacks(std::make_unique<UnmacroPPCallbacks>(
            Expansions, MacroNameOpt, CI.getPreprocessor()));
        return std::make_unique<ASTConsumer>();
    }

    void EndSourceFileAction() override {
        CompilerInstance &CI = getCompilerInstance();
        if (!CI.hasASTContext()) return;

        ASTContext &Context = CI.getASTContext();
        SourceManager &SM = Context.getSourceManager();

        std::set<unsigned> DangerousOffsets;
        BodyVisitor Visitor(DangerousOffsets, SM);
        Visitor.TraverseAST(Context);

        Rewriter TheRewriter;
        TheRewriter.setSourceMgr(SM, Context.getLangOpts());

        for (const auto &Info : Expansions) {
            unsigned StartOffset = SM.getFileOffset(Info.Range.getBegin());
            bool startIsDangerous = DangerousOffsets.count(StartOffset);

            bool semiIsDangerous = false;
            if (Info.ExternalSemiLoc.isValid()) {
                unsigned SemiOffset = SM.getFileOffset(Info.ExternalSemiLoc);
                semiIsDangerous = DangerousOffsets.count(SemiOffset);
            }

            if (startIsDangerous || semiIsDangerous) {
                // Keep the semicolon
                TheRewriter.RemoveText(Info.Range);
            } else {
                if (!Info.InternalSemi && Info.ExternalSemiLoc.isValid()) {
                    TheRewriter.RemoveText(SourceRange(Info.Range.getBegin(), Info.ExternalSemiLoc));
                } else {
                    TheRewriter.RemoveText(Info.Range);
                }
            }
        }

        if (TheRewriter.buffer_begin() != TheRewriter.buffer_end()) {
            if (InplaceOpt) {
                TheRewriter.overwriteChangedFiles();
            } else {
                TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
            }
        } else if (!InplaceOpt) {
            bool Invalid = false;
            llvm::StringRef Data = SM.getBufferData(SM.getMainFileID(), &Invalid);
            if (!Invalid) llvm::outs() << Data;
        }
    }
};

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, UnmacroCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<UnmacroAction>().get());
}
