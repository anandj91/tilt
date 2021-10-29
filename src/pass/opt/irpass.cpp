#include "tilt/pass/opt/irpass.h"

using namespace tilt;
using namespace tilt::tilder;
using namespace std;

Expr IRPass::visit(const Symbol& symbol) { return get_sym(symbol); }

Expr IRPass::visit(const Out& out) { return _out(out); }

Expr IRPass::visit(const Beat& beat) { return _beat(beat); }

Expr IRPass::visit(const Call& call)
{
    vector<Expr> new_args;
    for (auto arg : call.args) {
        new_args.push_back(eval(arg));
    }

    return _call(call.name, call.type, move(new_args));
}

Expr IRPass::visit(const IfElse& ifelse)
{
    return _ifelse(eval(ifelse.cond), eval(ifelse.true_body), eval(ifelse.false_body));
}

Expr IRPass::visit(const Select& sel)
{
    return _ifelse(eval(sel.cond), eval(sel.true_body), eval(sel.false_body));
}

Expr IRPass::visit(const Get& get)
{
    return _get(eval(get.input), get.n);
}

Expr IRPass::visit(const New& newexpr)
{
    vector<Expr> new_inputs;
    for (auto input : newexpr.inputs) {
        new_inputs.push_back(eval(input));
    }

    return _new(move(new_inputs));
}

Expr IRPass::visit(const Exists& exists)
{
    eval(exists.sym);
    return _exists(get_sym(exists.sym));
}

Expr IRPass::visit(const ConstNode& cnst)
{
    return _const(cnst);
}

Expr IRPass::visit(const Cast& cast)
{
    return _cast(cast.type.dtype, eval(cast.arg));
}

Expr IRPass::visit(const NaryExpr& nary)
{
    vector<Expr> args;
    for (auto arg : nary.args) {
        args.push_back(eval(arg));
    }
    return make_shared<NaryExpr>(nary.type.dtype, nary.op, move(args));
}

Expr IRPass::visit(const SubLStream& subls)
{
    eval(subls.lstream);
    return _subls(get_sym(subls.lstream), subls.win);
}

Expr IRPass::visit(const Element& elem)
{
    eval(elem.lstream);
    return _elem(get_sym(elem.lstream), elem.pt);
}

Op IRPass::Build(Sym sym, const OpNode* op)
{
    Params new_inputs;
    for (auto input : op->inputs) {
        auto new_input = _sym(input->name, input->type);
        new_inputs.push_back(new_input);
    }

    return IRPass::Build(sym, op, move(new_inputs));
}

Op IRPass::Build(Sym sym, const OpNode* op, Params new_inputs)
{
    auto out_op = _op(op->iter, new_inputs, SymTable(), _true(), nullptr, Aux());
    IRPassCtx new_ctx(sym, op, out_op);
    IRPass pass(move(new_ctx));

    pass.build_op();
    return move(out_op);
}

void IRPass::build_op()
{
    for (size_t i = 0; i < ctx().op->inputs.size(); i++) {
        set_sym(ctx().op->inputs[i], ctx().outop->inputs[i]);
    }

    ctx().outop->pred = eval(ctx().op->pred);
    eval(ctx().op->output);
    ctx().outop->output = get_sym(ctx().op->output);
}

Expr IRPass::visit(const OpNode& op)
{
    Params new_inputs;
    for (auto input : ctx().op->inputs) {
        eval(input);
        new_inputs.push_back(get_sym(input));
    }
    return IRPass::Build(ctx().sym, &op, move(new_inputs));
}

Expr IRPass::visit(const Reduce& red)
{
    eval(red.lstream);
    return _red(get_sym(red.lstream), eval(red.state), red.acc);
}
