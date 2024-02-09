#include "assembler.h"
#include <string>

// add label and its address to symbol table
void LabelMapType::AddLabel(const std::string &str, const unsigned address) {
    labels_.insert({str, address});
}

unsigned LabelMapType::GetAddress(const std::string &str) const {
    if (labels_.find(str) == labels_.end()) {
        // not found
        return -1;
    }
    return labels_.at(str);
}

std::string assembler::TranslateOprand(unsigned int current_address, std::string str, int line_id,
                                       int opcode_length) {
    // Translate the oprand
    str = Trim(str);
    auto item = label_map.GetAddress(str);
    if (item != -1) {
        // str is a label
        int v = item - current_address - 1;
        if (v < -(1 << (opcode_length - 1)) || v > (1 << (opcode_length - 1)) - 1) {
            std::cerr << "At line " << line_id << ": ERROR: Invalid value in oprand (too large or too small)\n";
            exit(-66);
        }
        if (v < 0) v += (1 << opcode_length);
        char str[18];
        _itob(v + (1 << opcode_length), str);
        return str + 1;
    }
    if (str[0] == 'R') {
        // str is a register
        if (str[1] >= '0' && str[1] <= '7') {
            return std::string(1, char('0' + bool(str[1] & 4))) + std::string(1, char('0' + bool(str[1] & 2))) + std::string(1, char('0' + bool(str[1] & 1)));
        }
        else {
            std::cerr << "At line " << line_id << ": ERROR: Invalid value in oprand (too large or too small)\n";
            exit(-66);
        }
    } else {
        // str is an immediate number
        int v = RecognizeNumberValue(str);
        if (v < -(1 << (opcode_length - 1)) || v > (1 << opcode_length) - 1) {
            std::cerr << "At line " << line_id << ": ERROR: Invalid value in oprand (too large or too small)\n";
            exit(-66);
        }
        if (v < 0) v += (1 << opcode_length);
        char str[18];
        _itob(v + (1 << opcode_length), str);
        return str + 1;
    }
}

std::string assembler::LineLabelSplit(const std::string &line,
                                      int current_address, int line_id) {
    // label?
    auto first_whitespace_position = line.find(' ');
    auto first_token = line.substr(0, first_whitespace_position);

    if (IsLC3Pseudo(first_token) == -1 && IsLC3Command(first_token) == -1 &&
        IsLC3TrapRoutine(first_token) == -1) {
        // * This is an label
        // save it in label_map
        if (label_map.GetAddress(first_token) != -1) {
            if (label_map.GetAddress(first_token) != current_address) {
                std::cerr << "At line " << line_id << ": ERROR: Duplicate label\n";
                exit(-64);
            }
        }
        else label_map.AddLabel(first_token, current_address);

        // remove label from the line
        if (first_whitespace_position == std::string::npos) {
            // nothing else in the line
            return "";
        }
        auto command = line.substr(first_whitespace_position + 1);
        return Trim(command);
    }
    return line;
}

// Scan #1: save commands and labels with their addresses
int assembler::firstPass(std::string &input_filename, std::string &output_filename, int start_pos) {
    std::string line;
    std::ifstream input_file(input_filename);
    if (!input_file.is_open()) {
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }
    input_file.seekg(start_pos, std::ios::beg);

    int orig_address = -1;
    int current_address = -1;
    int line_id = 0;

    while (std::getline(input_file, line)) {

        ++line_id;
        line = FormatLine(line);
        if (line.empty()) {
            continue;
        }

        auto command = LineLabelSplit(line, current_address, line_id);
        if (command.empty()) {
            continue;
        }

        // OPERATION or PSEUDO?
        auto first_whitespace_position = command.find(' ');
        auto first_token = command.substr(0, first_whitespace_position);

        // Special judge .ORIG and .END
        if (first_token == ".ORIG") {
            if (orig_address != -1) {
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: .ORIG without .END\n";
                return -9;
            }
            std::string orig_value =
                command.substr(first_whitespace_position + 1);
            orig_address = RecognizeNumberValue(orig_value);
            if (orig_address == std::numeric_limits<int>::max()) {
                // @ Error address
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error address\n";
                return -2;
            }
            commands.push_back({orig_address, "this_is_orig", CommandType::PSEUDO, 0});
            current_address = orig_address;
            continue;
        }

        if (orig_address == -1) {
            // @ Error Program begins before .ORIG
            if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: program begins before .ORIG\n";
            return -3;
        }

        if (current_address >= 65536) {
            // @ Error Invalid address
            if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid address\n";
            return -10;
        }

        if (first_token == ".END") {
            orig_address = -1;
            continue;
        }

        // For LC3 Operation
        if (IsLC3Command(first_token) != -1 ||
            IsLC3TrapRoutine(first_token) != -1) {
            commands.push_back(
                {current_address, command, CommandType::OPERATION, line_id});
            current_address += 1;
            continue;
        }

        // For Pseudo code
        commands.push_back({current_address, command, CommandType::PSEUDO, line_id});
        auto operand = command.substr(first_whitespace_position + 1);
        if (first_token == ".FILL") {
            auto num_temp = RecognizeNumberValue(operand);
            if (num_temp == std::numeric_limits<int>::max()) {
                // @ Error Invalid Number input @ FILL
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid Number input after .FILL\n";
                return -4;
            }
            if (num_temp > 65535 || num_temp < -65536) {
                // @ Error Too large or too small value  @ FILL
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: too large or too small value after .FILL\n";
                return -5;
            }
            current_address += 1;
        }
        if (first_token == ".BLKW") {
            // modify current_address
            Trim(operand);
            int value = RecognizeNumberValue(operand);
            if (value < 0 || value >= 65536 - current_address) {
                // @ Error Invalid or too large value  @ BLKW
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid or too large value after .BLKW\n";
                return -6;
            }
            current_address += value;
        }
        if (first_token == ".STRINGZ") {
            // modify current_address
            Trim(operand);
            if (operand[0] != '"') {
                // @ Error Invalid string  @ STRINGZ
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid string after .STRINGZ\n";
                return -7;
            }
            int now = 1, len = 0, valid = 1;
            while (now < (int)operand.size() && operand[now] != '"') {
                ++len;
                if (operand[now] == '\\') {
                    ++now;
                    if (now == (int)operand.size()) {
                        valid = 0;
                        break;
                    }
                }
                ++now;
            }
            if (!valid) {
                // @ Error Invalid string  @ STRINGZ
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid string after .STRINGZ\n";
                return -7;
            }
            current_address += len + 1;
        }
    }
    // OK flag
    return 0;
}

std::string assembler::TranslatePseudo(std::stringstream &command_stream, unsigned int line_id) {
    std::string pseudo_opcode;
    std::string output_line;
    command_stream >> pseudo_opcode;
    if (pseudo_opcode == ".FILL") {
        std::string number_str;
        command_stream >> number_str;
        output_line = NumberToAssemble(number_str);
        if (output_line == "") {
            if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid Number input after .FILL\n";
            exit(-4);
        }
        if (gIsHexMode)
            output_line = ConvertBin2Hex(output_line);
    } else if (pseudo_opcode == ".BLKW") {
        // Fill 0 here
        std::string number_str;
        command_stream >> number_str;
        int val = RecognizeNumberValue(number_str);
        if (val == 114514) {
            if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: invalid Number input after .BLKW\n";
            exit(-6);
        }
        while (val--) {
            output_line += gIsHexMode ? std::string("0000") : std::string(16, '0');
            if (val) output_line += "\n";
        }
    } else if (pseudo_opcode == ".STRINGZ") {
        // Fill string here
        std::string operand;
        std::getline(command_stream, operand);
        Trim(operand);
        int now = 1, len = 0, valid = 1;
        while (now < (int)operand.size() && operand[now] != '"') {
            std::string output;
            if (operand[now] == '\\') {
                ++now;
                if (operand[now] == 'a') output = NumberToAssemble(7);
                else if (operand[now] == 'b') output = NumberToAssemble(8);
                else if (operand[now] == 't') output = NumberToAssemble(9);
                else if (operand[now] == 'n') output = NumberToAssemble(10);
                else if (operand[now] == 'v') output = NumberToAssemble(11);
                else if (operand[now] == 'f') output = NumberToAssemble(12);
                else if (operand[now] == 'r') output = NumberToAssemble(13);
                else output = NumberToAssemble(operand[now]);
            }
            else output = NumberToAssemble(operand[now]);
            output_line += gIsHexMode ? ConvertBin2Hex(output) : output;
            ++now;
            output_line += '\n';
        }
        output_line += gIsHexMode ? std::string("0000") : std::string(16, '0');
    }
    return output_line;
}

std::string assembler::TranslateCommand(std::stringstream &command_stream,
                                        unsigned int current_address, unsigned int line_id) {
    std::string opcode;
    command_stream >> opcode;
    auto command_tag = IsLC3Command(opcode);

    std::vector<std::string> operand_list;
    std::string operand;
    while (command_stream >> operand) {
        operand_list.push_back(operand);
    }
    auto operand_list_size = operand_list.size();

    std::string output_line;

    if (command_tag == -1) {
        // This is a trap routine
        command_tag = IsLC3TrapRoutine(opcode);
        output_line = kLC3TrapMachineCode[command_tag];
    } else {
        // This is a LC3 command
        switch (command_tag) {
        case 0:
            // "ADD"
            output_line += "0001";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after ADD\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id);
            if (operand_list[2][0] == 'R') {
                // The third operand is a register
                output_line += "000";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], line_id);
            } else {
                // The third operand is an immediate number
                output_line += "1";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], line_id, 5);
            }
            break;
        case 1:
            // "AND"
            output_line += "0101";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after AND\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id);
            if (operand_list[2][0] == 'R') {
                // The third operand is a register
                output_line += "000";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], line_id);
            } else {
                // The third operand is an immediate number
                output_line += "1";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], line_id, 5);
            }
            break;
        case 2:
            // "BR"
            output_line += "0000111";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BR\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 3:
            // "BRN"
            output_line += "0000100";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRn\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 4:
            // "BRZ"
            output_line += "0000010";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRz\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 5:
            // "BRP"
            output_line += "0000001";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRp\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 6:
            // "BRNZ"
            output_line += "0000110";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRnz\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 7:
            // "BRNP"
            output_line += "0000101";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRnp\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 8:
            // "BRZP"
            output_line += "0000011";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRzp\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 9:
            // "BRNZP"
            output_line += "0000111";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after BRnzp\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 9);
            break;
        case 10:
            // "JMP"
            output_line += "1100000";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after JMP\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += "000000";
            break;
        case 11:
            // "JSR"
            output_line += "01001";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after JSR\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 11);
            break;
        case 12:
            // "JSRR"
            output_line += "0100000";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after JSRR\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += "000000";
            break;
        case 13:
            // "LD"
            output_line += "0010";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after LD\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id, 9);
            break;
        case 14:
            // "LDI"
            output_line += "1010";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after LDI\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id, 9);
            break;
        case 15:
            // "LDR"
            output_line += "0110";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after LDR\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id);
            output_line += TranslateOprand(current_address, operand_list[2], line_id, 6);
            break;
        case 16:
            // "LEA"
            output_line += "1110";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after LEA\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id, 9);
            break;
        case 17:
            // "NOT"
            output_line += "1001";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after NOT\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id);
            output_line += "111111";
            break;
        case 18:
            // RET
            output_line += "1100000111000000";
            if (operand_list_size != 0) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after RET\n";
                exit(-30);
            }
            break;
        case 19:
            // RTI
            output_line += "1000000000000000";
            if (operand_list_size != 0) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after RTI\n";
                exit(-30);
            }
            break;
        case 20:
            // ST
            output_line += "0011";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after ST\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id, 9);
            break;
        case 21:
            // STI
            output_line += "1011";
            if (operand_list_size != 2) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after STI\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id, 9);
            break;
        case 22:
            // STR
            output_line += "0111";
            if (operand_list_size != 3) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after STR\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id);
            output_line += TranslateOprand(current_address, operand_list[1], line_id);
            output_line += TranslateOprand(current_address, operand_list[2], line_id, 6);
            break;
        case 23:
            // TRAP
            output_line += "11110000";
            if (operand_list_size != 1) {
                // @ Error operand numbers
                if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: error operand numbers after TRAP\n";
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], line_id, 8);
            break;
        default:
            // Unknown opcode
            // @ Error
            if (gIsErrorLogMode) std::cerr << "At line " << line_id << ": ERROR: Unknown opcode\n";
            exit(-40);
        }
    }

    if (gIsHexMode)
        output_line = ConvertBin2Hex(output_line);

    return output_line;
}

int assembler::secondPass(std::string &output_filename) {
    // Scan #2:
    // Translate
    std::ofstream output_file;

    for (const auto &command : commands) {
        const unsigned address = std::get<0>(command);
        const std::string command_content = std::get<1>(command);
        const CommandType command_type = std::get<2>(command);
        const unsigned line_id = std::get<3>(command);
        auto command_stream = std::stringstream(command_content);

        if (command_type == CommandType::PSEUDO) {
            // Pseudo
            if (command_content == "this_is_orig") {
                if (output_file) output_file.close();
                char cc[5]; sprintf(cc, "%x", address);
                output_file.open((!strcmp(cc, "3000") ? output_filename : output_filename + "_x" + cc) + ".bin");
                if (!output_file) {
                    // @ Error at output file
                    if (gIsErrorLogMode) std::cerr << "ERROR: Failed to open output file\n";
                    return -20;
                }
            }
            else output_file << TranslatePseudo(command_stream, line_id) << std::endl;
        } else {
            // LC3 command
            output_file << TranslateCommand(command_stream, address, line_id) << std::endl;
        }
    }

    // Close the output file
    output_file.close();
    // OK flag
    return 0;
}

// assemble main function
int assembler::assemble(std::string &input_filename, std::string &output_filename, int start_pos) {
    auto first_scan_status = firstPass(input_filename, output_filename, start_pos);
    if (first_scan_status == -114514) return 0;
    if (first_scan_status != 0) {
        return first_scan_status;
    }
    auto second_scan_status = secondPass(output_filename);
    if (second_scan_status != 0) {
        return second_scan_status;
    }
    // OK flag
    return 0;
}
