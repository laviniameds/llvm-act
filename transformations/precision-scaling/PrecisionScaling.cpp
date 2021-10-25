#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "fstream"
#include "../../util/opUtil.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"

using namespace llvm;
using namespace act;

namespace {
	//define llvm pass
	struct PrecisionScalingPass : public ModulePass {
		std::set<Instruction*> I_set;
		bool FP_type, Int_type;
		
		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		PrecisionScalingPass() : ModulePass(ID) {}

		//LLVM Analysis
		void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfoWrapperPass>();
		}

		void reduce_float(Instruction* I){
			errs() << "TODO: reduce float =>" << *I << "\n";
		}

		void reduce_integer(Instruction* I){
			errs() << "TODO: reduce integer =>" << *I << "\n";
		}

		void reduce_precision(){
			while (!I_set.empty()){
				errs() << "size: " << I_set.size() << "\n";
				auto I = dyn_cast<llvm::ReturnInst>(*I_set.begin());
				I_set.erase(I_set.begin());
				FP_type = I->getReturnValue()->getType()->isDoubleTy();
				Int_type = I->getReturnValue()->getType()->isIntegerTy();

				if (FP_type){
					reduce_float(I);
				}
				else if(Int_type){
					reduce_integer(I);
				}	
				else errs() << "not int ot FP\n";
			}
		}

		void filter_instructions(Instruction *I){
			auto return_inst = dyn_cast<llvm::ReturnInst>(I);
			if (return_inst){		
				I_set.insert(I);				
			}
		}

		virtual bool runOnModule(Module &llvm_module){
			for (auto &F : llvm_module.getFunctionList()){
				for (auto &BB : F.getBasicBlockList()){				
					for (auto &I : BB.getInstList()){
						filter_instructions(&I);
						reduce_precision();
					}
				}
			}

			return false;
		}		
	};

} // namespace llvm

char PrecisionScalingPass::ID = 0;

static RegisterPass<PrecisionScalingPass> X("precision-scaling", "Precision Scaling Pass");