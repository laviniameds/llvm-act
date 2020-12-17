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

		bool isPerforable(Function &llvm_function, Loop *loop){

			//check if loop is simple
			if (!loop->isLoopSimplifyForm()) {
				return false;
			}

			//get the PHI node correspondent to the canonical induction variable
			PHINode *PHI = loop->getCanonicalInductionVariable();

			//if it's null
			if (PHI == nullptr) {
				return false;
			}

			//"find where the induction variable is modified by finding a user that
    		// is also an incoming value to the phi"

			//users: return the return the related instructions that uses the actual instruction
			//incoming values: return the instruction operands

			//define a variable 'value to change'
			Value *value_to_change = nullptr;

			//foreach user from PHI 
			for (auto User : PHI->users()) {
				//foreach incoming value from PHI
				for (auto &incoming : PHI->incoming_values()) {
					//if matches, store the instruction and break
					if (incoming == User) {
						value_to_change = incoming;
						break; 
					}
				}
			}
			
			if (value_to_change == nullptr) {
				return false;
			}

			if (!llvm::dyn_cast<llvm::BinaryOperator>(value_to_change)) {
				return false;
			}

			return true;
		}

		void handleLoop(Function &llvm_function, Loop *loop){
			//check if the actual loop is a good candidate to perforate
			if(isPerforable(llvm_function, loop))
				errs() << "That's a perforable loop!" << "!\n";
			
			//check also the subloops into each loop
			for(Loop *sub_loop : loop->getSubLoops()){
				handleLoop(llvm_function, sub_loop);
			}
		}

		void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfoWrapperPass>();
			//AU.addRequiredID(LoopSimplifyID);
		}
		
		virtual bool runOnFunction(Function &llvm_function){
			errs() << "I saw a function called " << llvm_function.getName() << "!\n";
			
			LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
			//handle the loops into every function
			for(auto &loop : loop_info){
				handleLoop(llvm_function, loop);
			}

			return false;	    
		}	    		
		
	};
} // namespace llvm

char LoopPerforationPass::ID = 0;

static RegisterPass<LoopPerforationPass> X("loop-perforation", "LoopPerforation Pass");
