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
	if ((returnValue = ReplaceFunctionAddressSet(returnValue, functionName)) == NULL) {
		std::cout << "\n[!] Error on the replacing function set routine!\n" << std::endl;
		return NULL;
	}
	if ((returnValue = ReplaceNumberOfFunctionsSet(returnValue, functionName.size())) == NULL) {
		std::cout << "\n[!] Error on the replacing number of functions!\n" << std::endl;
		return NULL;
	}

	if ((returnValue = ReplaceNumberOfNamesSet(returnValue, functionName.size())) == NULL) {
		std::cout << "\n[!] Error on the replacing number of names!\n" << std::endl;
		return NULL;
	}

	return returnValue;
}

int main(int argc, char* argv[]) {
	OPTIONS configurationOptions;
	int templateSize = 0;
	char templatePath[MAX_PATH] = { 0 };
	memset(&configurationOptions, 0x00, sizeof(OPTIONS));
	PrintBanner();
	if (!ParseArgs(argc, argv, configurationOptions)) {
		std::cout << "\n[!] Error on parsing arguments. You may have forgotten mandatory options. Use -h for help." << std::endl;
		return -1;
	}
	std::cout << "\n[+] Arguments are successfully parsed!" << std::endl;
	std::vector<PCHAR> functionNames = ParseInputFile(configurationOptions.inputPath);
	if (functionNames.empty()) {
		std::cout << "[!] Error on the function name list!" << std::endl;
		return -1;
	}
	std::cout << "[+] Function name list is successfully read!" << std::endl;
	sprintf(templatePath, "%s\\..\\template\\template.cpp", SOLUTIONDIR);
	PCHAR templateBytes = (PCHAR) ReadBinary(templatePath, templateSize);
	if (templateBytes == NULL || templateSize == 0) {
		std::cout << "[!] Error on DLL template!" << std::endl;
		return -1;
	}
	std::cout << "[+] Template DLL file is read!" << std::endl;

	templateBytes = ReplacePlaceholders(templateBytes, templateSize, functionNames,configurationOptions.dllName, configurationOptions.numberOfOtherFunctions);
	if (templateBytes == NULL || templateSize == 0) {
		std::cout << "[!] Error on replacing placeholders!" << std::endl;
		return -1;
	}
	std::cout << "[+] Placeholders in the template are successfully replaced!" << std::endl;

	if (!WriteBinary(configurationOptions.outputPath, (PBYTE) templateBytes, strlen(templateBytes))) {
		std::cout << "[!] Error on writing output! (Path: " << configurationOptions.outputPath << ")" << std::endl;
		return -1;
	}

	std::cout << "[+] Template DLL file is ready as " << configurationOptions.outputPath << "!" << std::endl;
	std::cout << "[+] Don't forget to put function definitions to the template!" << std::endl;

	return 0;
}