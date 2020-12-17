#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

using namespace llvm;
using namespace std;

namespace {
	//define llvm pass
	struct LoopPerforationPass : public FunctionPass {

		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		LoopPerforationPass() : FunctionPass(ID) {}

		bool isPerforable(Function &F, Loop *loop){

		}

		void handleLoop(Function &F, Loop *loop){
			if(isPerforable(F, loop))
				errs() << "That's a perforable loop!" << "!\n";

			for(Loop *sub_loop : loop->getSubLoops()){
				handleLoop(F, sub_loop);
			}
		}

		virtual bool runOnFunction(Function &F){
			errs() << "I saw a function called " << F.getName() << "!\n";
			
			LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
			for(auto &loop : loop_info){
				handleLoop(F, loop);
			}

			return false;	    
		}	    		
		
	};
} // namespace llvm

char LoopPerforationPass::ID = 0;

static RegisterPass<LoopPerforationPass> X("loop-perforation", "LoopPerforation Pass");
