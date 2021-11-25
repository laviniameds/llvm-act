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

namespace {
	//define llvm pass
	struct MemorySkippingPass : public ModulePass {

		std::map <LoadInst*, Function*> load_inst_map;
		std::map <LoadInst*, Function*>::iterator it_load_inst_mapp;
		
		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		MemorySkippingPass() : ModulePass(ID) {}

		//LLVM Analysis
		void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfoWrapperPass>();
			AU.addRequired<ScalarEvolutionWrapperPass>();
			//AU.addRequiredID(LoopSimplifyID);
		}

		void removeLoadValues(){
			errs() << "SIZE LOAD INST MAP: " << load_inst_map.size() << "\n";
			int size = load_inst_map.size()*InputLoopRate.getValue();
			it_load_inst_mapp = load_inst_map.begin();
			for(int i = 0; i < size; i++){
					LoadInst *LI = it_load_inst_mapp->first;
					errs() << "\n\nFunction: " << LI->getName() << "\n\n";
					errs() << "\n\nINST: " << *LI << "TYPE: " << *LI->getType() << "\n\n";
					Constant *value = NULL;

					if(LI->getOperand(0) != NULL){
						if(LI->getType()->isFloatTy()){
							float v = 1;
							value = ConstantFP::get(LI->getType(), v);
						}
						if(LI->getType()->isIntegerTy()){
							int v = 1;
							value = ConstantInt::get(LI->getType(), v);					
						}
						else {
							errs() << "LOAD INST NOT FLOAT OR INT TYPE" << "\n";
						}
						if(value != NULL){
							errs() << "REPLACING " << *LI << " USES WITH " << *value << "\n";
							LI->replaceAllUsesWith(value);
							LI->eraseFromParent();
						}
					}	
					it_load_inst_mapp++;
			}
		}

		virtual bool runOnModule(Module &llvm_module){
			if(InputLoopRate.getValue() != 0.0F){
				for (auto &F : llvm_module.getFunctionList()){
					for (auto &BB : F.getBasicBlockList()){				
						for (auto &I : BB.getInstList()){
							auto *load_inst = llvm::dyn_cast<llvm::LoadInst>(&I);
							auto *const_inst = llvm::dyn_cast<llvm::Constant>(&I);
							if(load_inst){
								if(load_inst->getType()->isFloatTy() || load_inst->getType()->isIntegerTy())
									load_inst_map.insert({load_inst, &F});						
							}
						}
					}
				}	
				removeLoadValues();
			}
			return false;
		}	
	};

} // namespace llvm

char MemorySkippingPass::ID = 0;

static RegisterPass<MemorySkippingPass> X("memory-skipping", "Memory Skipping Pass");
