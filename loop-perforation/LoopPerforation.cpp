#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "fstream"

using namespace llvm;

namespace {
	//define llvm pass
	struct LoopPerforationPass : public FunctionPass {

		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		LoopPerforationPass() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &F){
			errs() << "I saw a function called " << F.getName() << "!\n";

			return false;	    
		}	    		
		
	};
} // namespace llvm

char LoopPerforationPass::ID = 0;

static RegisterPass<LoopPerforationPass> X("loop-perforation", "LoopPerforation Pass");
