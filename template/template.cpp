#include <windows.h>

char arrayBuffer[/*[PLACEHOLDERARRAYSIZE]*/] = { 0x0 };
char namesBuffer[/*[PLACEHOLDERNAMESSIZE]*/] = { 0x0 };
char exportTableBuffer[/*[PLACEHOLDEREXPORTTABLESIZE]*/] = { 0x0 };
char nameOfTheDll[] /*[PLACEHOLDERFORDLLNAME]*/;

void DebugTable(PBYTE moduleAddress) {
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleAddress;
	PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)(moduleAddress + dosHeader->e_lfanew);
	DWORD relativeVirtualAddressExportTable = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; // Change 1
	DWORD sizeExportTable = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size; // Change 2

	PIMAGE_EXPORT_DIRECTORY imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(moduleAddress + relativeVirtualAddressExportTable);
	PDWORD nameArray = (PDWORD)(moduleAddress + imageExportDirectory->AddressOfNames);
	PWORD ordinalArray = (PWORD)(moduleAddress + imageExportDirectory->AddressOfNameOrdinals);
	PDWORD addressArray = (PDWORD)(moduleAddress + imageExportDirectory->AddressOfFunctions);
	PCHAR functionName;
	DWORD functionAddr;

	for (unsigned int i = 0; i < imageExportDirectory->NumberOfNames; i++) {
		functionName = (PCHAR)(moduleAddress + nameArray[i]);
		functionAddr = addressArray[ordinalArray[i]];
	}

	for (unsigned int i = 0; i < imageExportDirectory->NumberOfFunctions; i++) {
		functionAddr = addressArray[i];
	}
}

void SetNameArray(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory) {
	char functionNames[] = /*[PLACEHOLDERNAMEARRAY]*/;
	PDWORD nameArray = (PDWORD)(moduleAddress + newExportDirectory->AddressOfNames);
	int numberOfFunctions = sizeof(functionNames) / sizeof(PCHAR);
	int cursorForNameArea = 0;
	for (int i = 0; i < numberOfFunctions; i++) {
		memcpy(namesBuffer + cursorForNameArea, (LPVOID) functionNames[i], strlen((const char*) functionNames[i])+1);
		nameArray[i] = (DWORD)((PBYTE)(namesBuffer + cursorForNameArea) - moduleAddress);
		cursorForNameArea += (strlen((const char*)functionNames[i]) + 1);
	}
}

void SetFunctionAddresses(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory) {
	PDWORD addressArray = (PDWORD)(moduleAddress + newExportDirectory->AddressOfFunctions);
	/*[PLACEHOLDERFORNUMBEROFFUNCTIONADDRESS]*/

	/*
	for (int i = 0; i < newExportDirectory->NumberOfFunctions; i++) {
		addressArray[i] = (DWORD)(((PBYTE)testFunc) - moduleAddress);
	}
	*/
}

void SetOrdinalArray(PBYTE moduleAddress, PIMAGE_EXPORT_DIRECTORY newExportDirectory) {
	PWORD ordinalArray = (PWORD)(moduleAddress + newExportDirectory->AddressOfNameOrdinals);

	for (int i = 0; i < newExportDirectory->NumberOfNames; i++) {
		ordinalArray[i] = i;
	}
}

void ReconstructExport(PBYTE moduleAddress) {
	PDWORD nameArray;
	PCHAR functionName;
	DWORD functionAddr;
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleAddress;
	PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)(moduleAddress + dosHeader->e_lfanew);
	DWORD relativeVirtualAddressExportTableOld = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	PIMAGE_EXPORT_DIRECTORY newExportDirectory = (PIMAGE_EXPORT_DIRECTORY)exportTableBuffer;
	if (relativeVirtualAddressExportTableOld == 0) {
		// If there is no export, you can still export other functions
		newExportDirectory->Characteristics = 0;
		newExportDirectory->TimeDateStamp = -1; // Regular DLL compilation values
		newExportDirectory->MajorVersion = 0; // Regular DLL compilation values
		newExportDirectory->MinorVersion = 0; // Regular DLL compilation values
		newExportDirectory->Base = 1;
		newExportDirectory->Name = (DWORD)(((PBYTE) nameOfTheDll) - moduleAddress);
		/*[PLACEHOLDERFORNUMBEROFFUNCTIONSSET]*/
		/*[PLACEHOLDERFORNUMBEROFNAMESSET]*/

	}
	else {
		// TODO
		// If you don't want to hide all your exports, just put
		PIMAGE_EXPORT_DIRECTORY imageExportDirectoryOld = (PIMAGE_EXPORT_DIRECTORY)(moduleAddress + relativeVirtualAddressExportTableOld);
		// nameArray = (PDWORD)(moduleAddress + imageExportDirectoryOld->AddressOfNames);
		// ordinalArray = (PWORD)(moduleAddress + imageExportDirectoryOld->AddressOfNameOrdinals);
		// addressArray = (PDWORD)(moduleAddress + imageExportDirectoryOld->AddressOfFunctions);
		newExportDirectory->Characteristics = imageExportDirectoryOld->Characteristics;
		newExportDirectory->TimeDateStamp = imageExportDirectoryOld->TimeDateStamp;
		newExportDirectory->MajorVersion = imageExportDirectoryOld->MajorVersion;
		newExportDirectory->MinorVersion = imageExportDirectoryOld->MinorVersion;
		newExportDirectory->Base = imageExportDirectoryOld->Base;
		newExportDirectory->Name = imageExportDirectoryOld->Name;

	}
	// Given address smaller than DLL load adress
	PBYTE arrayArea = (PBYTE)arrayBuffer;
	newExportDirectory->AddressOfFunctions = (DWORD)(arrayArea - moduleAddress);
	newExportDirectory->AddressOfNames = (DWORD)(newExportDirectory->AddressOfFunctions + sizeof(DWORD) * newExportDirectory->NumberOfFunctions);
	newExportDirectory->AddressOfNameOrdinals = (DWORD)((newExportDirectory->AddressOfNames) + sizeof(DWORD) * newExportDirectory->NumberOfNames);
	DWORD totalNameSize = sizeof(namesBuffer);
	DWORD totalArraySize = newExportDirectory->NumberOfFunctions * sizeof(DWORD) * 2 + newExportDirectory->NumberOfNames * sizeof(WORD);

	SetFunctionAddresses(moduleAddress, newExportDirectory);
	SetNameArray(moduleAddress, newExportDirectory);
	SetOrdinalArray(moduleAddress, newExportDirectory);

	DWORD oldProtect = 0;
	VirtualProtect(&imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], sizeof(IMAGE_DATA_DIRECTORY), PAGE_READWRITE, &oldProtect);

	imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = (DWORD)((PBYTE)newExportDirectory - moduleAddress); // Change 1
	imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = sizeof(IMAGE_EXPORT_DIRECTORY) + totalArraySize + totalNameSize; // Change 2

	VirtualProtect(&imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], sizeof(IMAGE_DATA_DIRECTORY), oldProtect, NULL);
	DebugTable(moduleAddress);
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

