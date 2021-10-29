#ifndef INCLUDE_TILT_PASS_OPT_IRPASS_H_
#define INCLUDE_TILT_PASS_OPT_IRPASS_H_

#include <stdexcept>
#include <utility>

#include "tilt/pass/irgen.h"

using namespace std;

namespace tilt {

class IRPassCtx : public IRGenCtx<Expr, Expr> {
public:
    IRPassCtx(Sym sym, const OpNode* op, Op outop) :
        IRGenCtx(sym, &op->syms, &outop->syms), op(op), outop(outop)
    {}

private:
    const OpNode* op;
    const Op outop;

    friend class IRPass;
};

class IRPass : public IRGen<IRPassCtx, Expr, Expr> {
public:
    explicit IRPass(IRPassCtx ctx) : _ctx(move(ctx)) {}

    static Op Build(Sym, const OpNode*);

protected:
    IRPassCtx& ctx() override { return _ctx; }
    static Op Build(Sym, const OpNode*, Params);
    virtual void build_op();

    Expr visit(const Symbol&) override;
    Expr visit(const Out&) override;
    Expr visit(const Beat&) override;
    Expr visit(const Call&) override;
    Expr visit(const IfElse&) override;
    Expr visit(const Select&) override;
    Expr visit(const Get&) override;
    Expr visit(const New&) override;
    Expr visit(const Exists&) override;
    Expr visit(const ConstNode&) override;
    Expr visit(const Cast&) override;
    Expr visit(const NaryExpr&) override;
    Expr visit(const SubLStream&) override;
    Expr visit(const Element&) override;
    Expr visit(const OpNode&) override;
    Expr visit(const Reduce&) override;
    Expr visit(const Fetch&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Read&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Write&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Advance&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetCkpt&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetStartIdx&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetEndIdx&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetStartTime&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetEndTime&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const CommitData&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const CommitNull&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const AllocRegion&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const MakeRegion&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const LoopNode&) final { throw runtime_error("Invalid expression"); };

    IRPassCtx _ctx;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_PASS_OPT_IRPASS_H_
