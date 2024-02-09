#include "common.h"

namespace virtual_machine_nsp {
const int kInstructionLength = 16;

inline lc3_t TranslateInstruction(std::string &line) {
    lc3_t result = 0;
    if (line.size() == kInstructionLength) {
        for (int index = 0; index < kInstructionLength; ++index) {
            result = (result << 1) | (line[index] & 1);
        }
    }
    else {
        for (int index = 0; index < kInstructionLength / 4; ++index) {
            result = (result << 4) | (line[index] < 'A' ? line[index] - '0' : line[index] - 'A' + 10);
        }
    }
    return result;
}

const int kVirtualMachineMemorySize = 0x10000;

class memory_tp {
    private:
    lc3_t memory[kVirtualMachineMemorySize];

    public:
    memory_tp() {
        memset(memory, 0, sizeof(lc3_t) * kVirtualMachineMemorySize);
    }
    // Managements
    void ReadMemoryFromFile(std::string filename, int beginning_address=0x3000);
    lc3_t GetContent(lc3_t address) const;
    lc3_t& operator[](lc3_t address);
};

}; // virtual machine nsp