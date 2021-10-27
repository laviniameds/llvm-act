#include "../../util/opUtil.hpp"

namespace act {

std::string opUtil::getInstructionName(const unsigned llvm_opCpde){
    if(llvm_opCpde >= llvm::Instruction::BinaryOps::BinaryOpsBegin && llvm_opCpde < llvm::Instruction::BinaryOpsEnd){
        return getBinaryName(llvm_opCpde);
    }
    else if (llvm_opCpde >= llvm::Instruction::TermOpsBegin && llvm_opCpde < llvm::Instruction::TermOpsEnd){
        return "Control Flow";
    }
    else if (llvm_opCpde >= llvm::Instruction::MemoryOpsBegin && llvm_opCpde < llvm::Instruction::MemoryOpsEnd){
        return "Memory";
    }
    else if (llvm_opCpde >= llvm::Instruction::UnaryOpsBegin && llvm_opCpde < llvm::Instruction::UnaryOpsEnd){
        return "Binary";
    }
    else if (llvm_opCpde >= llvm::Instruction::CastOpsBegin && llvm_opCpde < llvm::Instruction::CastOpsEnd){
        return "Other";
    }
    else if(llvm_opCpde >= llvm::Instruction::OtherOpsBegin && llvm_opCpde < llvm::Instruction::OtherOpsEnd){
        return getOtherName(llvm_opCpde);
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

std::string opUtil::getTerminatorName(const unsigned llvm_opCpde){
	switch (llvm_opCpde) {
		case llvm::Instruction::TermOps::Ret:
            return "Return";
		    break;
		default:
            return "Terminator";
	}
}

std::string opUtil::getOtherName(const unsigned llvm_opCpde){
	switch (llvm_opCpde) {
		case llvm::Instruction::OtherOps::ICmp:
        case llvm::Instruction::OtherOps::FCmp:
            return "Binary";
            break;
        case llvm::Instruction::OtherOps::InsertValue:
        case llvm::Instruction::OtherOps::ExtractValue:
            return "Memory";
            break;
        case llvm::Instruction::OtherOps::Select:
        case llvm::Instruction::OtherOps::Call:
        case llvm::Instruction::OtherOps::PHI:
            return "Control Flow";
            break;
        // case llvm::Instruction::OtherOps::ExtractElement:
        // case llvm::Instruction::OtherOps::InsertElement:
        // case llvm::Instruction::OtherOps::ShuffleVector:
        //     return "Vector";
            break;
		default:
		  return "Other";
	}
}

}