#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "fstream"
#include "../../util/InstType.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

using namespace llvm;
using namespace act;

namespace {

	//define llvm pass
	struct Profiling : public PassInfoMixin<Profiling>{

		std::string pass_name = "";
		std::map<std::string, float> map_qtd_types;
		std::map<std::string, float>::iterator it;
		int total_qtd_instr = 0;
		std::ofstream file;
		std::string I_type;

		void printResults(){
			for(it = map_qtd_types.begin(); it != map_qtd_types.end(); it++){
				file << it->first << " : " << (it->second*100)/total_qtd_instr << "%\n";
			}
		}

		void runOnLoops(Function &F, FunctionAnalysisManager &FAM){
			LoopInfo LI;
			LI.analyze(FAM.getResult<DominatorTreeAnalysis>(F));

			//TODO: quantify loops
		}

		void runOnFunction(Function &F, FunctionAnalysisManager &FAM){

			//run on function loops
			runOnLoops(F, FAM);

			//run on each function basic block
			for (auto &BB : F.getBasicBlockList()){
				for (auto &I : BB.getInstList()){
						I_type = InstType::getITypeName(I.getOpcode());
						it = map_qtd_types.find(I_type);						
						if(it != map_qtd_types.end())
							it->second++;
						else
							map_qtd_types.insert({I_type, 1});

						total_qtd_instr++;	
					}
				}
		}

		PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM, FunctionAnalysisManager &FAM){
			std::string modulename = M.getSourceFileName().substr(M.getSourceFileName().find_last_of("/")+1);
			std::string filename("./analysis/profiling/results/");
			filename.append(modulename);
			filename.append(".txt");
			file.open(filename.c_str(), std::ios::out);
			file << "Profiling Results of '" << modulename << "'\n\n";

			for(auto &F : M.getFunctionList())
				runOnFunction(F, FAM);
			
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