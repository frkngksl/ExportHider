#include "OptionsHelper.h"
#include "Structs.h"

void PrintBanner() {
    const char* banner = R"(
   __                       _          _     _
  /__\_  ___ __   ___  _ __| |_  /\  /(_) __| | ___ _ __
 /_\ \ \/ / '_ \ / _ \| '__| __|/ /_/ / |/ _` |/ _ \ '__|
//__  >  <| |_) | (_) | |  | |_/ __  /| | (_| |  __/ |
\__/ /_/\_\ .__/ \___/|_|   \__\/ /_/ |_|\__,_|\___|_|
          |_|
                     by @R0h1rr1m
)";
	std::cout << banner << std::endl;
}

void PrintHelp(char* binaryName) {
    const char* optionsString = R"(
    -h | --help                                 Show the help message.
    -i | --input <Input Path>                   Input path for the list of function names to be hidden. (Mandatory) 
    -o | --output <Output Path>                 Output path for the DLL template. (Mandatory) 
    -n | --name <DLL Name>                      Name of the DLL for the Export Directory. (Mandatory)
    -c | --count <Number of Other Functions>    Number of other exported functions that won't be hidden.
)";
    std::cout << "Usage of " << binaryName << ":" << std::endl;
    std::cout << optionsString << std::endl;
}

bool ParseArgs(int argc, char* argv[], OPTIONS& configurationOptions) {
    for (int i = 1; i < argc; i++) {
        if (_strcmpi(argv[i], "-h") == 0 || _strcmpi(argv[i], "--help") == 0) {
            PrintHelp(argv[0]);
            exit(0);
        }
        else if (_strcmpi(argv[i], "-i") == 0 || _strcmpi(argv[i], "--input") == 0) {
            i++;
            if (i > argc) {
                return false;
            }
            configurationOptions.inputPath = argv[i];
        }
        else if (_strcmpi(argv[i], "-o") == 0 || _strcmpi(argv[i], "--output") == 0) {
            i++;
            if (i > argc) {
                return false;
            }
            configurationOptions.outputPath = argv[i];
        }
        else if (_strcmpi(argv[i], "-n") == 0 || _strcmpi(argv[i], "--name") == 0) {
            i++;
            if (i > argc) {
                return false;
            }
            configurationOptions.dllName = argv[i];
        }
        else if (_strcmpi(argv[i], "-c") == 0 || _strcmpi(argv[i], "--count") == 0) {
            i++;
            if (i > argc) {
                return false;
            }
            configurationOptions.numberOfOtherFunctions = atoi(argv[i]);
        }
    }
    if (configurationOptions.inputPath == NULL || configurationOptions.outputPath == NULL || configurationOptions.dllName == NULL) {
        return false;
    }
    return true;
}