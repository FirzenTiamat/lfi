// AArch64AttsfiTraceret.cpp
#include "AArch64.h"
#include "AArch64InstrInfo.h"
#include "AArch64RegisterInfo.h"
#include "AArch64Subtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <fstream>
#include <set>
#include <string>
#include <iostream>
using namespace llvm;

#define BACKEND_TRACERET_FUNCLIST_FILE "passhelper.backendtraceret.funclist"
#define TRACERET_SYMBOL "traceret" 

namespace {
  class AArch64AttsfiTraceret : public MachineFunctionPass {
  public:
    std::set<std::string> backend_traceret_funcs;
    static char ID;
    AArch64AttsfiTraceret() : MachineFunctionPass(ID) {
        initializeAArch64AttsfiTraceretPass(*PassRegistry::getPassRegistry());
    }

    bool readBackendTraceretFuncList() {
        if (backend_traceret_funcs.size() > 0) {
            return true;
        }
        backend_traceret_funcs.insert("main"); //default cases in cflowhj.c
        backend_traceret_funcs.insert("vulnerable_function");
        backend_traceret_funcs.insert("secret_function");

        std::ifstream file(BACKEND_TRACERET_FUNCLIST_FILE);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << BACKEND_TRACERET_FUNCLIST_FILE << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);
            if (!line.empty()) {
                backend_traceret_funcs.insert(line);
            }
        }
        file.close();
        return true;
    }

    bool runOnMachineFunction(MachineFunction &MF) override {
      const AArch64InstrInfo *TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();
      bool Modified = false;
      
      readBackendTraceretFuncList();

      std::string functionName = MF.getName().str();
      if (backend_traceret_funcs.find(functionName) == backend_traceret_funcs.end()) {
          return false;  // Function not in the list, no need to instrument
      }

      for (auto &MBB : MF) {
        for (auto &MI : MBB) {
          if (MI.getOpcode() == AArch64::RET) {
            // mov x27, x0; save the value of x0
            BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::ORRXrs))
            .addReg(AArch64::X27)
            .addReg(AArch64::XZR)
            .addReg(AArch64::X0)
            .addImm(0);

            // mov x0, x30; argument for traceret
            BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::ORRXrs))
            .addReg(AArch64::X0)
            .addReg(AArch64::XZR)
            .addReg(AArch64::LR)
            .addImm(0);

            // mov x26, x30; save the value of x30
            BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::ORRXrs))
            .addReg(AArch64::X26)
            .addReg(AArch64::XZR)
            .addReg(AArch64::LR)
            .addImm(0);

            // bl traceret; call traceret
            BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::BL))
            .addExternalSymbol(TRACERET_SYMBOL);

            // mov x30, x26; restore the value of x30
            BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::ORRXrs))
            .addReg(AArch64::LR)
            .addReg(AArch64::XZR)
            .addReg(AArch64::X26)
            .addImm(0);

            // mov x0, x27; restore the value of x0
            BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::ORRXrs))
            .addReg(AArch64::X0)
            .addReg(AArch64::XZR)
            .addReg(AArch64::X27)
            .addImm(0);

            Modified = true;
          }
        }
      }
      return Modified;
    }
  };
  char AArch64AttsfiTraceret::ID = 0;
}

INITIALIZE_PASS(AArch64AttsfiTraceret, "aarch64-atsfi-traceret",
                "Insert traceret before ret", false, false)

namespace llvm{
FunctionPass *createAArch64AttsfiTraceretPass() {
  return new AArch64AttsfiTraceret();
}
} // namespace llvm