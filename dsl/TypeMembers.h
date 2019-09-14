#ifndef SORBET_DSL_TYPEMEMBERS_H
#define SORBET_DSL_TYPEMEMBERS_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class does nothing but raise errors for and then delete duplicate type members
 */
class TypeMembers final {
public:
    static void patchDSL(core::MutableContext ctx, ast::ClassDef *cdef);

    TypeMembers() = delete;
};

} // namespace sorbet::dsl

#endif
