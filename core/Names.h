#ifndef SRUBY_NAMES_H
#define SRUBY_NAMES_H

#include "common/common.h"
#include <string>
#include <vector>

#include "absl/strings/string_view.h"

namespace ruby_typer {
namespace core {
class GlobalState;
class Name;
enum NameKind : u1 {
    UTF8 = 1,
    UNIQUE = 2,
    CONSTANT = 3,
};

CheckSize(NameKind, 1, 1);

inline int _NameKind2Id_UTF8(NameKind nm) {
    ENFORCE(nm == UTF8);
    return 1;
}

inline int _NameKind2Id_UNIQUE(NameKind nm) {
    ENFORCE(nm == UNIQUE);
    return 2;
}

inline int _NameKind2Id_CONSTANT(NameKind nm) {
    ENFORCE(nm == CONSTANT);
    return 3;
}

class NameRef final {
public:
    friend GlobalState;
    friend Name;

    NameRef() : _id(-1){};

    constexpr NameRef(unsigned int id) : _id(id) {}

    NameRef(const NameRef &nm) = default;

    NameRef(NameRef &&nm) = default;

    NameRef &operator=(const NameRef &rhs) = default;

    bool operator==(const NameRef &rhs) const {
        return _id == rhs._id;
    }

    bool operator!=(const NameRef &rhs) const {
        return !(rhs == *this);
    }

    inline int id() const {
        return _id;
    }

    Name &name(GlobalState &gs) const;

    inline bool exists() const {
        return _id != 0;
    }

    bool isBlockClashSafe(GlobalState &gs) const;

    NameRef addEq(GlobalState &gs) const;

    std::string toString(GlobalState &gs) const;

public:
    int _id;
};

CheckSize(NameRef, 4, 4);

struct RawName final {
    absl::string_view utf8;
};
CheckSize(RawName, 16, 8);

enum UniqueNameKind : u2 {
    Parser,
    Desugar,
    Namer,
    CFG,
    NestedScope, // used by freshName to make sure blocks local variables do not collapse into method variables
    Singleton,
};

struct UniqueName final {
    NameRef original;
    UniqueNameKind uniqueNameKind;
    u2 num;
};

CheckSize(UniqueName, 8, 4);

struct ConstantName final {
    NameRef original;
};

#include "core/Names_gen.h"

class Name final {
public:
    friend GlobalState;

    NameKind kind;

private:
    unsigned char UNUSED(_fill[3]);

public:
    union { // todo: can discriminate this union through the pointer to Name
        // itself using lower bits
        RawName raw;
        UniqueName unique;
        ConstantName cnst;
    };

    Name() noexcept {};

    Name(Name &&other) noexcept = default;

    Name(const Name &other) = delete;

    ~Name() noexcept;

    bool operator==(const Name &rhs) const;

    bool operator!=(const Name &rhs) const;

    std::string toString(GlobalState &gs) const;
    void sanityCheck(GlobalState &gs);
    NameRef ref(GlobalState &gs) const;

private:
    unsigned int hash(const GlobalState &gs) const;

public:
    static unsigned int hashNames(std::vector<NameRef> &lhs, GlobalState &gs);
};

CheckSize(Name, 24, 8);
} // namespace core
} // namespace ruby_typer

template <> struct std::hash<ruby_typer::core::NameRef> {
    size_t operator()(const ruby_typer::core::NameRef &x) const {
        return x._id;
    }
};

template <> struct std::equal_to<ruby_typer::core::NameRef> {
    constexpr bool operator()(const ruby_typer::core::NameRef &lhs, const ruby_typer::core::NameRef &rhs) const {
        return lhs._id == rhs._id;
    }
};

#endif // SRUBY_NAMES_H
