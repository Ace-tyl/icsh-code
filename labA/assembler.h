#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <limits>

const int kLC3LineLength = 16;

extern bool gIsErrorLogMode;
extern bool gIsHexMode;

static inline void _itob(int num, char s[]) {
    int n = 0;
    while (num) {
        s[n++] = (num & 1) + '0';
        num >>= 1;
    }
    s[n] = 0;
    std::reverse(s, s + n);
}

const std::vector<std::string> kLC3Pseudos({
    ".ORIG",
    ".END",
    ".STRINGZ",
    ".FILL",
    ".BLKW",
});

const std::vector<std::string> kLC3Commands({
    "ADD",    // 00: "0001" + reg(line[1]) + reg(line[2]) + op(line[3])
    "AND",    // 01: "0101" + reg(line[1]) + reg(line[2]) + op(line[3])
    "BR",     // 02: "0000000" + pcoffset(line[1],9)
    "BRN",    // 03: "0000100" + pcoffset(line[1],9)
    "BRZ",    // 04: "0000010" + pcoffset(line[1],9)
    "BRP",    // 05: "0000001" + pcoffset(line[1],9)
    "BRNZ",   // 06: "0000110" + pcoffset(line[1],9)
    "BRNP",   // 07: "0000101" + pcoffset(line[1],9)
    "BRZP",   // 08: "0000011" + pcoffset(line[1],9)
    "BRNZP",  // 09: "0000111" + pcoffset(line[1],9)
    "JMP",    // 10: "1100000" + reg(line[1]) + "000000"
    "JSR",    // 11: "01001" + pcoffset(line[1],11)
    "JSRR",   // 12: "0100000"+reg(line[1])+"000000"
    "LD",     // 13: "0010" + reg(line[1]) + pcoffset(line[2],9)
    "LDI",    // 14: "1010" + reg(line[1]) + pcoffset(line[2],9)
    "LDR",    // 15: "0110" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "LEA",    // 16: "1110" + reg(line[1]) + pcoffset(line[2],9)
    "NOT",    // 17: "1001" + reg(line[1]) + reg(line[2]) + "111111"
    "RET",    // 18: "1100000111000000"
    "RTI",    // 19: "1000000000000000"
    "ST",     // 20: "0011" + reg(line[1]) + pcoffset(line[2],9)
    "STI",    // 21: "1011" + reg(line[1]) + pcoffset(line[2],9)
    "STR",    // 22: "0111" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "TRAP"    // 23: "11110000" + h2b(line[1],8)
});

const std::vector<std::string> kLC3TrapRoutine({
    "GETC",   // x20
    "OUT",    // x21
    "PUTS",   // x22
    "IN",     // x23
    "PUTSP",  // x24
    "HALT"    // x25
});

const std::vector<std::string> kLC3TrapMachineCode({
    "1111000000100000",
    "1111000000100001",
    "1111000000100010",
    "1111000000100011",
    "1111000000100100",
    "1111000000100101"
});

enum CommandType { OPERATION, PSEUDO };

static inline void SetErrorLogMode(bool error) {
    gIsErrorLogMode = error;
}

static inline void SetHexMode(bool hex) {
    gIsHexMode = hex;
}

// A warpper class for std::unorderd_map in order to map label to its address
class LabelMapType {
private:
    std::unordered_map<std::string, unsigned> labels_;

public:
    void AddLabel(const std::string &str, unsigned address);
    unsigned GetAddress(const std::string &str) const;
};

static inline int IsLC3Pseudo(const std::string &str) {
    int index = 0;
    for (const auto &command : kLC3Pseudos) {
        if (str == command) {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int IsLC3Command(const std::string &str) {
    int index = 0;
    for (const auto &command : kLC3Commands) {
        if (str == command) {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int IsLC3TrapRoutine(const std::string &str) {
    int index = 0;
    for (const auto &trap : kLC3TrapRoutine) {
        if (str == trap) {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int CharToDec(const char &ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

static inline char DecToChar(const int &num) {
    if (num <= 9) {
        return num + '0';
    }
    return num - 10 + 'A';
}

// trim string from both left & right
static inline std::string &Trim(std::string &s) {
    int lpos = s.find_first_not_of(" ");
    int rpos = s.find_last_not_of(" ");
    if (lpos == std::string::npos) return s = "";
    return s = s.substr(lpos, rpos - lpos + 1);
}

// Format one line from asm file, do the following:
// 1. remove comments
// 2. convert the line into uppercase
// 3. replace all commas with whitespace (for splitting)
// 4. replace all "\t\n\r\f\v" with whitespace
// 5. remove the leading and trailing whitespace chars
// Note: please implement function Trim first
static std::string FormatLine(const std::string &line) {
    std::string _line = line;
    bool is_string_mode = 0;
    for (int i = 0; i < _line.size(); ++i) {
        if (!is_string_mode) {
            if (islower(_line[i])) _line[i] = toupper(_line[i]); // Convert to uppercase.
            else if (_line[i] == ',' || _line[i] == '\t' || _line[i] == '\n' || _line[i] == '\r' || _line[i] == '\f' || _line[i] == '\v') _line[i] = ' '; // Convert commas and "\t\n\r\f\v" to space.
            else if (_line[i] == '"') is_string_mode = 1;
            else if (_line[i] == ';') _line = _line.substr(0, i); // Remove comments.
        }
        else {
            if (_line[i] == '\\') ++i;
            else if (_line[i] == '"') is_string_mode = 0;
        }
    }
    return Trim(_line); // Trim the string.
}

static bool inline isValidNum(char c, int jz) {
    if (jz == 10) return isdigit(c);
    else return isdigit(c) || (c >= 'A' && c <= 'F');
}

static int inline toDigit(char c, int jz) {
    if (jz == 10) return c - '0';
    else return isdigit(c) ? c - '0' : c - 'A' + 10;
}

static int RecognizeNumberValue(const std::string &str) {
    // Convert string `str` into a number and return it
    int jz = (str[0] == 'X' ? 16 : (str[0] == '#' ? 10 : -1));
    std::string _s = str.substr(1);
    if (jz == -1) {
        _s = str;
        jz = 10;
    }
    bool neg = 0;
    if (_s[0] == '-') neg = 1, _s = _s.substr(1);
    int num = 0;
    for (char c : _s) {
        if (isValidNum(c, jz)) num = num * jz + toDigit(c, jz);
        else return 114514;
        if (num >= 65536) return 114514;
    }
    return neg ? -num : num;
}

static std::string NumberToAssemble(const int &number) {
    // Convert `number` into a 16 bit binary string
    if (number == 114514) return ""; // Invalid number
    int _num = number & 65535;
    char str[18];
    _itob(_num + 65536, str);
    return str + 1;
}

static std::string NumberToAssemble(const std::string &number) {
    // Convert `number` into a 16 bit binary string
    // You might use `RecognizeNumberValue` in this function
    return NumberToAssemble(RecognizeNumberValue(number));
}

static std::string ConvertBin2Hex(const std::string &bin) {
    // Convert the binary string `bin` into a hex string
    std::string _bin = bin, res;
    if (bin.size() & 4) _bin = std::string(-bin.size() & 4, '0') + bin;
    for (int i = 0; i < (int)bin.size(); i += 4) {
        int val = (bin[i] & 1) * 8 + (bin[i + 1] & 1) * 4 + (bin[i + 2] & 1) * 2 + (bin[i + 3] & 1);
        res += "0123456789ABCDEF"[val];
    }
    return res;
}

class assembler {
    using Commands = std::vector<std::tuple<unsigned, std::string, CommandType, unsigned>>;

private:
    LabelMapType label_map;
    Commands commands;

    static std::string TranslatePseudo(std::stringstream &command_stream, unsigned int line_id);
    std::string TranslateCommand(std::stringstream &command_stream,
                                 unsigned int current_address, unsigned int line_id);
    std::string TranslateOprand(unsigned int current_address, std::string str, int line_id,
                                int opcode_length = 3);
    std::string LineLabelSplit(const std::string &line, int current_address, int line_id);
    int firstPass(std::string &input_filename, std::string &output_filename, int start_pos);
    int secondPass(std::string &output_filename);

public:
    int assemble(std::string &input_filename, std::string &output_filename, int start_pos = 0);
};
