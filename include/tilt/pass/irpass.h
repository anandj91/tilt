#ifndef INCLUDE_TILT_PASS_IRPASS_H_
#define INCLUDE_TILT_PASS_IRPASS_H_

#include <map>
#include <memory>
#include <utility>

#include "tilt/pass/visitor.h"
#include "tilt/builder/tilder.h"

using namespace std;

namespace tilt {

template<typename CtxTy, typename InExprTy>
class IRPass;

template<typename InExprTy>
class IRPassCtx {
protected:
    IRPassCtx(Sym sym, const map<Sym, InExprTy>* in_sym_tbl) : sym(sym), in_sym_tbl(in_sym_tbl) {}

    Sym sym;
    const map<Sym, InExprTy>* in_sym_tbl;
    map<Sym, Sym> sym_map;

    template<typename CtxTy, typename InTy>
    friend class IRPass;
};

template<typename CtxTy, typename InExprTy>
class IRPass : public Visitor {
protected:
    virtual CtxTy& ctx() = 0;

    CtxTy& switch_ctx(CtxTy& new_ctx) { swap(new_ctx, ctx()); return new_ctx; }

    Sym tmp_sym(const Symbol& symbol)
    {
        shared_ptr<Symbol> tmp(const_cast<Symbol*>(&symbol), [](Symbol*) {});
        return tmp;
    }

    Sym& get_sym(const Sym& in_sym) { return ctx().sym_map.at(in_sym); }
    Sym& get_sym(const Symbol& symbol) { return get_sym(tmp_sym(symbol)); }
    void set_sym(const Sym& in_sym, const Sym out_sym) { ctx().sym_map[in_sym] = out_sym; }
    void set_sym(const Symbol& in_symbol, const Sym out_sym) { set_sym(tmp_sym(in_symbol), out_sym); }
    void set_sym(const Sym& in_sym) { set_sym(in_sym, in_sym); }

    void Visit(const Symbol& symbol) override
    {
        auto tmp = tmp_sym(symbol);

        if (ctx().sym_map.find(tmp) == ctx().sym_map.end()) {
            auto expr = ctx().in_sym_tbl->at(tmp);

            swap(ctx().sym, tmp);
            expr->Accept(*this);
            swap(tmp, ctx().sym);

            set_sym(tmp);
        }
    }
};

}  // namespace tilt

#endif  // INCLUDE_TILT_PASS_IRPASS_H_
