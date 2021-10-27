#include "tilt/builder/quilt.h"

namespace tilt::tilder {

Quilt Quilt::submit(string name, Op op)
{
    auto sym = _sym(name + "_" + to_string(ctx->count++), op);
    ctx->syms[sym] = op;
    return Quilt(sym, ctx);
}

Quilt Quilt::Stream(string name, DataType dtype, shared_ptr<QuiltCtx> ctx)
{
    auto sym = _sym(name, Type(move(dtype), _iter(0, -1)));
    ctx->inputs.push_back(sym);
    return Quilt(sym, ctx);
}

Op Quilt::Build()
{
    return nullptr;
}

Quilt Quilt::Select(function<Expr(_sym)> selector)
{
    auto in_sym = this->sym;

    auto e = in_sym[_pt(0)];
    auto e_sym = _sym("e", e);
    auto sel = selector(e_sym);
    auto sel_sym = _sym("sel", sel);

    auto sel_op = _op(
        _iter(0, 1),
        Params{in_sym},
        SymTable{{e_sym, e}, {sel_sym, sel}},
        _exists(e_sym),
        sel_sym);

    return submit("select", sel_op);
}

Quilt Quilt::Where(function<Expr(_sym)> predicate)
{
    auto in_sym = this->sym;

    auto e = in_sym[_pt(0)];
    auto e_sym = _sym("e", e);
    auto pred = predicate(e_sym);
    auto pred_sym = _sym("pred", pred);

    auto where_op = _op(
        _iter(0, 1),
        Params{in_sym},
        SymTable{{e_sym, e}, {pred_sym, pred}},
        _exists(e_sym) && pred_sym,
        e_sym);

    return submit("where", where_op);
}

Quilt Quilt::InnerJoin(Quilt right, function<Expr(_sym, _sym)> joiner)
{
    auto left_sym = this->sym;
    auto right_sym = right.sym;

    auto l = left_sym[_pt(0)];
    auto l_sym = _sym("l", l);
    auto r = right_sym[_pt(0)];
    auto r_sym = _sym("r", r);
    auto join = joiner(l_sym, r_sym);
    auto join_sym = _sym("join", join);

    auto join_op = _op(
        _iter(0, 1),
        Params{left_sym, right_sym},
        SymTable{{l_sym, l}, {r_sym, r}, {join_sym, join}},
        _exists(l_sym) && _exists(r_sym),
        join_sym);

    return submit("join", join_op);
}

Quilt Quilt::Sum(int64_t window, int64_t period)
{
    auto in_sym = this->sym;

    auto win = in_sym[_win(-window, 0)];
    auto win_sym = _sym("win", win);
    auto zero = _const(in_sym->type.dtype.btype, 0);
    auto sum = _red(win_sym, zero, [](Expr s, Expr st, Expr et, Expr d) { return _add(s, d); });
    auto sum_sym = _sym("sum", sum);

    auto wsum_op = _op(
        _iter(0, period),
        Params{in_sym},
        SymTable{{win_sym, win}, {sum_sym, sum}},
        _exists(win_sym),
        sum_sym);

    return submit("wsum", wsum_op);
}

}  // namespace tilt::tilder
