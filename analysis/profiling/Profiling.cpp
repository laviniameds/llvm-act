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
	int total_loops = 0;
	std::ofstream file;
	std::string I_Name;
	//StringRef *LoopBody = new StringRef("for.body");

	void printResults(){
		for(it = map_qtd_types.begin(); it != map_qtd_types.end(); it++){
			file << it->first << " : " << (it->second*100)/total_qtd_instr << "%\n";
		}
	}
	
	//TODO: quatify loops
	// void processLoop(Loop *loop){			
	// 	total_loops++;

	// 	//check also the subloops into each loop
	// 	for(Loop *sub_loop : loop->getSubLoops())
	// 		processLoop(sub_loop);					
	// }

	// void runOnLoops(Function &F, FunctionAnalysisManager &FAM){
	// 	LoopInfo LI;
	// 	LI.analyze(FAM.getResult<DominatorTreeAnalysis>(F));

	// 	for(auto &loop : LI)
	// 		processLoop(loop);
	// }

	void runOnFunction(Function &F, FunctionAnalysisManager &FAM){
		//run on each function basic block
		for (auto &BB : F.getBasicBlockList()){
			for (auto &I : BB.getInstList()){
				I_Name = opUtil::getInstructionName(I.getOpcode());
				

				it = map_qtd_types.find(I_Name);						
				if(it != map_qtd_types.end())
					it->second++;
				else
					map_qtd_types.insert({I_Name, 1});

				total_qtd_instr++;	
			}
		}

		//run on function loops
		//runOnLoops(F, FAM);
	}

	PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM){
		std::string modulename = M.getSourceFileName().substr(M.getSourceFileName().find_last_of("/")+1);
		std::string filename("./analysis/profiling/results/");
		filename.append(modulename);
		filename.append(".txt");
		file.open(filename.c_str(), std::ios::out);
		file << "Profiling Results of '" << modulename << "'\n\n";

		file << "-------- Function begin---------" << "\n";
		FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

		for(auto &F : M.getFunctionList())
			runOnFunction(F, FAM);				

		printResults();
		file << "-------- Function end---------" << "\n\n";

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