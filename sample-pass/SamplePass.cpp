#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/RegionPass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"

using namespace llvm;

namespace {
	//define llvm pass
	struct SamplePass : public ModulePass {
		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		SamplePass() : ModulePass(ID) {}

		void SamplePass::getAnalysisUsage(AnalysisUsage &Info) const {
			Info.addRequired<RegionInfoPass>();
			Info.setPreservesAll();
		}

		virtual bool runOnModule(ModulePass &llvm_module){
			auto RI = &getAnalysis<RegionInfoPass>().getRegionInfo();

			

			return false;	    
		}	    			
	};

} // namespace llvm

char SamplePass::ID = 0;

static RegisterPass<SamplePass> X("sample-pass", "Sample Pass");