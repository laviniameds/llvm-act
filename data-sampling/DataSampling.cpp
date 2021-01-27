#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"

using namespace llvm;
using namespace std;

namespace {
	//define llvm pass
	struct DataSamplingPass : public FunctionPass {
		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		DataSamplingPass() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &llvm_function){
			errs() << "I saw a function called " << llvm_function.getName() << "!\n";

			return false;	    
		}	    			
	};

} // namespace llvm

char DataSamplingPass::ID = 0;

static RegisterPass<DataSamplingPass> X("data-sampling", "DataSampling Pass");