#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "fstream"
#include "llvm/Support/CommandLine.h"
#include "../../util/opUtil.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#define LLVM_FLOAT_TY llvm::Type::getFloatTy(context)

using namespace llvm;
using namespace act;

//get input loop rate from command line
static cl::opt<float> InputLoopRate("loop_rate",
	cl::desc("Specify input loop_rate for Loop Perforation Pass"),
	cl::value_desc("loop_rate"));

namespace {
	//define llvm pass
	struct PrecisionScalingPass : public ModulePass {
		std::set<Instruction*> I_set;
		std::set<Instruction*>::iterator it_I_set;
		bool FP_type, Int_type;
		
		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		PrecisionScalingPass() : ModulePass(ID) {}

		//LLVM Analysis
		void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<LoopInfoWrapperPass>();
		}

		Type* getHalfType(Instruction &I){
			switch (I.getType()->getScalarSizeInBits()){
			case 16:
				return Type::getInt8Ty(I.getContext());
				break;	
			default:
				return Type::getInt64Ty(I.getContext());	
				break;
			}
		}

		void reduce_int(Instruction *I){
			int v = 2;
			errs() << "I: " << *I << "\n";
			auto *int_value = ConstantInt::get(I->getType(), v);		
			errs() << "int_value: " << *int_value << "\n";

			Instruction *modulo = BinaryOperator::CreateSRem(I, int_value, "");	
			I->insertAfter(modulo);
			errs() << "modulo: " << *modulo << "\n";
			// Instruction	*result = BinaryOperator::CreateSub(int_value, modulo, "");
			// modulo->insertAfter(result);
			// errs() << "result: " << *result << "\n";
			
			// Instruction *sub = BinaryOperator::CreateSub(I, sub, "");
			// result->insertAfter(sub);
			// errs() << "sub: " << *sub << "\n";

			//I->replaceUsesOutsideBlock(sub, I->getParent());
	
			// for(auto user : I->users()){
			// 	if((user != result) && (user != modulo)){
			// 		for(auto &op : user->operands()){
			// 			if(op == I){
			// 				errs() << "CHANGING INT [" << *op << "]" << " TO [" << *sub << "]\n";
			// 				op = sub;
			// 			}
			// 		}
			// 	}
			// }
		}

		void reduce_float(Instruction *I){
			Instruction *fptoint = FPToSIInst::Create(Instruction::CastOps::FPToSI, I, 
			getHalfType(*I), "");
			fptoint->insertAfter(I);
			//errs() << "fptoint: " << *fptoint << "\n";

			Instruction *inttofp = SIToFPInst::Create(Instruction::CastOps::SIToFP, fptoint, 
			I->getType(), "");
			inttofp->insertAfter(fptoint);
			//errs() << "inttofp: " << *inttofp << "\n";

			//I->replaceUsesOutsideBlock(inttofp, I->getParent());
	
			for(auto user : I->users()){
				if((user != fptoint) && (user != inttofp)){
					for(auto &op : user->operands()){
						if(op == I){
							errs() << "CHANGING FLOAT [" << *op << "]" << " TO [" << *inttofp << "]\n";
							op = inttofp;
						}
					}
				}
			}
		}

		void reduce_precision(){
			errs() << "SIZE LOAD INST MAP: " << I_set.size() << "\n";
			int size = I_set.size()*InputLoopRate.getValue();
			int k = 0;
			it_I_set = I_set.begin();

			for(int i = 0; i < size/2; i++){
				while (k < 2 && it_I_set != I_set.end()){
					it_I_set++;
					k++;
				}	
				k = 0;
				auto I = *it_I_set;

				FP_type = I->getType()->isFloatingPointTy();
				Int_type = I->getType()->isIntegerTy();

				if (FP_type){
					reduce_float(I);
				}
			}
		}

		void filter_instructions(Instruction *I){
			auto binary_inst = dyn_cast<BinaryOperator>(I);
			if(binary_inst){	
				I_set.insert(I);		
			}
		}

		virtual bool runOnModule(Module &llvm_module){
			if(InputLoopRate.getValue() != 0.0F){
				for (auto &F : llvm_module.getFunctionList()){
					for (auto &BB : F.getBasicBlockList()){				
						for (auto &I : BB.getInstList()){
							filter_instructions(&I);
							reduce_precision();
						}
					}
				}
			}

			return false;
		}		
	};

} // namespace llvm

char PrecisionScalingPass::ID = 0;

static RegisterPass<PrecisionScalingPass> X("precision-scaling", "Precision Scaling Pass");