#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "fstream"
//#include "llvm/Support/CommandLine.h"
#include "../../util/opUtil.hpp"
#include "../../util/passUtil.hpp"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using namespace act;

//static cl::opt<std::string> InputDirName("name_dir", cl::desc("the input name directory"), cl::value_desc("name_dir"));

namespace {

//define llvm pass
struct Profiling : public PassInfoMixin<Profiling>{

	std::map<std::string, float> map_qtd_types;
	std::map<std::string, float>::iterator it;
	int total_qtd_instr = 0;
	int max = 0;
	float percentage = 0;
	//int total_loops = 0;
	std::ofstream file;
	std::ofstream file_pass;
	std::string I_Name;
	std::string pass_name;
	//std::string name_dir = InputDirName.getValue();

	//StringRef *LoopBody = new StringRef("for.body");
	
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

	void printResults(){
		for(it = map_qtd_types.begin(); it != map_qtd_types.end(); it++){
			percentage = (it->second*100)/total_qtd_instr;
			if(percentage > max){
				max = percentage;
				pass_name = passUtil::getPassName(it->first);
			}
			file << it->first << " : " << percentage << "%\n";
		}
	}

	void runOnFunction(Function &F, FunctionAnalysisManager &FAM){
		//run on each function basic block
		for (auto &BB : F.getBasicBlockList()){
			for (auto &I : BB.getInstList()){
				auto debug_value_instruction = llvm::dyn_cast<llvm::DbgValueInst>(&I);
				if(!debug_value_instruction){
					I_Name = opUtil::getInstructionName(I.getOpcode());				
					if(I_Name != "Other"){
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

		//run on function loops
		//runOnLoops(F, FAM);
	}

	PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM){
		std::string modulename = M.getSourceFileName().substr(M.getSourceFileName().find_last_of("/")+1);
		// std::string path = M.getSourceFileName().substr(0, M.getSourceFileName().find_last_of("/"));
		std::string path = M.getSourceFileName();

		path.append("_profiling.txt");
		file.open(path.c_str(), std::ios::out);
		file << "Profiling Results of '" << modulename << "'\n\n";
		file << "-------- Function begin---------" << "\n";
		
		FunctionAnalysisManager &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
		for(auto &F : M.getFunctionList())
			runOnFunction(F, FAM);				
		printResults();

		file << "-------- Function end---------" << "\n\n";
		file << "Pass Name: " << pass_name << "\n";
		file.close();

		path = M.getSourceFileName();
		path.append("_pass.txt");
		file_pass.open(path.c_str(), std::ios::out);
		file_pass << "Pass Name: " << pass_name << "\n";
		file_pass.close();

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