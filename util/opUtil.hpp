#include "llvm/IR/Instructions.h"

namespace act {

class opUtil{
private:
    static std::string getBinaryName(const unsigned llvm_opCpde);  
public:
    static std::string getInstructionName(const unsigned llvm_opCpde);
};

}