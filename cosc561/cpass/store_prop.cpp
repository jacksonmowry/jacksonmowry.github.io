#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <cstddef>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#define SRC_IDX 0
#define DST_IDX 1

using namespace llvm;
using namespace std;

typedef map<Value *, Value *> ACPTable;

class BasicBlockInfo {
  public:
    BitVector COPY;
    BitVector KILL;
    BitVector CPIn;
    BitVector CPOut;
    ACPTable ACP;

    BasicBlockInfo(unsigned int max_copies) {
        COPY.resize(max_copies);
        KILL.resize(max_copies);
        CPIn.resize(max_copies);
        CPOut.resize(max_copies);

        COPY.reset();
        KILL.reset();
        CPIn.reset();
        CPOut.set();
    }
};

namespace {
struct StorePropagation : public PassInfoMixin<StorePropagation> {
  private:
    void localStorePropagation(Function &F);
    void globalStorePropagation(Function &F);
    void propagateStores(BasicBlock &bb, ACPTable &acp);

  public:
    static cl::opt<bool> verbose;
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

cl::opt<bool> StorePropagation::verbose(
    "store-prop-verbose",
    cl::desc("Enable verbose output for StorePropagation"), cl::init(false));

PreservedAnalyses StorePropagation::run(Function &F,
                                        FunctionAnalysisManager &AM) {
    if (verbose)
        errs() << "Running StorePropagation on function: " << F.getName()
               << "\n";

    localStorePropagation(F);
    globalStorePropagation(F);

    return PreservedAnalyses::none();
}

/* LLVM attaches an optnone attribute to functions compiled with -O0. This
 * pass removes the optnone attribute so that we can apply the
 * StorePropagation pass to code compiled with -O0.
 */
struct RemoveOptNone : public PassInfoMixin<RemoveOptNone> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &) {
        for (Function &F : M) {
            if (F.hasFnAttribute(Attribute::OptimizeNone)) {
                F.removeFnAttr(Attribute::OptimizeNone);
                if (F.hasFnAttribute(Attribute::NoInline))
                    F.removeFnAttr(Attribute::NoInline);
            }
        }
        return PreservedAnalyses::none();
    }
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "StorePropagation", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, ModulePassManager &MPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "remove-optnone") {
                            MPM.addPass(RemoveOptNone());
                            return true;
                        }
                        return false;
                    });

                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "store-prop") {
                            FPM.addPass(StorePropagation());
                            return true;
                        }
                        return false;
                    });
            }};
}
} // namespace

class DataFlowAnalysis {
  private:
    /* LLVM does not store the position of instructions in the Instruction
     * class, so we create maps of the store instructions to make them
     * easier to use and reference in the BitVector objects
     */
    std::vector<Value *> copies;
    std::map<Value *, int> copy_idx;
    std::map<int, Value *> idx_copy;
    std::map<BasicBlock *, BasicBlockInfo *> bb_info;
    unsigned int nr_copies;

    void addCopy(Value *v);
    void initCopyIdxs(Function &F);
    void initCOPYAndKILLSets(Function &F);
    void initCPInAndCPOutSets(Function &F);
    void initACPs();

  public:
    DataFlowAnalysis(Function &F);
    ACPTable &getACP(BasicBlock &bb);
    void printCopyIdxs();
    void printDFA();
};

/* NOTE: You should not modify any of the code or headers above this line. To
 * complete the pass, fill in the code for the stub functions defined below.
 */

/*
 * propagateStores performs store propagation over the block bb using the
 * associated values in the ACP table. It also removes load instructions if
 * they are no longer useful.
 *
 * Useful tips:
 *
 * Use C++ features to iterate over the instructions in a block, e.g.:
 *   Instruction *iptr;
 *   for (Instruction &ins : bb) {
 *     iptr = &ins;
 *     ...
 *   }
 *
 * You can use isa to determine the type of an instruction, e.g.:
 *   if (isa<StoreInst>(iptr)) {
 *     // iptr points to an Instruction that is a StoreInst
 *   }
 *
 * Other useful LLVM routines:
 *   int  Instruction::getOperand(int)
 *   void Instruction::setOperand(int,int)
 *   int  Instruction::getNumOperands()
 *   void Instruction::eraseFromParent()
 */
void StorePropagation::propagateStores(BasicBlock &bb, ACPTable &acp) {
    vector<Instruction *> instructions_to_erase;
    Instruction *iptr;
    for (Instruction &ins : bb) {
        iptr = &ins;

        if (isa<StoreInst>(iptr)) {
            assert(iptr->getNumOperands() == 2);
            // iptr points to an Instruction that is a StoreInst
            // As we can see in the book if we find a store instruction we first
            // need to check if the LHS is in our acp table If it is we want to
            // remove that entry from the acp table, possibly replacing it if it
            // has a single operand Otherwise we need to check it's RHS, if that
            // is present in the table we can
            Value *lhs = (Value *)iptr->getOperand(1);
            Value *rhs = (Value *)iptr->getOperand(0);
            if (acp.find(rhs) != acp.end()) {
                // RHS exists in table and has not been modified,
                // therefore we should replace it with it's own RHS
                // from its previous assignment
                iptr->setOperand(0, acp[rhs]);
            }

            if (acp.find(lhs) != acp.end()) {
                // LSH exists in table, just remove
                acp.erase(lhs);
            }

            acp[lhs] = rhs;
        } else if (isa<LoadInst>(iptr)) {
            Value *v = iptr->getOperand(0);

            if (acp.find(v) != acp.end()) {
                acp[iptr] = acp[v];
                instructions_to_erase.push_back(iptr);
            }
        } else {
            int num_operands = iptr->getNumOperands();
            for (size_t i = 0; i < num_operands; i++) {
                Value *v = (Value *)iptr->getOperand(i);

                Value *find = v;
                while (true) {
                    if (acp.find(find) != acp.end()) {
                        find = acp[find];
                    } else {
                        break;
                    }
                }

                if (find != v) {
                    iptr->setOperand(i, find);
                }
            }
        }
    }

    for (Instruction *i : instructions_to_erase) {
        i->eraseFromParent();
    }
}

/*
 * localStorePropagation performs local store propagation (LSP) over the basic
 * blocks in the function F. The algorithm for LSP described on pp. 357-358 in
 * the provided text (Muchnick).
 *
 * Useful tips:
 *
 * Use C++ features to iterate over the blocks in F, e.g.:
 *   for (BasicBlock &bb : F) {
 *     ...
 *   }
 *
 * This routine should call propagateStores
 */
void StorePropagation::localStorePropagation(Function &F) {

    size_t blocks = 0;
    for (BasicBlock &bb : F) {
        ACPTable acp;
        propagateStores(bb, acp);
        blocks++;
    }

    if (verbose) {
        errs() << "post local" << "\n" << (*(&F)) << "\n";
    }
}

/*
 * globalStorePropagation performs global store propagation (GSP) over the basic
 * blocks in the function F. The algorithm for GSP is described on pp. 358-360
 * in the provided text (Muchnick).
 *
 * Useful tips:
 *
 * This routine will use the DataFlowAnalysis to construct COPY, KILL, CPIn,
 * and CPOut sets and an ACP table for each block.
 *
 * Use C++ features to iterate over the blocks in F, e.g.:
 *   for (BasicBlock &bb : F) {
 *     ...
 *   }
 *
 * This routine should also call propagateStores
 */
void StorePropagation::globalStorePropagation(Function &F) {
    DataFlowAnalysis *dfa;
    dfa = new DataFlowAnalysis(F);

    // implement global store propagation here
    for (BasicBlock &bb : F) {
        propagateStores(bb, dfa->getACP(bb));
    }

    if (verbose) {
        errs() << "post global" << "\n" << (*(&F)) << "\n";
    }
}

/*
 * addCopy is a helper routine for initCopyIdxs. It updates state information
 * to record the index of a single copy instruction
 */
void DataFlowAnalysis::addCopy(Value *v) {
    copies.push_back(v);
    copy_idx[v] = nr_copies;
    idx_copy[nr_copies] = v;
    nr_copies++;
}

/*
 * initCopyIdxs creates a table that records unique identifiers for each copy
 * (i.e., argument and store) instructions in LLVM.
 *
 * LLVM does not store the position of instructions in the Instruction class,
 * so this routine is used to record unique identifiers for each copy
 * instruction in the Function F. This step makes it easier to identify copy
 * instructions in the COPY, KILL, CPIn, and CPOut sets.
 *
 * Useful tips:
 *
 * You should record function arguments and store instructions as copy
 * instructions.
 *
 * Some useful LLVM routines in this routine are:
 *   Function::arg_iterator Function::arg_begin()
 *   Function::arg_iterator Function::arg_end()
 *   bool llvm::isa<T>(Instruction *)
 */
void DataFlowAnalysis::initCopyIdxs(Function &F) {
    nr_copies = 0;
    for (auto ai = F.arg_begin(); ai != F.arg_end(); ++ai) {
        addCopy(ai);
    }

    Instruction *iptr;
    for (BasicBlock &bb : F) {
        for (Instruction &ins : bb) {
            iptr = &ins;

            if (isa<StoreInst>(iptr)) {
                addCopy(iptr);
            }
        }
    }
}

/*
 * initCOPYAndKILLSets initializes the COPY and KILL sets for each basic block
 * in the function F.
 *
 * Useful tips:
 *
 * This routine should visit the blocks in reverse post order. You can use an
 * LLVM iterator to complete this traversal, e.g.:
 *
 *   BasicBlock *bb;
 *   ReversePostOrderTraversal<Function*> RPOT(&F);
 *   for ( auto BB = RPOT.begin(); BB != RPOT.end(); BB++ ) {
 *       bb = *BB;
 *       ...
 *   }
 *
 * This routine should create BasicBlockInfo objects for each basic block and
 * record the BasicBlockInfo for each block in the bb_info map.
 *
 * Some useful LLVM routines in this routine are:
 *   bool llvm::isa<T>(Instruction *)
 *   int  Instruction::getOperand(int)
 */
void DataFlowAnalysis::initCOPYAndKILLSets(Function &F) {
    BasicBlock *bb;
    ReversePostOrderTraversal<Function *> RPOT(&F);
    for (auto BB = RPOT.begin(); BB != RPOT.end(); ++BB) {
        bb = *BB;

        BasicBlockInfo *bbi = new BasicBlockInfo(nr_copies);

        Instruction *iptr;
        for (Instruction &ins : *bb) {
            iptr = &ins;

            if (isa<StoreInst>(iptr)) {
                Value *lhs = (Value *)iptr->getOperand(1);
                Value *rhs = (Value *)iptr->getOperand(0);

                if (copy_idx.find(iptr) != copy_idx.end()) {
                    bbi->COPY[copy_idx[iptr]] = true;
                    bbi->KILL[copy_idx[iptr]] = false;
                }

                // Find all assignments that this assignment kills
                for (Value *v : copies) {
                    if (v == iptr) {
                        continue;
                    }

                    // Don't kill a copy that was made live within this block
                    if (bbi->COPY[copy_idx[v]]) {
                        continue;
                    }

                    Value *other_lhs = ((Instruction *)v)->getOperand(1);

                    if (lhs == other_lhs) {
                        // We kill this assignment
                        // bbi->COPY[copy_idx[v]] = false;
                        bbi->KILL[copy_idx[v]] = true;
                    }
                }
            }
        }

        bb_info[bb] = bbi;
    }
}

/*
 * initCPInAndCPOutSets initializes the CPIn and CPOut sets for each basic
 * block in the function F.
 *
 * Useful tips:
 *
 * Similar to initCOPYAndKillSets, you will need to traverse the blocks in
 * reverse post order.
 *
 * You can iterate the predecessors and successors of a block bb using
 * LLVM-defined iterators "predecessors" and "successors", e.g.:
 *
 *   for ( BasicBlock* pred : predecessors( bb ) ) {
 *       // pred points to a predecessor of bb
 *       ...
 *   }
 *
 * You will need to define a special case for the entry block (and some way to
 * identify the entry block).
 *
 * Use set operations on the appropriate BitVector to create CPIn and CPOut.
 */
void DataFlowAnalysis::initCPInAndCPOutSets(Function &F) {
    BasicBlock *bb;
    ReversePostOrderTraversal<Function *> RPOT(&F);
    // Pass 1
    for (auto BB = RPOT.begin(); BB != RPOT.end(); ++BB) {
        bb = *BB;
        BasicBlockInfo *bbi = bb_info[bb];

        size_t num_predecessors = 0;
        bool first = true;
        for (BasicBlock *pred : predecessors(bb)) {
            num_predecessors++;
            if (first) {
                bbi->CPIn = bb_info[pred]->CPOut;
                first = false;
            } else {
                bbi->CPIn &= bb_info[pred]->CPOut;
            }
        }

        if (num_predecessors == 0) {
            // We're the entry block
            // And COPY with CPOut
            bbi->CPOut &= bbi->COPY;
        } else {
            BitVector tmp_out = bbi->CPIn;
            tmp_out.reset(bbi->KILL);
            bbi->CPOut = bbi->COPY;
            bbi->CPOut |= tmp_out;
        }

        for (BasicBlock *succ : successors(bb)) {
            bb_info[succ]->CPIn &= bbi->CPOut;

            BitVector tmp_out = bb_info[succ]->CPIn;
            tmp_out.reset(bb_info[succ]->KILL);
            bb_info[succ]->CPOut = bb_info[succ]->COPY;
            bb_info[succ]->CPOut |= tmp_out;
        }
    }
    for (auto BB = RPOT.begin(); BB != RPOT.end(); ++BB) {
        bb = *BB;
        BasicBlockInfo *bbi = bb_info[bb];

        size_t num_predecessors = 0;
        bool first = true;
        for (BasicBlock *pred : predecessors(bb)) {
            num_predecessors++;
            if (first) {
                bbi->CPIn = bb_info[pred]->CPOut;
                first = false;
            } else {
                bbi->CPIn &= bb_info[pred]->CPOut;
            }
        }

        if (num_predecessors == 0) {
            // We're the entry block
            // And COPY with CPOut
            bbi->CPOut &= bbi->COPY;
        } else {
            BitVector tmp_out = bbi->CPIn;
            tmp_out.reset(bbi->KILL);
            bbi->CPOut = bbi->COPY;
            bbi->CPOut |= tmp_out;
        }

        for (BasicBlock *succ : successors(bb)) {
            bb_info[succ]->CPIn &= bbi->CPOut;

            BitVector tmp_out = bb_info[succ]->CPIn;
            tmp_out.reset(bb_info[succ]->KILL);
            bb_info[succ]->CPOut = bb_info[succ]->COPY;
            bb_info[succ]->CPOut |= tmp_out;
        }
    }
}

/*
 * initACPs creates an ACP table for each basic block, which will be used to
 * conduct global copy propagation.
 *
 * Useful tips:
 *
 * You will need to use CPIn to determine if a copy should be in the ACP for
 * this block.
 */
void DataFlowAnalysis::initACPs() {
    for (auto BB = bb_info.begin(); BB != bb_info.end(); ++BB) {
        BasicBlockInfo *bbi = BB->second;

        for (size_t i = 0; i < bbi->CPIn.size(); i++) {
            if (bbi->CPIn[i]) {
                bbi->ACP[((Instruction *)copies[i])->getOperand(1)] =
                    ((Instruction *)copies[i])->getOperand(0);
            }
        }
    }
}

ACPTable &DataFlowAnalysis::getACP(BasicBlock &bb) {
    return bb_info[(&bb)]->ACP;
}

void DataFlowAnalysis::printCopyIdxs() {
    errs() << "copy_idx:" << "\n";
    for (auto it = copy_idx.begin(); it != copy_idx.end(); ++it) {
        errs() << "  " << format("%-3d", it->second) << " --> " << *(it->first)
               << "\n";
    }
    errs() << "\n";
}

void DataFlowAnalysis::printDFA() {
    unsigned int i;

    // used for formatting
    std::string str;
    llvm::raw_string_ostream rso(str);

    for (auto it = bb_info.begin(); it != bb_info.end(); ++it) {
        BasicBlockInfo *bbi = bb_info[it->first];

        errs() << "BB ";
        it->first->printAsOperand(errs(), false);
        errs() << "\n";

        errs() << "  CPIn  ";
        for (i = 0; i < bbi->CPIn.size(); i++) {
            errs() << bbi->CPIn[i] << ' ';
        }
        errs() << "\n";

        errs() << "  CPOut ";
        for (i = 0; i < bbi->CPOut.size(); i++) {
            errs() << bbi->CPOut[i] << ' ';
        }
        errs() << "\n";

        errs() << "  COPY  ";
        for (i = 0; i < bbi->COPY.size(); i++) {
            errs() << bbi->COPY[i] << ' ';
        }
        errs() << "\n";

        errs() << "  KILL  ";
        for (i = 0; i < bbi->KILL.size(); i++) {
            errs() << bbi->KILL[i] << ' ';
        }
        errs() << "\n";

        errs() << "  ACP:" << "\n";
        for (auto it = bbi->ACP.begin(); it != bbi->ACP.end(); ++it) {
            rso << *(it->first);
            errs() << "  " << format("%-30s", rso.str().c_str())
                   << "==  " << *(it->second) << "\n";
            str.clear();
        }
        errs() << "\n" << "\n";
    }
}

/*
 * DataFlowAnalysis constructs the data flow analysis for the function F.
 *
 * You will not need to modify this routine.
 */
DataFlowAnalysis::DataFlowAnalysis(Function &F) {
    initCopyIdxs(F);
    initCOPYAndKILLSets(F);
    initCPInAndCPOutSets(F);
    initACPs();

    if (StorePropagation::verbose) {
        errs() << "post DFA" << "\n";
        printCopyIdxs();
        printDFA();
    }
}
