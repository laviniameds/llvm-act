#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/LoopPass.h"
#include "fstream"
#include "../../util/opUtil.hpp"
#include "../../util/passUtil.hpp"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using namespace std;

namespace
{
	//define llvm pass
	struct ProfilingPass : public ModulePass{

		std::ofstream file;
		std::string I_Name;
		std::map<std::string, float> map_qtd_types;
		std::map<std::string, float>::iterator it;
		int total_qtd_instr = 0;

		//define pass ID
		static char ID;
		//define derivate from a ModulePass class
		ProfilingPass() : ModulePass(ID) {}

		//LLVM Analysis
		void getAnalysisUsage(AnalysisUsage &AU) const{
			AU.addRequired<LoopInfoWrapperPass>();
			//AU.addRequired<ScalarEvolutionWrapperPass>();
		}

		void printResults(){
			for(it = map_qtd_types.begin(); it != map_qtd_types.end(); it++){
				// percentage = (it->second*100)/total_qtd_instr;
				// if(percentage > max){
				// 	max = percentage;
				// 	pass_name = passUtil::getPassName(it->first);
				// }
				file << it->first << ",";
			}
			file << "\n";
			for(it = map_qtd_types.begin(); it != map_qtd_types.end(); it++)
				file << it->second << ",";	
			file << "\n";
		}

		//return true or false if a loop is perforable
		//TODO: check perforation for loop bounds
		bool isPerforable(Loop *Loop){
			//check if loop is simple
			if (!Loop->isLoopSimplifyForm()){
				errs() << "Not a simple loop" << "\n";
				return false;
			}

			//get the PHI node correspondent to the canonical induction variable
			PHINode *PHI = Loop->getCanonicalInductionVariable();

			//if it's null, return false
			if (PHI == nullptr){
				errs() << "PHI is null" << "\n";
				return false;
			}
			
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
			if (value_to_change == nullptr){
				errs() << "null value to change" << "\n";
				return false;
			}

			//if it's not a binary operator, return false
			if (!llvm::dyn_cast<llvm::BinaryOperator>(value_to_change)){
				errs() << "not a binary operator" << "\n";
				return false;
			}

			return true;
		}

		//process a loop
		void processLoop(Loop *Loop){
			//check if the actual loop is a good candidate to perforate
			if (isPerforable(Loop)){
				I_Name = "Control Flow";
				it = map_qtd_types.find(I_Name);						
				if(it != map_qtd_types.end())
					it->second++;
				else
					map_qtd_types.insert({I_Name, 1});
				total_qtd_instr++;
			}

			//check also the subloops into each loop
			for (auto *sub_loop : Loop->getSubLoops()){
				processLoop(sub_loop);
			}
		}

		virtual bool runOnModule(Module &M){
			map_qtd_types.insert({"Binary", 0});
			map_qtd_types.insert({"Control Flow", 0});
			map_qtd_types.insert({"Memory", 0});

			std::string modulename = M.getSourceFileName().substr(M.getSourceFileName().find_last_of("/")+1);	
			std::string path = M.getSourceFileName().substr(0, M.getSourceFileName().find_last_of("/"));
			path.append("/profiling_results/");
			path.append(modulename);
			path.append("_profiling.csv");
			file.open(path.c_str(), std::ios::out);

			//run on each function basic block
			for(auto &F : M.getFunctionList()){

				if(!F.isDeclaration()){
					LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
					//ScalarEvolution &SE = getAnalysis<ScalarEvolutionWrapperPass>().getSE();

					for (auto &Loop : LI){
						processLoop(Loop);
					}
				}

				for (auto &BB : F.getBasicBlockList()){
					for (auto &I : BB.getInstList()){
						auto *load_inst = llvm::dyn_cast<llvm::LoadInst>(&I);
						auto *binary_inst = llvm::dyn_cast<BinaryOperator>(&I);

						if(load_inst){
							if(load_inst->getType()->isFloatTy() || load_inst->getType()->isIntegerTy()){
								I_Name = "Memory";
								it = map_qtd_types.find(I_Name);						
								if(it != map_qtd_types.end())
									it->second++;
								else
									map_qtd_types.insert({I_Name, 1});
								total_qtd_instr++;	
							}
						}
						if(binary_inst){
							if(binary_inst->getType()->isFloatingPointTy()){	
								I_Name = "Binary";
								it = map_qtd_types.find(I_Name);						
								if(it != map_qtd_types.end())
									it->second++;
								else
									map_qtd_types.insert({I_Name, 1});
								total_qtd_instr++;							
							}
						}
					}
				}
			}
			printResults();
			file.close();

			return false;
		}
	};

} // namespace llvm

char ProfilingPass::ID = 0;

static RegisterPass<ProfilingPass> X("profiling-pass", "Profiling Pass");