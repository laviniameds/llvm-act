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
	//define llvm pass
	struct Profiling : public PassInfoMixin<Profiling>{
		// void runOnBasicBlocks(Function &F){
		// 	for (auto &BB : F.getBasicBlockList()){
		// 		for (auto &I : BB.getInstList()){
		// 			//check if instruction is valid to approximate
		// 		}
		// 	}
		// }

		PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
		{
			if (F.hasName())
				errs() << "Hello " << F.getName() << "\n";
			//runOnBasicBlocks(F);
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