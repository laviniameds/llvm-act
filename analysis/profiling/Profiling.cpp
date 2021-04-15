#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "fstream"

using namespace llvm;

namespace{
	std::string pass_name = "";

	struct Profiling : public PassInfoMixin<Profiling>{
		void runOnBasicBlocks(Function &F){
			for (auto &BB : F.getBasicBlockList()){
				for (auto &I : BB.getInstList()){
					//check instruction type
				}
			}
		}

		//define llvm pass
		PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM){
			std::ofstream file;

			//cont_bb = 0;
			file.clear();
			std::string filename("./results/");
			filename.append(pass_name);
			filename.append(".txt");
			file.open(filename.c_str(), std::ios::out);

			//run on each function basic block 
			runOnBasicBlocks(F);

			file.close();
			return PreservedAnalyses::all();
		}
	};

} // namespace llvm

llvm::PassPluginLibraryInfo getProfilingPluginInfo(){
	return {LLVM_PLUGIN_API_VERSION, "Profiling", LLVM_VERSION_STRING,
			[](PassBuilder &PB) {
				PB.registerPipelineParsingCallback(
					[](StringRef Name, FunctionPassManager &FPM,
					   ArrayRef<PassBuilder::PipelineElement>) {
						if (Name == "profiling"){
							FPM.addPass(Profiling());
							return true;
						}
						return false;
					});
			}};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo(){
	return getProfilingPluginInfo();
}