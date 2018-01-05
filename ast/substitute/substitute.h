#ifndef SRUBY_SUBSTITUTE_H
#define SRUBY_SUBSTITUTE_H

#include "ast/ast.h"
#include "core/Context.h"

namespace ruby_typer {
namespace ast {
class Substitute {
public:
    static std::unique_ptr<Expression> run(core::Context ctx, const core::GlobalSubstitution &subst,
                                           std::unique_ptr<Expression> what);
};
} // namespace ast
} // namespace ruby_typer
#endif // SRUBY_SUBSTITUTE_H
