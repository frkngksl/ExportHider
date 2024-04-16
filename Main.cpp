#include <Windows.h>
#include "Structs.h"
#include "OptionsHelper.h"
#include "AuxFunctions.h"

int main(int argc, char* argv[]) {
	OPTIONS configurationOptions;
	memset(&configurationOptions, 0x00, sizeof(OPTIONS));
	PrintBanner();
	if (!ParseArgs(argc, argv, configurationOptions)) {
		std::cout << "\n[!] Error on parsing arguments. You may have forgotten mandatory options. Use -h for help.\n" << std::endl;
		return -1;
	}
	std::vector<PCHAR> functionNames = ParseInputFile(configurationOptions.inputPath);

	return 0;
}