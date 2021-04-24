#include "../../util/InstType.hpp"

namespace act {

std::string InstType::getITypeName(const unsigned llvm_opCpde){
    if(llvm_opCpde >= llvm::Instruction::BinaryOps::BinaryOpsBegin && llvm_opCpde < llvm::Instruction::BinaryOpsEnd){
        return "Binary";
    }
    else if (llvm_opCpde >= llvm::Instruction::TermOpsBegin && llvm_opCpde < llvm::Instruction::TermOpsEnd){
        return "Terminator";
    }
    else if (llvm_opCpde >= llvm::Instruction::MemoryOpsBegin && llvm_opCpde < llvm::Instruction::MemoryOpsEnd){
        return "Memory";
    }
    else if (llvm_opCpde >= llvm::Instruction::UnaryOpsBegin && llvm_opCpde < llvm::Instruction::UnaryOpsEnd){
        return "Unary";
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

}