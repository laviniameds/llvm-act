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
static cl::opt<float> InputLoopRate("loop_rate",
	cl::desc("Specify input loop_rate for Loop Perforation Pass"),
	cl::value_desc("loop_rate"));
	
//get input loop method from command line
static cl::opt<int> InputLoopMethod("loop_method",
	cl::desc("Specify input loop_method for Loop Perforation Pass"),
	cl::value_desc("loop_method"));

namespace
{
	//define llvm pass
	struct LoopPerforationPass : public FunctionPass
	{

		std::map<Loop *, Function *> f_loop_map;
		std::map<Loop *, Function *>::iterator it_f_loop_map;

		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		LoopPerforationPass() : FunctionPass(ID) {}

		//LLVM Analysis
		void getAnalysisUsage(AnalysisUsage &AU) const
		{
			AU.addRequired<LoopInfoWrapperPass>();
			AU.addRequired<ScalarEvolutionWrapperPass>();
			//AU.addRequiredID(LoopSimplifyID);
		}

		//process a loop
		void processLoop(Function &llvm_function, Loop *loop)
		{
			//check if the actual loop is a good candidate to perforate
			if (isPerforable(llvm_function, loop)){
				//errs() << "That's a perforable loop!" << "!\n";
				f_loop_map.insert({loop, &llvm_function});
			}

			//check also the subloops into each loop
			for (Loop *sub_loop : loop->getSubLoops()){
				processLoop(llvm_function, sub_loop);
			}
		}

		//return true or false if a loop is perforable
		//TODO: check perforation for loop bounds
		bool isPerforable(Function &llvm_function, Loop *loop)
		{
			//check if loop is simple
			if (!loop->isLoopSimplifyForm())
				return false;

			//get the PHI node correspondent to the canonical induction variable
			PHINode *PHI = loop->getCanonicalInductionVariable();

			//if it's null, return false
			if (PHI == nullptr)
				return false;
			
			//"find where the induction variable is modified by finding a user that
			// is also an incoming value to the phi"

			//users: return the related instructions that uses the actual instruction
			//incoming values: return the instruction operands

			//define a variable 'value to change'
			Value *value_to_change = nullptr;

			//foreach user from PHI
			for (auto User : PHI->users())
			{
				//foreach incoming value from PHI
				for (auto &incoming : PHI->incoming_values())
				{
					//if matches, store the instruction and break
					if (incoming == User)
					{
						value_to_change = incoming;
						break;
					}
				}
			}

			//if it's null, return false
			if (value_to_change == nullptr)
				return false;

			//if it's not a binary operator, return false
			if (!llvm::dyn_cast<llvm::BinaryOperator>(value_to_change))
				return false;

			return true;
		}

		//truncation perforation
		void perforateTruncation(){
			for (it_f_loop_map = f_loop_map.begin(); it_f_loop_map != f_loop_map.end(); ++it_f_loop_map)
			{
				auto loop = it_f_loop_map->first;
				auto llvm_function = it_f_loop_map->second;

				PHINode *indVar = loop->getCanonicalInductionVariable();
				BinaryOperator *indVarUpdate = NULL;
				ICmpInst *cmp = NULL;

				if (BasicBlock *latchBlock = loop->getExitingBlock())
				{
					for (auto &lbInst : *latchBlock)
					{
						if (auto *exitingBranch = dyn_cast<BranchInst>(&lbInst))
						{
							// branch must have a condition (which sets the loop bound)
							if (exitingBranch->isConditional())
							{
								cmp = dyn_cast<ICmpInst>(exitingBranch->getCondition());
								if (cmp)
								{
									Value *op1 = cmp->getOperand(0);
									Value *op2 = cmp->getOperand(1);
									Value *loopBound = op1 == indVar ? op2 : (op2 == indVar ? op1 : NULL);

									// loop bound must be a constant. otherwise we can't vectorize
									if (loopBound != NULL){
										auto *loopBoundConst = dyn_cast<ConstantInt>(loopBound);
										if (loopBoundConst){
											float loop_rate = 0;
											loop_rate = InputLoopRate.getValue();
											Type *ConstType = loopBound->getType();
											Constant *NewInc = ConstantInt::get(ConstType, (1 - loop_rate) * loopBoundConst->getZExtValue() /*value*/, true /*issigned*/);
											
											for(auto &op : cmp->operands()){
												if(op == indVar) continue;
												//errs() << "Truncation -- Changing [" << *op << "] to [" << *NewInc << "]!\n";
												op = NewInc;
											}
										}
									}
									else
										errs() << "no loop bound found!\n";
								}
							}
							else 
								errs() << "branch with no conditional!\n";
						}
						else 
							errs() << "no exiting branch!\n";
					}
				}
				else 
					errs() << "no latchblock!\n";
			}
		}

		//modulo/interleaving perforation
		void perforateModulo(){
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
					if(InputLoopRate.getValue() > 0 && InputLoopRate.getValue() < 1)
						loop_rate = InputLoopRate.getValue()*10;

					Type *ConstType = op->getType();
					Constant *NewInc = ConstantInt::get(ConstType, loop_rate /*value*/, true /*issigned*/);

					//errs() << "Modulo -- Changing [" << *op << "] to [" << *NewInc << "]!\n";

					op = NewInc;
				}	

			}
		}

		// TODO: perforate random
		// void perforateRandom(){ 	
		// }

		//perforate loop
		void perforateLoops(){
			errs() << InputLoopMethod.getValue() << "\n";
			switch (InputLoopMethod.getValue()){
			case 1:
				perforateTruncation();
				break;
			case 2:
				perforateModulo();
				break;
			default:
				perforateModulo();
				break;
			}
		}

		virtual bool runOnFunction(Function &llvm_function)
		{
			if (!llvm_function.getName().equals_lower("main"))
			{
				//errs() << "I saw a function called " << llvm_function.getName() << "!\n";

				f_loop_map.clear();

				LoopInfo &loop_info = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
				//ScalarEvolution *se = &getAnalysis<ScalarEvolutionWrapperPass>().getSE();

				//handle the loops into every function
				for (auto &loop : loop_info){
					processLoop(llvm_function, loop);
				}

				perforateLoops();
			}

			return false;
		}
	};

} // namespace llvm

char LoopPerforationPass::ID = 0;

static RegisterPass<LoopPerforationPass> X("loop-perforation", "LoopPerforation Pass");
