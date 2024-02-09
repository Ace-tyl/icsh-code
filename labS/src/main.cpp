#include "simulator.h"
#include "common.h"

using namespace virtual_machine_nsp;
namespace po = boost::program_options;

bool gIsSingleStepMode = false;
bool gIsDetailedMode = false;
std::string gInputFileName = "input.txt";
std::string gRegisterStatusFileName = "register.txt";
std::string gOutputFileName = "";
int gBeginningAddress = 0x3000;
std::vector<int> watchAddrs;

bool inline isValidNum(char c, int jz) {
    if (jz == 10) return isdigit(c);
    else return isdigit(c) || (c >= 'A' && c <= 'F');
}

int inline toDigit(char c, int jz) {
    if (jz == 10) return c - '0';
    else return isdigit(c) ? c - '0' : c - 'A' + 10;
}

int RecognizeNumberValue(std::string str) {
    for (auto &it : str) if (islower(it)) it = toupper(it);
    // Convert string `str` into a number and return it
    int jz = ((str[0] == 'X' || str[0] == 'x') ? 16 : (str[0] == '#' ? 10 : -1));
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

int main(int argc, char **argv) {
    po::options_description desc{"\e[1mLC3 SIMULATOR\e[0m\n\n\e[1mOptions\e[0m"};
    desc.add_options()                                                                             //
        ("help,h", "Help screen")                                                                  //
        ("file,f", po::value<std::vector<std::string>>(), "Input file (default input.bin)")             //
        ("register,r", po::value<std::string>()->default_value("register.txt"), "Register Status") //
        ("single,s", "Single Step Mode")                                                           //
        ("begin,b", po::value<std::vector<std::string>>(), "Begin address (0x3000)")
        ("output,o", po::value<std::string>()->default_value(""), "Output file")
        ("watch,w", po::value<std::vector<std::string>>(), "Watch memory addresses")
        ("detail,d", "Detailed Mode");

    po::variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    std::vector<std::string> infiles;
    if (vm.count("file")) {
        infiles = vm["file"].as<std::vector<std::string>>();
        if (!infiles.size()) infiles.push_back("input.bin");
    }
    if (vm.count("register")) {
        gRegisterStatusFileName = vm["register"].as<std::string>();
    }
    if (vm.count("single")) {
        gIsSingleStepMode = true;
    }
    std::vector<int> begaddrs;
    if (vm.count("begin")) {
        std::vector<std::string> tmps = vm["begin"].as<std::vector<std::string>>();
        for (auto &it : tmps) {
            int addr = RecognizeNumberValue(it);
            if (addr < 0 || addr > 65535) {
                std::cerr << "Invalid beginning address!\n";
                return -67;
            }
            begaddrs.push_back(addr);
        }
        if (begaddrs.size() == infiles.size() - 1) begaddrs.push_back(0x3000);
        else if (begaddrs.size() < infiles.size() - 1 || begaddrs.size() > infiles.size()) {
            std::cerr << "Number of files can't match number of addresses.\n";
            return -66;
        }
    }
    else {
        begaddrs.push_back(0x3000);
    }
    if (vm.count("output")) {
        gOutputFileName = vm["output"].as<std::string>();
    }
    if (vm.count("detail")) {
        gIsDetailedMode = true;
    }
    if (vm.count("watch")) {
        std::vector<std::string> watches = vm["watch"].as<std::vector<std::string>>();
        for (auto &it : watches) {
            if (it.find("-") != std::string::npos) {
                int p = it.find("-");
                std::string bes = it.substr(0, p), ens = it.substr(p + 1);
                int be = RecognizeNumberValue(bes), en = RecognizeNumberValue(ens);
                if (be >= 0 && be < 65536 && en >= be && en < 65536) {
                    for (int i = be; i <= en; ++i) watchAddrs.push_back(i);
                }
                continue;
            }
            int val = RecognizeNumberValue(it);
            if (val < 0 || val >= 65536) continue;
            watchAddrs.push_back(val);
        }
    }

    virtual_machine_tp virtual_machine(gBeginningAddress, "", gRegisterStatusFileName);
    for (int i = 0; i < begaddrs.size(); ++i) {
        virtual_machine.mem.ReadMemoryFromFile(infiles[i], begaddrs[i]);
    }
    int halt_flag = true;
    int time_flag = 0;
    while(halt_flag) {
        // Single step
        virtual_machine.NextStep(watchAddrs, gIsSingleStepMode);
        if (gIsDetailedMode)
            std::cout << virtual_machine.reg << std::endl;
        if (gIsSingleStepMode) {
            puts("Press ESC to exit or any key to continue ...");
            int ch = getch();
            if (ch == 27) break;
        }
        ++time_flag;
    }

    std::cout << virtual_machine.reg << std::endl;
    std::cout << "cycle = " << time_flag << std::endl;
    return 0;
}