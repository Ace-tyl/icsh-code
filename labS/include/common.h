#pragma once

#include <iostream>
#include <fstream>
#include <cstdio>
#include <bitset>
#include <array>
#include <vector>
#include <cmath>
#include <climits>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include <conio.h>

// Boost library
#include <boost/program_options.hpp>

typedef unsigned short lc3_t;

// Application global variables
extern bool gIsSingleStepMode;
extern bool gIsDetailedMode;
extern std::string gInputFileName;
extern std::string gRegisterStatusFileName;
extern std::string gOutputFileName;
extern int gBeginningAddress;
