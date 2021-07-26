#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#define LLVM_FLOAT_TY llvm::Type::getFloatTy(context)

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
				errs() << "That's a perforable loop: " << *loop << "!\n\n";
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
			vector<Instruction *> vector_lb;
			for (it_f_loop_map = f_loop_map.begin(); it_f_loop_map != f_loop_map.end(); ++it_f_loop_map){
				auto loop = it_f_loop_map->first;
				auto llvm_function = it_f_loop_map->second;

				PHINode *indVar = loop->getCanonicalInductionVariable();
				BinaryOperator *indVarUpdate = NULL;
				ICmpInst *cmp = NULL;

				for(auto &bb : loop->getBlocksVector()){
					for (auto &lbInst : *bb){
						auto *exitingBranch = dyn_cast<BranchInst>(&lbInst);
						if (exitingBranch){
							//branch must have a condition (which sets the loop bound)
							if (exitingBranch->isConditional()){		
								cmp = dyn_cast<ICmpInst>(exitingBranch->getCondition());						
								if (cmp){
									Value* loopBound;
									for(auto &op : cmp->operands()){
										if(auto *iv = dyn_cast<PHINode>(&op)) indVar = iv;
										else loopBound = op;
									}

									if (loopBound != NULL){
										errs() << "cmp: " << *cmp << "\n\n";
										errs() << "loopBound: " << *loopBound << "\n\n";									
										if (auto *loopBoundConst = dyn_cast<Instruction>(loopBound)){	

											errs() << "loopBoundConst: " << *loopBoundConst << "\n";
											errs() << "indVar: " << *indVar << "\n";									

											//float y = (1 - perforation_rate)
											float value = (1 - InputLoopRate.getValue());
											//errs() << "value: " << value << "\n\n";
											//if(v <= 0) v = 1;
											auto *y = ConstantFP::get(Type::getFloatTy(bb->getContext()), value);					
											errs() << "y: " << *y << "\n";

											// (float)(loopBoundConst)
											Instruction *loopBoundConst_float = SIToFPInst::Create(Instruction::CastOps::SIToFP, 
											loopBoundConst,	Type::getFloatTy(bb->getContext()), "");
											
											loopBoundConst_float->insertAfter(loopBoundConst);

											errs() << "loopBoundConst_float: " << *loopBoundConst_float << "\n";
											
											// float k = (y * loopBoundConst_float)
											Instruction *k = BinaryOperator::CreateFMul(y, loopBoundConst_float, "");
											k->insertAfter(loopBoundConst_float);

											errs() << "k: " << *k << "\n";

											//(int)(k)
											Instruction *temp = FPToUIInst::Create(Instruction::CastOps::FPToUI, k,
											Type::getInt32Ty(bb->getContext()), "");
											temp->insertAfter(k);

											errs() << "temp: " << *temp << "\n";

											// // auto *zero = ConstantInt::get(Type::getInt32Ty(bb->getContext()), 0);					
											// // errs() << "zero: " << *zero << "\n";

											// // //int h = (int)(k)
											// // Instruction *h = BinaryOperator::CreateAdd(temp, zero, "");
											// // bb->getInstList().insert(bb->getFirstInsertionPt()->getIterator(), h);
											
											// //loopBoundConst->replaceAllUsesWith(temp);

											for(auto &op : cmp->operands()){
												if(op == indVar) continue;
												errs() << "Truncation -- Changing [" << *op << "] to [" << *temp << "]!\n";
												op = temp;
											}
											// //loopBoundConst->replaceAllUsesWith(temp);

											errs() << "new_cmp" << *cmp << "\n\n";
												
										}
										else if(auto* loopBoundConst = dyn_cast<ConstantInt>(loopBound)){
											float loop_rate = 0;
											loop_rate = InputLoopRate.getValue();
											int value = (1 - loop_rate) * loopBoundConst->getZExtValue();
											if(value <= 0) value = 1;

											Type *ConstType = loopBound->getType();
											Constant *NewInc = ConstantInt::get(ConstType, value /*value*/, true /*issigned*/);
											
											for(auto &op : cmp->operands()){
												if(op == indVar) continue;
												errs() << "Truncation -- Changing [" << *op << "] to [" << *NewInc << "]!\n\n";
												op = NewInc;
											}
										}
										else if(auto* loopBoundConst = dyn_cast<Argument>(loopBound)){
											//TODO
											errs() << "loop bound type argument!" << "\n";
										}
										else{
											errs() << "loop bound type not found!" << "\n";
										}	
									}
									else
										errs() << "no loop bound found!\n";
								}
							}
							// else
							// 	errs() << "exiting branch has no conditional!";
						}
						// else
						// 	errs() << "no exiting branch!\n";
					}
					// for(auto *loopBoundConst : vector_lb){
	
					// }
				}		
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

					errs() << "Modulo -- Changing [" << *op << "] to [" << *NewInc << "]!\n";

					op = NewInc;
				}	

			}
		}

		// TODO: perforate random
		// void perforateRandom(){ 	
		// }

		//perforate loop
		void perforateLoops(){
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
