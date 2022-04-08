#include "../../util/passUtil.hpp"

namespace act {

std::string passUtil::getPassName(const std::string instruction_name){
    if(instruction_name == "Memory")
        return "memory-skipping";
    else if(instruction_name == "Binary")
        return "precision-scaling";
    else if(instruction_name == "Control Flow" || instruction_name == "Cast")
        return "loop-perforation";
    else
        return "loop-perforation";
}

}