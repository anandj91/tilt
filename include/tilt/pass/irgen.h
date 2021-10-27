#ifndef INCLUDE_TILT_PASS_IRGEN_H_
#define INCLUDE_TILT_PASS_IRGEN_H_

#include <map>
#include <memory>
#include <utility>

#include "tilt/pass/irpass.h"
#include "tilt/builder/tilder.h"

using namespace std;

namespace tilt {

template<typename CtxTy, typename InExprTy, typename OutExprTy>
class IRGen;

template<typename InExprTy, typename OutExprTy>
class IRGenCtx : public IRPassCtx<InExprTy> {
protected:
    IRGenCtx(Sym sym, const map<Sym, InExprTy>* in_sym_tbl, map<Sym, OutExprTy>* out_sym_tbl) :
        IRPassCtx<InExprTy>(sym, in_sym_tbl), out_sym_tbl(out_sym_tbl)
    {}

    map<Sym, OutExprTy>* out_sym_tbl;
    OutExprTy val;

    template<typename CtxTy, typename InTy, typename OutTy>
    friend class IRGen;
};

template<typename CtxTy, typename InExprTy, typename OutExprTy>
class IRGen : public IRPass<CtxTy, InExprTy> {
protected:
    virtual OutExprTy visit(const Symbol&) = 0;
    virtual OutExprTy visit(const Out&) = 0;
    virtual OutExprTy visit(const Beat&) = 0;
    virtual OutExprTy visit(const IfElse&) = 0;
    virtual OutExprTy visit(const Select&) = 0;
    virtual OutExprTy visit(const Get&) = 0;
    virtual OutExprTy visit(const New&) = 0;
    virtual OutExprTy visit(const Exists&) = 0;
    virtual OutExprTy visit(const ConstNode&) = 0;
    virtual OutExprTy visit(const Cast&) = 0;
    virtual OutExprTy visit(const NaryExpr&) = 0;
    virtual OutExprTy visit(const SubLStream&) = 0;
    virtual OutExprTy visit(const Element&) = 0;
    virtual OutExprTy visit(const OpNode&) = 0;
    virtual OutExprTy visit(const Reduce&) = 0;
    virtual OutExprTy visit(const Fetch&) = 0;
    virtual OutExprTy visit(const Read&) = 0;
    virtual OutExprTy visit(const Write&) = 0;
    virtual OutExprTy visit(const Advance&) = 0;
    virtual OutExprTy visit(const GetCkpt&) = 0;
    virtual OutExprTy visit(const GetStartIdx&) = 0;
    virtual OutExprTy visit(const GetEndIdx&) = 0;
    virtual OutExprTy visit(const GetStartTime&) = 0;
    virtual OutExprTy visit(const GetEndTime&) = 0;
    virtual OutExprTy visit(const CommitData&) = 0;
    virtual OutExprTy visit(const CommitNull&) = 0;
    virtual OutExprTy visit(const AllocRegion&) = 0;
    virtual OutExprTy visit(const MakeRegion&) = 0;
    virtual OutExprTy visit(const Call&) = 0;
    virtual OutExprTy visit(const LoopNode&) = 0;

    void Visit(const Out& expr) final { val() = visit(expr); }
    void Visit(const Beat& expr) final { val() = visit(expr); }
    void Visit(const IfElse& expr) final { val() = visit(expr); }
    void Visit(const Select& expr) final { val() = visit(expr); }
    void Visit(const Get& expr) final { val() = visit(expr); }
    void Visit(const New& expr) final { val() = visit(expr); }
    void Visit(const Exists& expr) final { val() = visit(expr); }
    void Visit(const ConstNode& expr) final { val() = visit(expr); }
    void Visit(const Cast& expr) final { val() = visit(expr); }
    void Visit(const NaryExpr& expr) final { val() = visit(expr); }
    void Visit(const SubLStream& expr) final { val() = visit(expr); }
    void Visit(const Element& expr) final { val() = visit(expr); }
    void Visit(const OpNode& expr) final { val() = visit(expr); }
    void Visit(const Reduce& expr) final { val() = visit(expr); }
    void Visit(const Fetch& expr) final { val() = visit(expr); }
    void Visit(const Read& expr) final { val() = visit(expr); }
    void Visit(const Write& expr) final { val() = visit(expr); }
    void Visit(const Advance& expr) final { val() = visit(expr); }
    void Visit(const GetCkpt& expr) final { val() = visit(expr); }
    void Visit(const GetStartIdx& expr) final { val() = visit(expr); }
    void Visit(const GetEndIdx& expr) final { val() = visit(expr); }
    void Visit(const GetStartTime& expr) final { val() = visit(expr); }
    void Visit(const GetEndTime& expr) final { val() = visit(expr); }
    void Visit(const CommitData& expr) final { val() = visit(expr); }
    void Visit(const CommitNull& expr) final { val() = visit(expr); }
    void Visit(const AllocRegion& expr) final { val() = visit(expr); }
    void Visit(const MakeRegion& expr) final { val() = visit(expr); }
    void Visit(const Call& expr) final { val() = visit(expr); }
    void Visit(const LoopNode& expr) final { val() = visit(expr); }

    OutExprTy get_expr(const Sym& sym) { auto& m = *(this->ctx().out_sym_tbl); return m.at(sym); }
    OutExprTy get_expr(const Symbol& symbol) { return get_expr(this->tmp_sym(symbol)); }

    virtual void set_expr(const Sym& sym, OutExprTy val)
    {
        this->set_sym(sym, sym);
        auto& m = *(this->ctx().out_sym_tbl);
        m[sym] = val;
    }
    void set_expr(const Symbol& symbol, OutExprTy val) { set_expr(this->tmp_sym(symbol), val); }

    OutExprTy& val() { return this->ctx().val; }

    OutExprTy eval(const InExprTy expr)
    {
        OutExprTy val = nullptr;

        swap(val, this->ctx().val);
        expr->Accept(*this);
        swap(this->ctx().val, val);

        return val;
    }

    void Visit(const Symbol& symbol) final
    {
        auto tmp = this->tmp_sym(symbol);

        if (this->ctx().sym_map.find(tmp) == this->ctx().sym_map.end()) {
            auto expr = this->ctx().in_sym_tbl->at(tmp);

            swap(this->ctx().sym, tmp);
            auto value = eval(expr);
            swap(tmp, this->ctx().sym);

            auto sym_clone = tilder::_sym(symbol);
            this->set_sym(tmp, sym_clone);
            this->set_expr(sym_clone, value);
        }

        val() = visit(symbol);
    }
};

}  // namespace tilt

#endif  // INCLUDE_TILT_PASS_IRGEN_H_
