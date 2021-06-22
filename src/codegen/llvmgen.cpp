#include "tilt/base/type.h"
#include "tilt/codegen/llvmgen.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"

using namespace std;
using namespace tilt;
using namespace llvm;

Function* LLVMGen::llfunc(const string name, llvm::Type* ret_type, vector<llvm::Type*> arg_types)
{
    auto fn = llmod()->getFunction(name);
    if (!fn) {
        auto fn_type = FunctionType::get(ret_type, arg_types, false);
        fn = Function::Create(fn_type, Function::ExternalLinkage, name, llmod());
    }

    return fn;
}

Value* LLVMGen::llcall(const string name, llvm::Type* ret_type, vector<Value*> arg_vals)
{
    vector<llvm::Type*> arg_types;
    for (const auto& arg_val: arg_vals) {
        arg_types.push_back(arg_val->getType());
    }
    auto fn = llfunc(name, ret_type, arg_types);
    return builder()->CreateCall(fn, arg_vals);
}

Value* LLVMGen::llcall(const string name, llvm::Type* ret_type, vector<ExprPtr> args)
{
    vector<Value*> arg_vals;
    for (const auto& arg: args) {
        arg_vals.push_back(eval(arg));
    }

    return llcall(name, ret_type, arg_vals);
}

llvm::Type* LLVMGen::lltype(const PrimitiveType& btype)
{
    switch (btype)
    {
    case PrimitiveType::BOOL:
        return llvm::Type::getInt1Ty(llctx());
    case PrimitiveType::CHAR:
        return llvm::Type::getInt8Ty(llctx());
    case PrimitiveType::INT8:
    case PrimitiveType::UINT8:
        return llvm::Type::getInt8Ty(llctx());
    case PrimitiveType::INT16:
    case PrimitiveType::UINT16:
        return llvm::Type::getInt16Ty(llctx());
    case PrimitiveType::INT32:
    case PrimitiveType::UINT32:
        return llvm::Type::getInt32Ty(llctx());
    case PrimitiveType::INT64:
    case PrimitiveType::UINT64:
        return llvm::Type::getInt64Ty(llctx());
    case PrimitiveType::FLOAT32:
        return llvm::Type::getFloatTy(llctx());
    case PrimitiveType::FLOAT64:
        return llvm::Type::getDoubleTy(llctx());
    case PrimitiveType::TIMESTAMP:
        return llvm::Type::getInt64Ty(llctx());
    case PrimitiveType::TIME:
        return llvm::Type::getInt64Ty(llctx());
    case PrimitiveType::INDEX:
        static auto idx_type = llvm::StructType::create(
            llctx(),
            {
                lltype(types::TIME),                            // time
                lltype(types::INT32),                           // index
            },
            "index_t"
        );
        return idx_type;
    case PrimitiveType::UNKNOWN:
    default:
        throw std::runtime_error("Invalid type");
    }
}

llvm::Type* LLVMGen::lltype(const vector<PrimitiveType>& btypes, const bool is_ptr)
{
    vector<llvm::Type*> lltypes;
    for (auto btype: btypes) {
        lltypes.push_back(lltype(btype));
    }

    llvm::Type* type;
    if (btypes.size() > 1) {
        type = StructType::get(llctx(), lltypes);
    } else {
        type = lltypes[0];
    }

    if (is_ptr) {
        type = PointerType::get(type, 0);
    }

    return type;
}

llvm::Type* LLVMGen::lltype(const Type& type)
{
    if (type.isLStream()) {
        static auto reg_type = StructType::create(
            llctx(),
            {
                lltype(types::INDEX),                           // start index
                lltype(types::INDEX),                           // end index
                lltype(types::INDEX.ptypes, true),              // timeline array
                lltype(types::INT8.ptypes, true),               // data buffer
            },
            "region_t"
        );
        return PointerType::get(reg_type, 0);
    } else {
        return lltype(type.dtype);
    }
}

Value* LLVMGen::visit(const Symbol& symbol)
{
    auto tmp_sym = get_sym(symbol);
    return sym(map_sym(tmp_sym));
}

Value* LLVMGen::visit(const IfElse& ifelse)
{
    auto cond = eval(ifelse.cond);
    auto true_body = eval(ifelse.true_body);
    auto false_body = eval(ifelse.false_body);
    return builder()->CreateSelect(cond, true_body, false_body);
}


Value* LLVMGen::visit(const IConst& iconst)
{
    return ConstantInt::getSigned(lltype(iconst), iconst.val);
}

Value* LLVMGen::visit(const UConst& uconst)
{
    return ConstantInt::get(lltype(uconst), uconst.val);
}

Value* LLVMGen::visit(const FConst& fconst)
{
    return ConstantFP::get(lltype(fconst), fconst.val);
}

Value* LLVMGen::visit(const CConst& cconst)
{
    return ConstantInt::get(lltype(cconst), cconst.val);
}

Value* LLVMGen::visit(const TConst& tconst)
{
    return ConstantInt::get(lltype(tconst), tconst.val);
}

Value* LLVMGen::visit(const Add& add)
{
    return builder()->CreateAdd(eval(add.Left()), eval(add.Right()));
}

Value* LLVMGen::visit(const Sub& sub)
{
    return builder()->CreateSub(eval(sub.Left()), eval(sub.Right()));
}

Value* LLVMGen::visit(const Max& max)
{
    auto left = eval(max.Left());
    auto right = eval(max.Right());
    auto ge = builder()->CreateICmpSGE(left, right);
    return builder()->CreateSelect(ge, left, right);
}

Value* LLVMGen::visit(const Min& min)
{
    auto left = eval(min.Left());
    auto right = eval(min.Right());
    auto le = builder()->CreateICmpSLE(left, right);
    return builder()->CreateSelect(le, left, right);
}

Value* LLVMGen::visit(const Now&) { throw std::runtime_error("Invalid expression"); }

Value* LLVMGen::visit(const Exists& exists)
{
    return builder()->CreateIsNull(eval(exists.sym));
}

Value* LLVMGen::visit(const Equals& equals)
{
    return builder()->CreateICmpEQ(eval(equals.Left()), eval(equals.Right()));
}

Value* LLVMGen::visit(const Not& not_expr)
{
    return builder()->CreateNot(eval(not_expr.Input()));
}

Value* LLVMGen::visit(const And& and_expr)
{
    return builder()->CreateAnd(eval(and_expr.Left()), eval(and_expr.Right()));
}

Value* LLVMGen::visit(const Or& or_expr)
{
    return builder()->CreateOr(eval(or_expr.Left()), eval(or_expr.Right()));
}

Value* LLVMGen::visit(const True&)
{
    return ConstantInt::getTrue(llctx());
}

Value* LLVMGen::visit(const False&)
{
    return ConstantInt::getFalse(llctx());
}

Value* LLVMGen::visit(const LessThan& lt)
{
    return builder()->CreateICmpSLT(eval(lt.Left()), eval(lt.Right()));
}

Value* LLVMGen::visit(const LessThanEqual& lte)
{
    return builder()->CreateICmpSLE(eval(lte.Left()), eval(lte.Right()));
}

Value* LLVMGen::visit(const GreaterThan& gt)
{
    return builder()->CreateICmpSGT(eval(gt.Left()), eval(gt.Right()));
}

Value* LLVMGen::visit(const AggExpr&) { throw std::runtime_error("Invalid expression"); }

Value* LLVMGen::visit(const GetTime& get_time)
{
    return llcall("get_time", lltype(get_time), { eval(get_time.idx) });
}

Value* LLVMGen::visit(const Fetch& fetch)
{
    auto ret_type = lltype({PrimitiveType::INT8}, true);
    auto addr = llcall("fetch", ret_type, { fetch.reg, fetch.idx });
    return builder()->CreateBitCast(addr, lltype(fetch.reg->type.dtype.ptypes, true));
}

Value* LLVMGen::visit(const Advance& adv)
{
    return llcall("advance", lltype(adv), { adv.reg, adv.idx, adv.time });
}

Value* LLVMGen::visit(const Next& next)
{
    return llcall("next", lltype(next), { next.reg, next.idx });
}

Value* LLVMGen::visit(const GetStartIdx& start_idx)
{
    return builder()->CreateStructGEP(eval(start_idx.reg), 0);
}

Value* LLVMGen::visit(const CommitNull& commit)
{
    return llcall("commit_null", lltype(commit), { commit.reg, commit.time });
}

Value* LLVMGen::visit(const CommitData& commit)
{
    auto reg_val = eval(commit.reg);
    auto t_val = eval(commit.time);

    auto data_val = eval(commit.data);
    auto data_ptr = builder()->CreateAlloca(lltype(commit.data));
    auto local_ptr = builder()->CreateBitCast(data_ptr, lltype(types::CHAR.ptypes, true));
    builder()->CreateStore(data_val, data_ptr);

    auto data_size = llmod()->getDataLayout().getTypeSizeInBits(data_val->getType()).getFixedSize();
    auto size_val = ConstantInt::get(lltype(types::UINT64), data_size);
    return llcall("commit_data", lltype(commit), { reg_val, t_val, local_ptr, size_val });
}


Value* LLVMGen::visit(const AllocIndex& alloc_idx)
{
    auto init_val = builder()->CreateLoad(eval(alloc_idx.init_idx));
    auto idx_ptr = builder()->CreateAlloca(lltype(types::INDEX));
    builder()->CreateStore(init_val, idx_ptr);
    return idx_ptr;
}

Value* LLVMGen::visit(const Load& load)
{
    auto ptr_val = eval(load.ptr);
    return builder()->CreateLoad(ptr_val);
}

Value* LLVMGen::visit(const AllocRegion&) { throw std::runtime_error("Invalid expression"); }
Value* LLVMGen::visit(const MakeRegion&) { throw std::runtime_error("Invalid expression"); }

Value* LLVMGen::visit(const Call& call)
{
    return llcall(call.loop->GetName(), lltype(call), call.args);
}

Value* LLVMGen::visit(const Loop& loop)
{
    vector<llvm::Type*> args_type;
    for (const auto& input: loop.inputs) {
        args_type.push_back(lltype(input->type));
    }

    auto loop_fn = llfunc(loop.GetName(), lltype(loop.output), args_type);

    auto preheader_bb = BasicBlock::Create(llctx(), "preheader");
    auto header_bb = BasicBlock::Create(llctx(), "header");
    auto body_bb = BasicBlock::Create(llctx(), "body");
    auto end_bb = BasicBlock::Create(llctx(), "end");
    auto exit_bb = BasicBlock::Create(llctx(), "exit");

    for (size_t i = 0; i < loop.inputs.size(); i++) {
        auto input = loop.inputs[i];
        sym(input) = loop_fn->getArg(i);
        sym(input)->setName(input->name);
    }

    /* Initial values of base states */
    loop_fn->getBasicBlockList().push_back(preheader_bb);
    builder()->SetInsertPoint(preheader_bb);
    map<SymPtr, llvm::Value*> base_inits;
    for (const auto& [state_sym, state]: loop.states) {
        base_inits[state.base] = eval(state.init);
    }
    builder()->CreateBr(header_bb);

    /* Phi nodes for base states */
    loop_fn->getBasicBlockList().push_back(header_bb);
    builder()->SetInsertPoint(header_bb);
    for (const auto& [base_sym, val]: base_inits) {
        auto base = builder()->CreatePHI(lltype(base_sym->type), 2, base_sym->name);
        sym(base_sym) = base;
        base->addIncoming(val, preheader_bb);
    }

    /* Update timer */
    sym(loop.t) = eval(loop.states.at(loop.t).update);
    auto exit_cond = eval(loop.exit_cond);
    builder()->CreateCondBr(exit_cond, exit_bb, body_bb);

    loop_fn->getBasicBlockList().push_back(body_bb);
    builder()->SetInsertPoint(body_bb);
    /* Update indices */
    for (const auto& idx: loop.idxs) {
        sym(idx) = eval(loop.states.at(idx).update);
    }

    /* Loop body */
    sym(loop.output) = eval(loop.states.at(loop.output).update);
    builder()->CreateBr(end_bb);

    /* Update states and loop back to header */
    loop_fn->getBasicBlockList().push_back(end_bb);
    builder()->SetInsertPoint(end_bb);
    for (const auto& [state_sym, state]: loop.states) {
        auto base = dyn_cast<PHINode>(sym(state.base));
        base->addIncoming(sym(state_sym), end_bb);
    }
    builder()->CreateBr(header_bb);

    /* Exit the loop */
    loop_fn->getBasicBlockList().push_back(exit_bb);
    builder()->SetInsertPoint(exit_bb);
    builder()->CreateRet(sym(loop.states.at(loop.output).base));

    return loop_fn;
}
