#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "fstream"

using namespace llvm;

namespace{
	//define llvm pass
	struct Profiling : public PassInfoMixin<Profiling>{
		std::string pass_name = "";
		std::map<std::string, float> map_qtd_types;
		std::map<std::string, float>::iterator it;
		int total_qtd_instr = 0;
		std::ofstream file;
		std::string I_type;

		std::string getITypeName(const unsigned llvm_opCpde){
			if(llvm_opCpde >= llvm::Instruction::BinaryOps::BinaryOpsBegin && llvm_opCpde < llvm::Instruction::BinaryOpsEnd){
				return "Binary";
			}
			else if (llvm_opCpde >= llvm::Instruction::TermOpsBegin && llvm_opCpde < llvm::Instruction::TermOpsEnd){
				return "Terminator";
			}
			else if (llvm_opCpde >= llvm::Instruction::MemoryOpsBegin && llvm_opCpde < llvm::Instruction::MemoryOpsEnd){
				return "Memory";
			}
			else if (llvm_opCpde >= llvm::Instruction::UnaryOpsBegin && llvm_opCpde < llvm::Instruction::UnaryOpsEnd){
				return "Unary";
			}
			else if (llvm_opCpde >= llvm::Instruction::CastOpsBegin && llvm_opCpde < llvm::Instruction::CastOpsEnd){
				return "Cast";
			}
			else if(llvm_opCpde >= llvm::Instruction::OtherOpsBegin && llvm_opCpde < llvm::Instruction::OtherOpsEnd){
				return "Other";
			}
			else
				return "Undefined";
		}

		void runOnFunction(Function &F){
			//run on each function basic block
			for (auto &BB : F.getBasicBlockList()){
				for (auto &I : BB.getInstList()){
						I_type = getITypeName(I.getOpcode());
						it = map_qtd_types.find(I_type);						
						if(it != map_qtd_types.end())
							it->second++;
						else
							map_qtd_types.insert({I_type, 1});

						total_qtd_instr++;	
					}
				}
		}

		void printResults(){
			for(it = map_qtd_types.begin(); it != map_qtd_types.end(); it++){
				file << it->first << " : " << (it->second*100)/total_qtd_instr << "%\n";
			}
		}

		PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM){
			std::string modulename = M.getSourceFileName().substr(M.getSourceFileName().find_last_of("/")+1);
			std::string filename("./analysis/profiling/results/");
			filename.append(modulename);
			filename.append(".txt");
			file.open(filename.c_str(), std::ios::out);
			file << "Profiling Results of '" << modulename << "'\n\n";

			for(auto &F : M.getFunctionList())
				runOnFunction(F);

			printResults();

			file.close();
			return PreservedAnalyses::all();
		}
	};

} // namespace llvm

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo(){
	return {
		LLVM_PLUGIN_API_VERSION, "Profiling", "v0.1",
		[](PassBuilder &PB) {
			PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MAM,
				   ArrayRef<PassBuilder::PipelineElement>) {
					if (Name == "profiling"){
						MAM.addPass(Profiling());
						return true;
					}
					return false;
				});
		}};
}