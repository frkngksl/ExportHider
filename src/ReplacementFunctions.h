#pragma once
#include<Windows.h>
#include"AuxFunctions.h"

PCHAR ReplaceArraySize(PCHAR templateBuffer, std::vector<PCHAR>& functionName, int numberOfOtherFunctions);
PCHAR ReplaceNameSize(PCHAR templateBuffer, std::vector<PCHAR>& functionName);
PCHAR ReplaceExportTableSize(PCHAR templateBuffer);
PCHAR ReplaceDllName(PCHAR templateBuffer, LPSTR dllName);
PCHAR ReplaceFunctionNameArray(PCHAR templateBuffer, std::vector<PCHAR>& functionName);
PCHAR ReplaceFunctionAddressSet(PCHAR templateBuffer, std::vector<PCHAR>& functionName);
PCHAR ReplaceNumberOfFunctionsSet(PCHAR templateBuffer, int numberOfFunctions);
PCHAR ReplaceNumberOfNamesSet(PCHAR templateBuffer, int numberOfNamedFunctions);