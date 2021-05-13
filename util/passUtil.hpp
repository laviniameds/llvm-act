#include "llvm/IR/Instruction.h"

namespace act {

class passUtil{
private: 
public:
    static std::string getPassName(const std::string instruction_name);
};

}