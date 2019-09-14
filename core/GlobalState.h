#ifndef SORBET_GLOBAL_STATE_H
#define SORBET_GLOBAL_STATE_H
#include "absl/synchronization/mutex.h"

#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Files.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/lsp/Query.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <memory>

namespace sorbet::core {

class Name;
class NameRef;
class Symbol;
class SymbolRef;
class GlobalSubstitution;
class ErrorRegion;
class ErrorQueue;
struct GlobalStateHash;

namespace serialize {
class Serializer;
class SerializerImpl;
} // namespace serialize

class GlobalState final {
    friend Name;
    friend NameRef;
    friend Symbol;
    friend SymbolRef;
    friend File;
    friend FileRef;
    friend GlobalSubstitution;
    friend ErrorRegion;
    friend ErrorBuilder;
    friend serialize::Serializer;
    friend serialize::SerializerImpl;
    friend class UnfreezeNameTable;
    friend class UnfreezeSymbolTable;
    friend class UnfreezeFileTable;
    friend struct NameRefDebugCheck;

public:
    GlobalState(std::shared_ptr<ErrorQueue> errorQueue);

    void initEmpty();
    void installIntrinsics();

    // Expand tables to use approximate `kb` KiB of memory. Can be used prior to
    // operation to avoid table resizes.
    void reserveMemory(u4 kb);

    GlobalState(const GlobalState &) = delete;
    GlobalState(GlobalState &&) = delete;

    ~GlobalState() = default;

    SymbolRef enterClassSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterTypeMember(Loc loc, SymbolRef owner, NameRef name, Variance variance);
    SymbolRef enterTypeArgument(Loc loc, SymbolRef owner, NameRef name, Variance variance);
    SymbolRef enterMethodSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterNewMethodOverload(Loc loc, SymbolRef original, core::NameRef originalName, u2 num,
                                     const std::vector<int> &argsToKeep);
    SymbolRef enterFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    SymbolRef enterStaticFieldSymbol(Loc loc, SymbolRef owner, NameRef name);
    ArgInfo &enterMethodArgumentSymbol(Loc loc, SymbolRef owner, NameRef name);

    SymbolRef lookupSymbol(SymbolRef owner, NameRef name) {
        return lookupSymbolWithFlags(owner, name, 0);
    }
    SymbolRef lookupTypeMemberSymbol(SymbolRef owner, NameRef name) {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::TYPE_MEMBER);
    }
    SymbolRef lookupClassSymbol(SymbolRef owner, NameRef name) {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::CLASS_OR_MODULE);
    }
    SymbolRef lookupMethodSymbol(SymbolRef owner, NameRef name) {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::METHOD);
    }
    SymbolRef lookupMethodSymbolWithHash(SymbolRef owner, NameRef name, std::vector<u4> methodHash) const;
    SymbolRef lookupStaticFieldSymbol(SymbolRef owner, NameRef name) {
        return lookupSymbolWithFlags(owner, name, Symbol::Flags::STATIC_FIELD);
    }
    SymbolRef findRenamedSymbol(SymbolRef owner, SymbolRef name);

    SymbolRef staticInitForFile(Loc loc);
    SymbolRef staticInitForClass(SymbolRef klass, Loc loc);

    SymbolRef lookupStaticInitForFile(Loc loc) const;
    SymbolRef lookupStaticInitForClass(SymbolRef klass) const;

    NameRef enterNameUTF8(std::string_view nm);

    NameRef lookupNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num) const;
    NameRef freshNameUnique(UniqueNameKind uniqueNameKind, NameRef original, u2 num);

    NameRef enterNameConstant(NameRef original);
    NameRef enterNameConstant(std::string_view original);

    FileRef enterFile(std::string_view path, std::string_view source);
    FileRef enterFile(const std::shared_ptr<File> &file);
    FileRef enterNewFileAt(const std::shared_ptr<File> &file, FileRef id);
    FileRef reserveFileRef(std::string path);
    static std::unique_ptr<GlobalState> replaceFile(std::unique_ptr<GlobalState> inWhat, FileRef whatFile,
                                                    const std::shared_ptr<File> &withWhat);
    static std::unique_ptr<GlobalState> markFileAsTombStone(std::unique_ptr<GlobalState>, FileRef fref);
    FileRef findFileByPath(std::string_view path) const;

    void mangleRenameSymbol(SymbolRef what, NameRef origName);
    spdlog::logger &tracer() const;
    unsigned int namesUsed() const;

    unsigned int symbolsUsed() const;
    unsigned int filesUsed() const;

    void sanityCheck() const;
    void markAsPayload();

    // These methods are here to make it easier to print the symbol table in lldb.
    // (don't have to remember the default args)
    std::string toString() {
        bool showFull = false;
        bool showRaw = false;
        return toStringWithOptions(showFull, showRaw);
    }
    std::string toStringFull() {
        bool showFull = true;
        bool showRaw = false;
        return toStringWithOptions(showFull, showRaw);
    }
    std::string showRaw() {
        bool showFull = false;
        bool showRaw = true;
        return toStringWithOptions(showFull, showRaw);
    }
    std::string showRawFull() {
        bool showFull = true;
        bool showRaw = true;
        return toStringWithOptions(showFull, showRaw);
    }

    bool hadCriticalError() const;

    ErrorBuilder beginError(Loc loc, ErrorClass what) const;
    void _error(std::unique_ptr<Error> error) const;

    int totalErrors() const;
    bool wasModified() const;

    int globalStateId;
    bool silenceErrors = false;
    bool autocorrect = false;
    bool suggestRuntimeProfiledType = false;

    // We have a lot of internal names of form `<something>` that's chosen with `<` and `>` as you can't make
    // this into a valid ruby identifier without suffering.
    // We want to make sure we don't round-trip through strings for those names.
    //
    // If this attribute is set to `true`, all strings will be checked for `<` and `>` characters in them.
    bool ensureCleanStrings = false;

    // So we can know whether we're running in autogen mode.
    // Right now this is only used to turn certain DSL passes on or off.
    // Think very hard before looking at this value in namer / resolver!
    // (hint: probably you want to find an alternate solution)
    bool runningUnderAutogen = false;
    bool censorForSnapshotTests = false;

    std::unique_ptr<GlobalState> deepCopy(bool keepId = false) const;
    mutable std::shared_ptr<ErrorQueue> errorQueue;

    // Contains a path prefix that should be stripped from all printed paths.
    std::string pathPrefix;
    // Returns a string_view of the given path with the path prefix removed.
    std::string_view getPrintablePath(std::string_view path) const;

    // Contains a location / symbol / variable reference that various Sorbet passes are looking for.
    // See ErrorQueue#queryResponse
    lsp::Query lspQuery;

    FlowId creation; // used to track flow of global states

    // Indicates the number of times LSP has run the type checker with this global state.
    // Used to ensure GlobalState is in the correct state to process requests.
    unsigned int lspTypecheckCount = 0;

    void trace(std::string_view msg) const;

    std::unique_ptr<GlobalStateHash> hash() const;
    const std::vector<std::shared_ptr<File>> &getFiles() const;

    // Contains a string to be used as the base of the error URL.
    // The error code is appended to this string.
    std::string errorUrlBase;
    void suppressErrorClass(int code);
    void onlyShowErrorClass(int code);

    std::vector<std::string> dslRubyExtraArgs;
    void addDslPlugin(std::string_view method, std::string_view command);
    std::optional<std::string_view> findDslPlugin(NameRef method) const;
    bool hasAnyDslPlugin() const;

    std::vector<std::unique_ptr<pipeline::semantic_extension::SemanticExtension>> semanticExtensions;

private:
    bool shouldReportErrorOn(Loc loc, ErrorClass what) const;
    struct DeepCloneHistoryEntry {
        int globalStateId;
        unsigned int lastNameKnownByParentGlobalState;
    };
    std::vector<DeepCloneHistoryEntry> deepCloneHistory;

    static constexpr int STRINGS_PAGE_SIZE = 4096;
    std::vector<std::shared_ptr<std::vector<char>>> strings;
    std::string_view enterString(std::string_view nm);
    u2 stringsLastPageUsed = STRINGS_PAGE_SIZE + 1;
    std::vector<Name> names;
    UnorderedMap<std::string, FileRef> fileRefByPath;
    std::vector<Symbol> symbols;
    std::vector<std::pair<unsigned int, unsigned int>> namesByHash;
    std::vector<std::shared_ptr<File>> files;
    UnorderedSet<int> suppressedErrorClasses;
    UnorderedSet<int> onlyErrorClasses;
    UnorderedMap<NameRef, std::string> dslPlugins;
    bool wasModified_ = false;

    bool freezeSymbolTable();
    bool freezeNameTable();
    bool freezeFileTable();
    bool unfreezeSymbolTable();
    bool unfreezeNameTable();
    bool unfreezeFileTable();
    bool nameTableFrozen = true;
    bool symbolTableFrozen = true;
    bool fileTableFrozen = true;

    void expandNames(int growBy = 2);

    SymbolRef synthesizeClass(NameRef nameID, u4 superclass = Symbols::todo()._id, bool isModule = false);
    SymbolRef enterSymbol(Loc loc, SymbolRef owner, NameRef name, u4 flags);

    SymbolRef lookupSymbolSuchThat(SymbolRef owner, NameRef name, std::function<bool(SymbolRef)> pred) const;
    SymbolRef lookupSymbolWithFlags(SymbolRef owner, NameRef name, u4 flags) const;

    SymbolRef getTopLevelClassSymbol(NameRef name);

    std::string toStringWithOptions(bool showFull, bool showRaw) const;
};
// CheckSize(GlobalState, 152, 8);
// Historically commented out because size of unordered_map was different between different versions of stdlib

} // namespace sorbet::core

#endif // SORBET_GLOBAL_STATE_H
