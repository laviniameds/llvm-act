#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace std;

//get input loop rate from command line
static cl::opt<int> InputLoopRate("loop_rate", 
cl::desc("Specify input loop_rate for Loop Perforation Pass"), 
cl::value_desc("loop_rate"));

namespace {
	//define llvm pass
	struct LoopPerforationPass : public FunctionPass {
		
		std::map<Loop*, Function*> f_loop_map;
		std::map<Loop*, Function*>::iterator it_f_loop_map;

		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		LoopPerforationPass() : FunctionPass(ID) {}

		//LLVM Analysis
		void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfoWrapperPass>();
			//AU.addRequiredID(LoopSimplifyID);
		}

		//return true or false if a loop is perforable
		bool isPerforable(Function &llvm_function, Loop *loop){

			//check if loop is simple
			if (!loop->isLoopSimplifyForm()) {
				return false;
			}

			//get the PHI node correspondent to the canonical induction variable
			PHINode *PHI = loop->getCanonicalInductionVariable();

			//if it's null, return false
			if (PHI == nullptr) {
				return false;
			}

			//"find where the induction variable is modified by finding a user that
    		// is also an incoming value to the phi"

			//users: return the related instructions that uses the actual instruction
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
			
			//if it's null, return false
			if (value_to_change == nullptr) {
				return false;
			}

			//if it's not a binary operator, return false
			if (!llvm::dyn_cast<llvm::BinaryOperator>(value_to_change)) {
				return false;
			}

			return true;
		}

		//process a loop
		void processLoop(Function &llvm_function, Loop *loop){
			//check if the actual loop is a good candidate to perforate
			if(isPerforable(llvm_function, loop)){
				//errs() << "That's a perforable loop!" << "!\n";
				f_loop_map.insert({loop, &llvm_function});
			}
			
			//check also the subloops into each loop
			for(Loop *sub_loop : loop->getSubLoops()){
				processLoop(llvm_function, sub_loop);
			}
		}

		//perforate loop
		void perforateLoops(){

			for(it_f_loop_map = f_loop_map.begin(); it_f_loop_map != f_loop_map.end(); ++it_f_loop_map){

				auto loop = it_f_loop_map->first;
				auto llvm_function = it_f_loop_map->second;  

				//get the PHI node correspondent to the canonical induction variable
				PHINode *PHI = loop->getCanonicalInductionVariable();

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
				
				//replace induction variable with new value 'loop rate'
				BinaryOperator *i = dyn_cast<BinaryOperator>(value_to_change);
				for (auto &op : i->operands()) {
					if (op == PHI) continue;

					int loop_rate = 1;
					loop_rate = InputLoopRate.getValue();
					Type *ConstType = op->getType();
					Constant *NewInc = ConstantInt::get(ConstType, loop_rate /*value*/, true /*issigned*/);

					//errs() << "Changing [" << *op << "] to [" << *NewInc << "]!\n";

					op = NewInc;
				}	

			}				
		}
		
		virtual bool runOnFunction(Function &llvm_function){
			//errs() << "I saw a function called " << llvm_function.getName() << "!\n";
			
			f_loop_map.clear();

			LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
			//handle the loops into every function
			for(auto &loop : loop_info){
				processLoop(llvm_function, loop);
			}

			perforateLoops();

			return false;	    
		}	    			
	};

} // namespace llvm

char LoopPerforationPass::ID = 0;

static RegisterPass<LoopPerforationPass> X("loop-perforation", "LoopPerforation Pass");
