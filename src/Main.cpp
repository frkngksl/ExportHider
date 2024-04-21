#include <Windows.h>
#include "Structs.h"
#include "OptionsHelper.h"
#include "AuxFunctions.h"
#include "ReplacementFunctions.h"

PCHAR ReplacePlaceholders(PCHAR templateBytes, int& templateSize, std::vector<PCHAR> &functionName, LPSTR dllName, int numberOfOtherFunctions) {
	PCHAR returnValue = NULL;
	if ((returnValue = ReplaceArraySize(templateBytes,functionName,numberOfOtherFunctions)) == NULL) {
		std::cout << "\n[!] Error on the replacing array buffer size!\n" << std::endl;
		return NULL;
	}
	if ((returnValue = ReplaceNameSize(returnValue,functionName)) == NULL) {
		std::cout << "\n[!] Error on the replacing names buffer size!\n" << std::endl;
		return NULL;
	}
	if ((returnValue = ReplaceExportTableSize(returnValue)) == NULL) {
		std::cout << "\n[!] Error on the replacing export table buffer size!\n" << std::endl;
		return NULL;
	}
	if ((returnValue = ReplaceDllName(returnValue,dllName)) == NULL) {
		std::cout << "\n[!] Error on the replacing dll name buffer!\n" << std::endl;
		return NULL;
	}
	if ((returnValue = ReplaceFunctionNameArray(returnValue, functionName)) == NULL) {
		std::cout << "\n[!] Error on the replacing function name array!\n" << std::endl;
		return NULL;
	}

	std::cout << returnValue << std::endl;

}

int main(int argc, char* argv[]) {
	OPTIONS configurationOptions;
	int templateSize = 0;
	char templatePath[MAX_PATH] = { 0 };
	memset(&configurationOptions, 0x00, sizeof(OPTIONS));
	PrintBanner();
	if (!ParseArgs(argc, argv, configurationOptions)) {
		std::cout << "\n[!] Error on parsing arguments. You may have forgotten mandatory options. Use -h for help.\n" << std::endl;
		return -1;
	}
	std::vector<PCHAR> functionNames = ParseInputFile(configurationOptions.inputPath);
	if (functionNames.empty()) {
		std::cout << "\n[!] Error on the function name list!\n" << std::endl;
		return -1;
	}
	sprintf(templatePath, "%s\\..\\template\\template.cpp", SOLUTIONDIR);
	PCHAR templateBytes = (PCHAR) ReadBinary(templatePath, templateSize);
	if (templateBytes == NULL || templateSize == 0) {
		std::cout << "\n[!] Error on DLL template!\n" << std::endl;
		return -1;
	}
	templateBytes = ReplacePlaceholders(templateBytes, templateSize, functionNames,configurationOptions.dllName, configurationOptions.numberOfOtherFunctions);
	if (templateBytes == NULL || templateSize == 0) {
		std::cout << "\n[!] Error on replacing placeholders!\n" << std::endl;
		return -1;
	}
	return 0;
}