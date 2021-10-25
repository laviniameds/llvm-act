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

		virtual bool runOnModule(Module &llvm_module){
			for (auto &F : llvm_module.getFunctionList()){
				for (auto &BB : F.getBasicBlockList()){				
					for (auto &I : BB.getInstList()){
						auto *load_inst = llvm::dyn_cast<llvm::LoadInst>(&I);
						if(load_inst && load_inst->isSafeToRemove() && 
						!load_inst->isVolatile() && load_inst->hasNUndroppableUses(0)){
							if(load_inst_map.find(load_inst) != load_inst_map.end())
								break;
							else
								load_inst_map.insert({load_inst, &F});						
							// load_inst->eraseFromParent();
							// errs() << *load_inst << "was erased\n";
						}
					}
				}
			}
			for(auto &it : load_inst_map){
				errs() << "\n\nFunction: " << it.second->getName() << "\n\n";
				errs() << "This 'load' can be erased: " << *it.first << "\n";
			}

			return false;
		}		
	};

} // namespace llvm

char MemorySkippingPass::ID = 0;

static RegisterPass<MemorySkippingPass> X("memory-skipping", "Memory Skipping Pass");
