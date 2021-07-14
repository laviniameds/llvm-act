#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
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

		//modulo/interleaving
		void perforateModulo(ScalarEvolution &se){
			for(it_f_loop_map = f_loop_map.begin(); 
			it_f_loop_map != f_loop_map.end(); ++it_f_loop_map){
				
				unsigned tripcount = 0;
				unsigned maxtripcount = 0;
				unsigned tripmultiple = 1;
				BasicBlock *exitingblock = it_f_loop_map->first->getLoopLatch();

				exitingblock = it_f_loop_map->first->getExitingBlock();
				if (exitingblock){
					tripcount = se.getSmallConstantMaxTripCount(it_f_loop_map->first);
					tripmultiple = se.getSmallConstantTripMultiple(it_f_loop_map->first);
					errs() << "trip count is " << tripcount <<"\n";
					errs() << "trip multiple is " << tripmultiple <<"\n";
				}
				Optional<Loop::LoopBounds> bounds = it_f_loop_map->first->getBounds(se);
				if (!bounds.hasValue()){
					errs()<< "Did not get the bounds\n";
				}
				else{	
					auto loop = it_f_loop_map->first;
					auto llvm_function = it_f_loop_map->second;  

					//get the PHI node correspondent to the canonical induction variable
					PHINode *PHI = loop->getCanonicalInductionVariable();

					//define a variable 'value to change' and get the loop bound
					Value *value_to_change = &bounds->getFinalIVValue();
					
					//replace the loop bound variable with new value
					BinaryOperator *i = dyn_cast<BinaryOperator>(value_to_change);
					for (auto &op : i->operands()) {
						if (op == PHI) continue;

						string string_value = op->getNameOrAsOperand();
						int int_value = stoi(string_value);

						int loop_rate = 1;
						loop_rate = (1 - InputLoopRate.getValue()) * int_value;
						Type *ConstType = op->getType();
						Constant *NewInc = ConstantInt::get(ConstType, loop_rate /*value*/, true /*issigned*/);

						//errs() << "Changing [" << *op << "] to [" << *NewInc << "]!\n";

						op = NewInc;
					}	
				}

			}			
		}
		
		//truncation
		void perforateTruncation(){
			
		}
		
		//random
		void perforateRandom(){
			
		}		

		//perforate loop
		void perforateLoops(ScalarEvolution &se){

			perforateModulo(se);
				
		}
		
		virtual bool runOnFunction(Function &llvm_function){
			if(!llvm_function.getName().equals_lower("main")){
				//errs() << "I saw a function called " << llvm_function.getName() << "!\n";
				
				f_loop_map.clear();

				LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
				ScalarEvolution &se = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

				//handle the loops into every function
				for(auto &loop : loop_info){
					processLoop(llvm_function, loop);
				}

				perforateLoops(se);
			}

			return false;	    
		}	    			
	};

} // namespace llvm

char LoopPerforationPass::ID = 0;

static RegisterPass<LoopPerforationPass> X("loop-perforation", "LoopPerforation Pass");
