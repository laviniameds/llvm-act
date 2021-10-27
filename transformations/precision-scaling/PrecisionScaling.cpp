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

		// Constant *ConstantFoldSSEConvertToInt(const APFloat &Val, bool roundTowardZero, Type *Ty) {
		// 	// All of these conversion intrinsics form an integer of at most 64bits.
		// 	unsigned ResultWidth = Ty->getIntegerBitWidth();
		// 	assert(ResultWidth <= 64 && "Can only constant fold conversions to 64 and 32 bit ints");

		// 	uint64_t UIntVal;
		// 	bool isExact = false;
		// 	APFloat::roundingMode mode = roundTowardZero? APFloat::rmTowardZero
		// 												: APFloat::rmNearestTiesToEven;
		// 	APFloat::opStatus status = Val.convertToInteger(&UIntVal, ResultWidth, /*isSigned=*/true, mode,	&isExact);
		// 	if (status != APFloat::opOK && (!roundTowardZero || status != APFloat::opInexact))
		// 		return nullptr;
			
		// 	return ConstantInt::get(Ty, UIntVal, /*isSigned=*/true);
		// }

		void reduce_float(Instruction* I){
			errs() << "TODO: reduce float =>" << *I << "\n";

			// //convert instruction: int16 k = (int16) return value
			// Instruction *temp = FPToSIInst::Create(Instruction::CastOps::FPToSI, return_inst->getReturnValue(),
			// Type::getInt16Ty(I->getContext()), "");
			// temp->insertBefore(return_inst);
			// errs() << "temp: " << *temp << "\n";
			// //replace	
			// for(auto &op : I->operands()){
			// 	errs() << "op: " << *op << "\n";
			// 	errs() << "CHANGING [" << *op << "] TO [" << *temp << "]\n";
			// 	op = temp;
			// }
			// errs() << "reduce float =>" << *I << "\n";
		}

		void reduce_integer(Instruction* I){
			errs() << "TODO: reduce integer =>" << *I << "\n";
		}

		void reduce_precision(){
			while (!I_set.empty()){
				auto I = *I_set.begin();
				I_set.erase(I_set.begin());
				
				FP_type = I->getType()->isFloatingPointTy();
				Int_type = I->getType()->isIntegerTy();

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
			auto return_inst = dyn_cast<BinaryOperator>(I);
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