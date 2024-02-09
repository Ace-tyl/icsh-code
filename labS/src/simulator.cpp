#include "simulator.h"

namespace virtual_machine_nsp {
template <typename T, unsigned B>
inline T SignExtend(const T x) {
    // Extend the number
    return !((x >> (B - 1)) & 1) ? x : -(-x & ((1 << B) - 1));
}

void virtual_machine_tp::UpdateCondRegister(int regname) {
    // Update the condition register
    reg[R_COND] = !reg[regname] ? 2 : ((reg[regname] >> 15) ? 4 : 1);
}

void virtual_machine_tp::VM_ADD(lc3_t inst) {
    int flag = inst & 0b100000;
    int dr = (inst >> 9) & 0x7;
    int sr1 = (inst >> 6) & 0x7;
    if (flag) {
        // add inst number
        lc3_t imm = SignExtend<lc3_t, 5>(inst & 0b11111);
        reg[dr] = reg[sr1] + imm;
    } else {
        if ((inst >> 3) & 0x3) {
            std::cerr << "Invalid ADD command\n";
            exit(-1);
        }
        // add register
        int sr2 = inst & 0x7;
        reg[dr] = reg[sr1] + reg[sr2];
    }
    // Update condition register
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_AND(lc3_t inst) {
    int flag = inst & 0b100000;
    int dr = (inst >> 9) & 0x7;
    int sr1 = (inst >> 6) & 0x7;
    if (flag) {
        // add inst number
        lc3_t imm = SignExtend<lc3_t, 5>(inst & 0b11111);
        reg[dr] = reg[sr1] & imm;
    } else {
        if ((inst >> 3) & 0x3) {
            std::cerr << "Invalid AND command\n";
            exit(-5);
        }
        // add register
        int sr2 = inst & 0x7;
        reg[dr] = reg[sr1] & reg[sr2];
    }
    // Update condition register
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_BR(lc3_t inst) {
    lc3_t pc_offset = SignExtend<lc3_t, 9>(inst & 0x1FF);
    lc3_t cond_flag = (inst >> 9) & 0x7;
    if (gIsDetailedMode) {
        std::cout << reg[R_PC] << std::endl;
        std::cout << pc_offset << std::endl;
    }
    if (cond_flag & reg[R_COND]) {
        reg[R_PC] += pc_offset;
    }
}

void virtual_machine_tp::VM_JMP(lc3_t inst) {
    if (((inst >> 9) & 0x7) || (inst & 0x3F)) {
        std::cerr << "Invalid JMP command\n";
        exit(-12);
    }
    int dr = (inst >> 6) & 0x7;
    reg[R_PC] = reg[dr];
}

void virtual_machine_tp::VM_JSR(lc3_t inst) {
    lc3_t temp = reg[R_PC];
    if ((inst >> 11) & 1) {
        lc3_t pc_offset = SignExtend<lc3_t, 11>(inst & 0x7FF);
        reg[R_PC] += pc_offset;
    }
    else {
        if (((inst >> 9) & 0x7) || (inst & 0x3F)) {
            std::cerr << "Invalid JSRR command\n";
            exit(-4);
        }
        int dr = (inst >> 6) & 0x7;
        reg[R_PC] = reg[dr];
    }
    reg[7] = temp;
}

void virtual_machine_tp::VM_LD(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    lc3_t pc_offset = SignExtend<lc3_t, 9>(inst & 0x1FF);
    reg[dr] = mem[reg[R_PC] + pc_offset];
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_LDI(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    lc3_t pc_offset = SignExtend<lc3_t, 9>(inst & 0x1FF);
    reg[dr] = mem[mem[reg[R_PC] + pc_offset]];
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_LDR(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    int sr = (inst >> 6) & 0x7;
    lc3_t offset = SignExtend<lc3_t, 6>(inst & 0x3F);
    reg[dr] = mem[reg[sr] + offset];
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_LEA(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    lc3_t pc_offset = SignExtend<lc3_t, 9>(inst & 0x1FF);
    reg[dr] = reg[R_PC] + pc_offset;
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_NOT(lc3_t inst) {
    if ((inst & 0x3F) != 0x3F) {
        std::cerr << "Invalid NOT command\n";
        exit(-9);
    }
    int dr = (inst >> 9) & 0x7;
    int sr1 = (inst >> 6) & 0x7;
    reg[dr] = ~reg[sr1];
    // Update condition register
    UpdateCondRegister(dr);
}

void virtual_machine_tp::VM_RTI(lc3_t inst) {
    std::cerr << "RTI Access violation!\n";
    exit(-8);
}

void virtual_machine_tp::VM_ST(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    lc3_t pc_offset = SignExtend<lc3_t, 9>(inst & 0x1FF);
    mem[reg[R_PC] + pc_offset] = reg[dr];
}

void virtual_machine_tp::VM_STI(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    lc3_t pc_offset = SignExtend<lc3_t, 9>(inst & 0x1FF);
    mem[mem[reg[R_PC] + pc_offset]] = reg[dr];
}

void virtual_machine_tp::VM_STR(lc3_t inst) {
    int dr = (inst >> 9) & 0x7;
    int sr = (inst >> 6) & 0x7;
    lc3_t offset = SignExtend<lc3_t, 6>(inst & 0x3F);
    mem[reg[sr] + offset] = reg[dr];
}

void virtual_machine_tp::VM_TRAP(lc3_t inst) {
    int trapnum = inst & 0xFF;
    if (trapnum == 0x25)
        exit(0);
    if (trapnum == 0x20) {
        reg[0] = getch();
        if (reg[0] == 13) reg[0] = 10;
    }
    else if (trapnum == 0x21) {
        std::cout << (char)(reg[0] & 255);
        std::cout.flush();
    }
    else if (trapnum == 0x22) {
        lc3_t pos = reg[0];
        while (mem[pos]) std::cout << (char)(mem[pos++] & 255), std::cout.flush();
    }
    else if (trapnum == 0x23) {
        std::cout << "\nInput a character> "; std::cout.flush();
        char ch = getch();
        if (ch == 13) ch = 10;
        std::cout << ch << std::endl;
        reg[0] = ch;
    }
    else if (trapnum == 0x24) {
        lc3_t pos = reg[0];
        while (mem[pos]) {
            std::cout << (char)(mem[pos] & 255) << (char)((mem[pos] >> 8) & 255);
            std::cout.flush();
            ++pos;
        }
    }
    else {
        std::cerr << "Bad trap!\n";
        exit(-15);
    }
}

virtual_machine_tp::virtual_machine_tp(const lc3_t address, const std::string &memfile, const std::string &regfile) {
    // Read memory
    if (memfile != ""){
        mem.ReadMemoryFromFile(memfile);
    }
    
    // Read registers
    std::ifstream input_file;
    input_file.open(regfile);
    if (input_file.is_open()) {
        int line_count = std::count(std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>(), '\n');
        input_file.close();
        input_file.open(regfile);
        if (line_count >= 8) {
            for (int index = R_R0; index <= R_R7; ++index) {
                input_file >> reg[index];
            }
        } else {
            for (int index = R_R0; index <= R_R7; ++index) {
                reg[index] = 0;
            }
        }
        input_file.close();
    } else {
        for (int index = R_R0; index <= R_R7; ++index) {
            reg[index] = 0;
        }
    }

    // Set address
    reg[R_PC] = address;
    reg[R_COND] = 0;
}

void virtual_machine_tp::SetReg(const register_tp &new_reg) {
    reg = new_reg;
}

lc3_t virtual_machine_tp::NextStep(std::vector<int> &watch, bool isstep) {
    lc3_t current_pc = reg[R_PC];
    if (current_pc < 0x3000 || current_pc >= 0xfe00) {
        std::cerr << "Access violation!\n";
        exit(-17);
    }
    reg[R_PC]++;
    lc3_t current_instruct = mem[current_pc];
    int opcode = (current_instruct >> 12) & 15;
    
    switch (opcode) {
        case O_ADD:
        if (gIsDetailedMode) {
            std::cout << "ADD" << std::endl;
        }
        VM_ADD(current_instruct);
        break;
        case O_AND:
        if (gIsDetailedMode) {
            std::cout << "AND" << std::endl;
        }
        VM_AND(current_instruct);
        break;
        case O_BR:
        if (gIsDetailedMode) {
            std::cout << "BR" << std::endl;
        }
        VM_BR(current_instruct);
        break;
        case O_JMP:
        if (gIsDetailedMode) {
            std::cout << "JMP" << std::endl;
        }
        VM_JMP(current_instruct);
        break;
        case O_JSR:
        if (gIsDetailedMode) {
            std::cout << "JSR" << std::endl;
        }
        VM_JSR(current_instruct);
        break;
        case O_LD:
        if (gIsDetailedMode) {
            std::cout << "LD" << std::endl;
        }
        VM_LD(current_instruct);
        break;
        case O_LDI:
        if (gIsDetailedMode) {
            std::cout << "LDI" << std::endl;
        }
        VM_LDI(current_instruct);
        break;
        case O_LDR:
        if (gIsDetailedMode) {
            std::cout << "LDR" << std::endl;
        }
        VM_LDR(current_instruct);
        break;
        case O_LEA:
        if (gIsDetailedMode) {
            std::cout << "LEA" << std::endl;
        }
        VM_LEA(current_instruct);
        break;
        case O_NOT:
        if (gIsDetailedMode) {
            std::cout << "NOT" << std::endl;
        }
        VM_NOT(current_instruct);
        break;
        case O_RTI:
        if (gIsDetailedMode) {
            std::cout << "RTI" << std::endl;
        }
        VM_RTI(current_instruct);
        break;
        case O_ST:
        if (gIsDetailedMode) {
            std::cout << "ST" << std::endl;
        }
        VM_ST(current_instruct);
        break;
        case O_STI:
        if (gIsDetailedMode) {
            std::cout << "STI" << std::endl;
        }
        VM_STI(current_instruct);
        break;
        case O_STR:
        if (gIsDetailedMode) {
            std::cout << "STR" << std::endl;
        }
        VM_STR(current_instruct);
        break;
        case O_TRAP:
        if (gIsDetailedMode) {
            std::cout << "TRAP" << std::endl;
        }
        if ((current_instruct & 0xFF) == 0x25) {
            reg[R_PC] = 0;
            for (int addr: watch) {
                std::cout << "MEM[x" << std::hex << addr << "] = x" << mem[addr] << " = #" << std::dec << mem[addr] << std::endl;
            }
        }
        VM_TRAP(current_instruct);
        break;
        default:
        VM_RTI(current_instruct);
        break;       
    }

    if (isstep || gIsDetailedMode) {
        for (int addr: watch) {
            std::cout << "MEM[x" << std::hex << addr << "] = x" << mem[addr] << " = #" << std::dec << (short)mem[addr] << std::endl;
        }
    }

    if (current_instruct == 0) {
        // END
        std::cerr << "Warning: Empty instruct found before HALT." << std::endl;
        return 0;
    }
    return reg[R_PC];
}

} // namespace virtual_machine_nsp