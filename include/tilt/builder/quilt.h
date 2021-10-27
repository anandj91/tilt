#ifndef INCLUDE_TILT_BUILDER_QUILT_H_
#define INCLUDE_TILT_BUILDER_QUILT_H_

#include <map>

#include "tilt/builder/tilder.h"

using namespace std;

namespace tilt::tilder {

class QuiltCtx {
public:
    QuiltCtx() : count(0) {}

private:
    vector<Sym> inputs;
    map<Sym, Expr> syms;
    int count;

    friend class Quilt;
};

class Quilt {
public:
    static Quilt Stream(string, DataType, shared_ptr<QuiltCtx>);
    Op Build();

    Quilt Select(function<Expr(_sym)>);
    Quilt Where(function<Expr(_sym)>);
    Quilt InnerJoin(Quilt, function<Expr(_sym, _sym)>);
    Quilt Sum(int64_t, int64_t);

private:
    Quilt(_sym sym, shared_ptr<QuiltCtx> ctx) : sym(sym), ctx(ctx) {}

    Quilt submit(string, Op);

    _sym sym;
    shared_ptr<QuiltCtx> ctx;
};

}  // namespace tilt::tilder

#endif  // INCLUDE_TILT_BUILDER_QUILT_H_