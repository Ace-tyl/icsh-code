#pragma once

#include "common.h"
#include "register.h"
#include "memory.h"

namespace virtual_machine_nsp {

enum kOpcodeList {
    O_ADD = 0b0001,
    O_AND = 0b0101,
    O_BR  = 0b0000,
    O_JMP = 0b1100,
    O_JSR = 0b0100,
    O_LD  = 0b0010,
    O_LDI = 0b1010,
    O_LDR = 0b0110,
    O_LEA = 0b1110,
    O_NOT = 0b1001,
    O_RTI = 0b1000,
    O_ST  = 0b0011,
    O_STI = 0b1011,
    O_STR = 0b0111,
    O_TRAP = 0b1111
};

enum kTrapRoutineList {
};

class virtual_machine_tp {
    public:
    register_tp reg;
    memory_tp mem;
    
    // Instructions
    void VM_ADD(lc3_t inst);
    void VM_AND(lc3_t inst);
    void VM_BR(lc3_t inst);
    void VM_JMP(lc3_t inst);
    void VM_JSR(lc3_t inst);
    void VM_LD(lc3_t inst);
    void VM_LDI(lc3_t inst);
    void VM_LDR(lc3_t inst);
    void VM_LEA(lc3_t inst);
    void VM_NOT(lc3_t inst);
    void VM_RTI(lc3_t inst);
    void VM_ST(lc3_t inst);
    void VM_STI(lc3_t inst);
    void VM_STR(lc3_t inst);
    void VM_TRAP(lc3_t inst);

    // Managements
    virtual_machine_tp() {}
    virtual_machine_tp(const lc3_t address, const std::string &memfile, const std::string &regfile);
    void UpdateCondRegister(int reg);
    void SetReg(const register_tp &new_reg);
    lc3_t NextStep(std::vector<int>&, bool);
};

}; // virtual machine namespace
