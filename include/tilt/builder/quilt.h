#ifndef INCLUDE_TILT_BUILDER_QUILT_H_
#define INCLUDE_TILT_BUILDER_QUILT_H_

#include <map>

#include "tilt/builder/tilder.h"

using namespace std;

namespace tilt::tilder {

class QuiltCtx {
private:
    map<Sym, Expr> syms;

    friend class Quilt;
};

class Quilt {
public:
    Quilt(shared_ptr<QuiltCtx> ctx) : ctx(ctx) {}

    Op build();

    Quilt Select(function<Expr(_sym)>);
    Quilt Where(function<Expr(_sym)>);
    Quilt Join(Quilt, function<Expr(_sym, _sym)>);

private:
    Sym sym;
    shared_ptr<QuiltCtx> ctx;
};

}  // namespace tilt::tilder

#endif  // INCLUDE_TILT_BUILDER_QUILT_H_