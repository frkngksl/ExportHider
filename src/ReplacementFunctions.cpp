#include "ReplacementFunctions.h"

PCHAR ReplaceArraySize(PCHAR templateBuffer, std::vector<PCHAR>& functionName, int numberOfOtherFunctions) {
	char tempBuffer[1024] = { 0 };
	int totalFunctionSize = numberOfOtherFunctions + functionName.size();
	int totalArraySize = totalFunctionSize * sizeof(DWORD) * 2 + totalFunctionSize * sizeof(WORD);
	sprintf(tempBuffer, "%d", totalArraySize);
	return ReplacePlaceholder(templateBuffer, tempBuffer, "/*[PLACEHOLDERARRAYSIZE]*/");
}

PCHAR ReplaceNameSize(PCHAR templateBuffer, std::vector<PCHAR>& functionName) {
	char tempBuffer[1024] = { 0 };
	int totalSize = 0;
	for (int i = 0; i < functionName.size(); i++) {
		totalSize += (strlen(functionName[i]) + 1);
	}
	sprintf(tempBuffer, "%d", totalSize);
	return ReplacePlaceholder(templateBuffer, tempBuffer, "/*[PLACEHOLDERNAMESSIZE]*/");
}

PCHAR ReplaceExportTableSize(PCHAR templateBuffer) {
	char tempBuffer[1024] = { 0 };
	sprintf(tempBuffer, "%d", (int) sizeof(IMAGE_EXPORT_DIRECTORY));
	return ReplacePlaceholder(templateBuffer, tempBuffer, "/*[PLACEHOLDEREXPORTTABLESIZE]*/");
}

PCHAR ReplaceDllName(PCHAR templateBuffer, LPSTR dllName) {
	char tempBuffer[1024] = { 0 };
	sprintf(tempBuffer, "= \"%s\"", dllName);
	return ReplacePlaceholder(templateBuffer, tempBuffer, "/*[PLACEHOLDERFORDLLNAME]*/");
}

PCHAR ReplaceFunctionNameArray(PCHAR templateBuffer, std::vector<PCHAR>& functionName) {
	// {"test1","test2","test3"}
	int totalSize = 2;
	for (int i = 0; i < functionName.size(); i++) {
		totalSize += (strlen(functionName[i]) + 3);
	}
	PCHAR dynamicBuffer = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalSize);
	dynamicBuffer[0] = '{';
	int cursor = 1;
	for (int i = 0; i < functionName.size(); i++) {
		dynamicBuffer[cursor++] = '"';
		memcpy(dynamicBuffer + cursor, functionName[i],strlen(functionName[i]));
		cursor += strlen(functionName[i]);
		dynamicBuffer[cursor++] = '"';
		if (i != functionName.size() - 1) {
			dynamicBuffer[cursor++] = ',';
		}
	}
	dynamicBuffer[totalSize - 2] = '}';
	dynamicBuffer[totalSize - 1] = '\0';
	return ReplacePlaceholder(templateBuffer, dynamicBuffer, "/*[PLACEHOLDERNAMEARRAY]*/");
}

PCHAR ReplaceFunctionAddressSet(PCHAR templateBuffer, std::vector<PCHAR>& functionName) {
	// 0D 0A 09 seperator
	char tempBuffer[1024] = { 0 };
	PCHAR dynamicBuffer = NULL;
	int index = 0;
	const char *templateString = "addressArray[oldFunctionSize + %d] = (DWORD)(((PBYTE) %s) - moduleAddress);\r\n\t";
	for (int i = 0; i < functionName.size(); i++) {
		sprintf(tempBuffer, templateString, index++, functionName[i]);
		dynamicBuffer = AppendToOutput(dynamicBuffer, tempBuffer);
		if (dynamicBuffer == NULL) {
			std::cout << "Output Appending Error: " << GetLastError() << std::endl;
			return NULL;
		}
		memset(tempBuffer, 0x00, 1024);
	}
	return ReplacePlaceholder(templateBuffer, dynamicBuffer, "/*[PLACEHOLDERFORNUMBEROFFUNCTIONADDRESS]*/");
}

PCHAR ReplaceNumberOfFunctionsSet(PCHAR templateBuffer, int numberOfFunctions) {
	char tempBuffer[1024] = { 0 };
	sprintf(tempBuffer, "newExportDirectory->NumberOfFunctions = oldFunctionSize + %d;", numberOfFunctions);
	return ReplacePlaceholder(templateBuffer, tempBuffer, "/*[PLACEHOLDERFORNUMBEROFFUNCTIONSSET]*/");
}

PCHAR ReplaceNumberOfNamesSet(PCHAR templateBuffer, int numberOfNamedFunctions) {
	char tempBuffer[1024] = { 0 };
	sprintf(tempBuffer, "newExportDirectory->NumberOfNames = oldNameArraySize + %d;", numberOfNamedFunctions);
	return ReplacePlaceholder(templateBuffer, tempBuffer, "/*[PLACEHOLDERFORNUMBEROFNAMESSET]*/");
}