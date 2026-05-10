#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "UnmacroAction.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

static cl::OptionCategory UnmacroCategory("unmacro options");
static cl::list<std::string> MacroNamesOpt("macro-name", cl::desc("The specific macro to remove (can be specified multiple times)"), cl::OneOrMore, cl::cat(UnmacroCategory));
static cl::opt<bool> InplaceOpt("inplace", cl::desc("Modify the file directly"), cl::init(false), cl::cat(UnmacroCategory));
static cl::opt<bool> RemoveExtraStatementsOpt("remove-extra-statements", cl::desc("Remove empty if/for/while/do statements after macro removal"), cl::init(false), cl::cat(UnmacroCategory));

int main(int argc, const char **argv) {
    bool isCppVersion = false;
    if (argc > 0) {
        StringRef ExecName = llvm::sys::path::filename(argv[0]);
        if (ExecName == "unmacro++") {
            isCppVersion = true;
        }
    }

    auto ExpectedParser = CommonOptionsParser::create(argc, argv, UnmacroCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    Tool.appendArgumentsAdjuster([isCppVersion](const CommandLineArguments &Args, StringRef Filename) {
        CommandLineArguments AdjustedArgs;
        bool hasXC = false;
        bool hasStdCpp = false;
        for (const auto &Arg : Args) {
            if (Arg == "-xc++" || Arg == "-xc") hasXC = true;
            if (Arg.find("-std=c++") != std::string::npos) hasStdCpp = true;
        }

        if (!hasXC && (isCppVersion || (hasStdCpp && Filename.ends_with(".h")))) {
             AdjustedArgs.push_back("-xc++");
        }
        for (const auto &Arg : Args) AdjustedArgs.push_back(Arg);
        return AdjustedArgs;
    });

    class UnmacroActionFactory : public FrontendActionFactory {
        std::vector<std::string> MacroNames;
        bool Inplace;
        bool RemoveExtraStatements;
    public:
        UnmacroActionFactory(std::vector<std::string> Names, bool Inplace, bool Extra)
            : MacroNames(Names), Inplace(Inplace), RemoveExtraStatements(Extra) {}
        std::unique_ptr<FrontendAction> create() override {
            return std::make_unique<UnmacroAction>(MacroNames, Inplace, RemoveExtraStatements);
        }
    };

    std::vector<std::string> MacroNames;
    for (const auto &Name : MacroNamesOpt) {
        MacroNames.push_back(Name);
    }
    UnmacroActionFactory Factory(MacroNames, InplaceOpt, RemoveExtraStatementsOpt);
    int Result = Tool.run(&Factory);
    llvm::errs() << "[Main] Tool.run finished with " << Result << "\n";
    return Result;
}
