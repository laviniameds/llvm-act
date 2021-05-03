#include "../../util/opUtil.hpp"

namespace act {

std::string opUtil::getInstructionName(const unsigned llvm_opCpde){
    if(llvm_opCpde >= llvm::Instruction::BinaryOps::BinaryOpsBegin && llvm_opCpde < llvm::Instruction::BinaryOpsEnd){
        return opUtil::getBinaryName(llvm_opCpde);
    }
    else if (llvm_opCpde >= llvm::Instruction::TermOpsBegin && llvm_opCpde < llvm::Instruction::TermOpsEnd){
        return "Control Flow";
    }
    else if (llvm_opCpde >= llvm::Instruction::MemoryOpsBegin && llvm_opCpde < llvm::Instruction::MemoryOpsEnd){
        return "Memory";
    }
    else if (llvm_opCpde >= llvm::Instruction::UnaryOpsBegin && llvm_opCpde < llvm::Instruction::UnaryOpsEnd){
        return "Float Point";
    }
    else if (llvm_opCpde >= llvm::Instruction::CastOpsBegin && llvm_opCpde < llvm::Instruction::CastOpsEnd){
        return "Cast";
    }
    else if(llvm_opCpde >= llvm::Instruction::OtherOpsBegin && llvm_opCpde < llvm::Instruction::OtherOpsEnd){
        return "Other";
    }
    else
        return "Undefined";
}

std::string opUtil::getBinaryName(const unsigned llvm_opCpde){
	switch (llvm_opCpde) {
		case llvm::Instruction::BinaryOps::FAdd:
		case llvm::Instruction::BinaryOps::FDiv:
		case llvm::Instruction::BinaryOps::FMul:
		case llvm::Instruction::BinaryOps::FRem:
		case llvm::Instruction::BinaryOps::FSub:
		case llvm::Instruction::BinaryOps::UDiv:
		  return "Float Point";
		  break;
		default:
		  return "Integer";
	}
}



}