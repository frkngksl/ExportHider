#include <windows.h>

char arrayBuffer[/*[PLACEHOLDERARRAYSIZE]*/] = { 0x0 };
char namesBuffer[/*[PLACEHOLDERNAMESSIZE]*/] = { 0x0 };
char exportTableBuffer[/*[PLACEHOLDEREXPORTTABLESIZE]*/] = { 0x0 };
char nameOfTheDll[] /*[PLACEHOLDERFORDLLNAME]*/;

void SetNameArray(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory, PDWORD oldArray, int oldArraySize) {
	const char *functionNames[] = /*[PLACEHOLDERNAMEARRAY]*/;
	PDWORD nameArray = (PDWORD)(moduleAddress + newExportDirectory->AddressOfNames);
	int numberOfFunctions = sizeof(functionNames) / sizeof(PCHAR) + oldArraySize;
	int cursorForNameArea = 0;
	for (int i = 0, j=0; i < numberOfFunctions; i++) {
		if (i < oldArraySize) {
			nameArray[i] = oldArray[i];
		}
		else {
			memcpy(namesBuffer + cursorForNameArea, (LPVOID)functionNames[j], strlen((const char*)functionNames[j]) + 1);
			nameArray[i] = (DWORD)((PBYTE)(namesBuffer + cursorForNameArea) - moduleAddress);
			cursorForNameArea += (strlen((const char*)functionNames[j++]) + 1);
		}
	}
}

void SetFunctionAddresses(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory, PDWORD oldFunctionArray, int oldFunctionSize) {
	PDWORD addressArray = (PDWORD)(moduleAddress + newExportDirectory->AddressOfFunctions);
	for (int i = 0; i < oldFunctionSize; i++) {
		addressArray[i] = oldFunctionArray[i];
	}
	/*[PLACEHOLDERFORNUMBEROFFUNCTIONADDRESS]*/

}

void SetOrdinalArray(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory, PWORD oldOrdinalArray, int oldOrdinalArraySize, int oldFunctionArraySize) {
	PWORD ordinalArray = (PWORD)(moduleAddress + newExportDirectory->AddressOfNameOrdinals);

	for (int i = 0; i < newExportDirectory->NumberOfNames; i++) {
		if (i<oldOrdinalArraySize) {
			ordinalArray[i] = oldOrdinalArray[i];
		}
		else {
			ordinalArray[i] = oldFunctionArraySize++;
		}
	}
}

// Need to sort export table
void SortExportedNameArray(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory) {
	PDWORD nameArray = (PDWORD)(moduleAddress + newExportDirectory->AddressOfNames);
	PWORD ordinalArray = (PWORD)(moduleAddress + newExportDirectory->AddressOfNameOrdinals);
	PCHAR functionName1;
	PCHAR functionName2;
	DWORD tempHolderForName;
	WORD tempHolderForOrdinal;
	for (int i = 0; i < newExportDirectory->NumberOfNames - 1; i++) {
		for (int j = 0; j < newExportDirectory->NumberOfNames - i - 1; j++) {
			functionName1 = (PCHAR)(moduleAddress + nameArray[j]);
			functionName2 = (PCHAR)(moduleAddress + nameArray[j + 1]);
			if (strcmp(functionName1, functionName2) > 0) {
				tempHolderForName = nameArray[j];
				nameArray[j] = nameArray[j + 1];
				nameArray[j + 1] = tempHolderForName;
				tempHolderForOrdinal = ordinalArray[j];
				ordinalArray[j] = ordinalArray[j + 1];
				ordinalArray[j + 1] = tempHolderForOrdinal;
			}
		}
	}
}

void ReconstructExport(PBYTE moduleAddress) {
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleAddress;
	PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)(moduleAddress + dosHeader->e_lfanew);
	DWORD relativeVirtualAddressExportTableOld = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY newExportDirectory = (PIMAGE_EXPORT_DIRECTORY)exportTableBuffer;
	int oldNameArraySize = 0;
	int oldOrdinalArraySize = 0;
	int oldFunctionSize = 0;
	PDWORD oldNameArray = NULL;
	PDWORD oldFunctionArray = NULL;
	PWORD oldOrdinalArray = NULL;
	if (relativeVirtualAddressExportTableOld == 0) {
		// If there is no export, you can still export other functions
		newExportDirectory->Characteristics = 0;
		newExportDirectory->TimeDateStamp = -1; // Regular DLL compilation values
		newExportDirectory->MajorVersion = 0; // Regular DLL compilation values
		newExportDirectory->MinorVersion = 0; // Regular DLL compilation values
		newExportDirectory->Base = 1;
		newExportDirectory->Name = (DWORD)(((PBYTE) nameOfTheDll) - moduleAddress);
	}
	else {
		// If you don't want to hide all your exports, just put
		PIMAGE_EXPORT_DIRECTORY imageExportDirectoryOld = (PIMAGE_EXPORT_DIRECTORY)(moduleAddress + relativeVirtualAddressExportTableOld);
		newExportDirectory->Characteristics = imageExportDirectoryOld->Characteristics;
		newExportDirectory->TimeDateStamp = imageExportDirectoryOld->TimeDateStamp;
		newExportDirectory->MajorVersion = imageExportDirectoryOld->MajorVersion;
		newExportDirectory->MinorVersion = imageExportDirectoryOld->MinorVersion;
		newExportDirectory->Base = imageExportDirectoryOld->Base;
		newExportDirectory->Name = imageExportDirectoryOld->Name;
		oldNameArraySize = imageExportDirectoryOld->NumberOfNames;
		oldNameArray = (PDWORD)(imageExportDirectoryOld->AddressOfNames + moduleAddress);
		oldOrdinalArraySize = imageExportDirectoryOld->NumberOfNames;
		oldOrdinalArray = (PWORD)(imageExportDirectoryOld->AddressOfNameOrdinals + moduleAddress);
		oldFunctionSize = imageExportDirectoryOld->NumberOfFunctions;
		oldFunctionArray = (PDWORD)(imageExportDirectoryOld->AddressOfFunctions + moduleAddress);
	}
	/*[PLACEHOLDERFORNUMBEROFFUNCTIONSSET]*/
	/*[PLACEHOLDERFORNUMBEROFNAMESSET]*/

	// Given address smaller than DLL load adress
	PBYTE arrayArea = (PBYTE)arrayBuffer;
	newExportDirectory->AddressOfFunctions = (DWORD)(arrayArea - moduleAddress);
	newExportDirectory->AddressOfNames = (DWORD)(newExportDirectory->AddressOfFunctions + sizeof(DWORD) * newExportDirectory->NumberOfFunctions);
	newExportDirectory->AddressOfNameOrdinals = (DWORD)((newExportDirectory->AddressOfNames) + sizeof(DWORD) * newExportDirectory->NumberOfNames);
	DWORD totalNameSize = sizeof(namesBuffer);
	DWORD totalArraySize = newExportDirectory->NumberOfFunctions * sizeof(DWORD) * 2 + newExportDirectory->NumberOfNames * sizeof(WORD);

	SetFunctionAddresses(moduleAddress, newExportDirectory, oldFunctionArray, oldFunctionSize);
	SetNameArray(moduleAddress, newExportDirectory,oldNameArray, oldNameArraySize);
	SetOrdinalArray(moduleAddress, newExportDirectory,oldOrdinalArray,oldOrdinalArraySize,oldFunctionSize);

	SortExportedNameArray(moduleAddress, newExportDirectory);

	DWORD oldProtect = 0;
	VirtualProtect(&imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], sizeof(IMAGE_DATA_DIRECTORY), PAGE_READWRITE, &oldProtect);

	imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = (DWORD)((PBYTE)newExportDirectory - moduleAddress); // Change 1
	imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = sizeof(IMAGE_EXPORT_DIRECTORY) + totalArraySize + totalNameSize; // Change 2

	VirtualProtect(&imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], sizeof(IMAGE_DATA_DIRECTORY), oldProtect, NULL);
}




BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ReconstructExport((PBYTE)hModule);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

