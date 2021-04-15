#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Analysis/RegionPass.h"
#include "llvm/Support/raw_ostream.h"
#include "fstream"

using namespace llvm;

namespace {
	//define llvm pass
	struct Profiling : public ModulePass {
		//define pass ID
		static char ID;
		//define derivate from a FunctionPass class
		Profiling() : ModulePass(ID) {}

		std::deque<Region *> RQ;

		void Profiling::getAnalysisUsage(AnalysisUsage &Info) const {
			Info.addRequired<RegionInfoPass>();
			Info.setPreservesAll();
		}

		bool isValidInst(Instruction &instruction){
			if(instruction.isDebugOrPseudoInst())
				return false;
			if (instruction.isBinaryOp())
				return true;
		}

		// Recurse through all subregions and all regions  into RQ.
		static void addRegionIntoQueue(Region &R, std::deque<Region *> &RQ) {
			RQ.push_back(&R);
			for (const auto &E : R)
				addRegionIntoQueue(*E, RQ);
		}

		void runOnBasicBlocks(Region *R){
			for (auto *basic_block : R->blocks()){
				for(auto &instruction : basic_block->getInstList()){
					//check if instruction is valid to approximate
					if (isValidInst(instruction)){
						//
					}
				}
			}
		}

		void runOnRegions(std::deque<Region *> &RQ){
			while (!RQ.empty()) {
				auto current_region = RQ.back();
				runOnBasicBlocks(current_region);
				RQ.pop_back();
			}			
		}

		virtual bool runOnModule(Module &llvm_module){
			auto RI = &getAnalysis<RegionInfoPass>().getRegionInfo();

			//TODO: Global variables

			//collect regions
   			addRegionIntoQueue(*RI->getTopLevelRegion(), RQ);
			
			//run on regions
			runOnRegions(RQ);

			return false;	    
		}			
	};

} // namespace llvm

char Profiling::ID = 0;

static RegisterPass<Profiling> X("profiling", "Profiling Pass");