#include "common.h"
#include "memory.h"

namespace virtual_machine_nsp {
    void memory_tp::ReadMemoryFromFile(std::string filename, int beginning_address) {
        // Read from the file
        std::ifstream fin(filename);
        std::string str;
        while (fin >> str) {
            memory[beginning_address++] = TranslateInstruction(str);
        }
    }

    lc3_t memory_tp::GetContent(lc3_t address) const {
        // get the content
        return memory[address];
    }

    lc3_t& memory_tp::operator[](lc3_t address) {
        // get the content
        return memory[address];
    }    
}; // virtual machine namespace
