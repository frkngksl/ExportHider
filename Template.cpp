// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>


char arrayBuf[5024] = { 0x0 };
char nameBuf[5024] = { 0x1 };
char exportTableBuf[5024] = { 0x2 };

typedef void(*vFun)(void);

extern "C" __declspec(dllexport) void testFunc() {
	MessageBoxA(NULL, "Test", "Test2", 0);
}

void ReconstructExport(PBYTE moduleAddress) {
	memset(arrayBuf, 0x00, 5024);
	memset(nameBuf, 0x00, 5024);
	memset(exportTableBuf, 0x00, 5024);

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleAddress;
	PIMAGE_NT_HEADERS imageNTHeaders = (PIMAGE_NT_HEADERS)(moduleAddress + dosHeader->e_lfanew);
	DWORD relativeVirtualAddressExportTable = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; // Change 1
	DWORD sizeExportTable = imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size; // Change 2
	PIMAGE_EXPORT_DIRECTORY imageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(moduleAddress + relativeVirtualAddressExportTable);
	DWORD test = sizeof(IMAGE_EXPORT_DIRECTORY) + sizeof(DWORD) * imageExportDirectory->NumberOfFunctions + sizeof(DWORD) * imageExportDirectory->NumberOfNames + sizeof(WORD) * imageExportDirectory->NumberOfNames;
	PDWORD nameArray = (PDWORD)(moduleAddress + imageExportDirectory->AddressOfNames);
	PWORD ordinalArray = (PWORD)(moduleAddress + imageExportDirectory->AddressOfNameOrdinals);
	PDWORD addressArray = (PDWORD)(moduleAddress + imageExportDirectory->AddressOfFunctions);
	PCHAR functionName;
	DWORD functionAddr;
	PIMAGE_EXPORT_DIRECTORY newExportDir = (PIMAGE_EXPORT_DIRECTORY)exportTableBuf;
	newExportDir->Characteristics = imageExportDirectory->Characteristics;
	newExportDir->TimeDateStamp = imageExportDirectory->TimeDateStamp;
	newExportDir->MajorVersion = imageExportDirectory->MajorVersion;
	newExportDir->MinorVersion = imageExportDirectory->MinorVersion;
	newExportDir->Name = imageExportDirectory->Name;
	newExportDir->Base = imageExportDirectory->Base;
	newExportDir->NumberOfFunctions = 2;
	newExportDir->NumberOfNames = 2;
	DWORD totalArraySize = newExportDir->NumberOfFunctions * sizeof(DWORD) * 2 + newExportDir->NumberOfFunctions * sizeof(WORD);
	// Given address smaller than DLL load adress
	PBYTE arrayArea = (PBYTE)arrayBuf;
	newExportDir->AddressOfFunctions = (DWORD)(arrayArea - moduleAddress);
	newExportDir->AddressOfNames = (DWORD)(newExportDir->AddressOfFunctions + sizeof(DWORD) * newExportDir->NumberOfFunctions);
	newExportDir->AddressOfNameOrdinals = (DWORD)((newExportDir->AddressOfNames) + sizeof(DWORD) * newExportDir->NumberOfNames);
	DWORD totalNameSize = 10;
	PBYTE nameArea = (PBYTE)nameBuf;
	memcpy(nameArea, "test", 5);
	memcpy(nameArea + 5, "test", 5);
	addressArray = (PDWORD)(moduleAddress + newExportDir->AddressOfFunctions);
	ordinalArray = (PWORD)(moduleAddress + newExportDir->AddressOfNameOrdinals);
	nameArray = (PDWORD)(moduleAddress + newExportDir->AddressOfNames);
	for (int i = 0; i < newExportDir->NumberOfFunctions; i++) {
		addressArray[i] = (DWORD)(((PBYTE)testFunc) - moduleAddress);
	}
	for (int i = 0; i < newExportDir->NumberOfNames; i++) {
		ordinalArray[i] = i;
	}
	for (int i = 0; i < newExportDir->NumberOfNames; i++) {
		nameArray[i] = (DWORD)((nameArea + i * 5) - moduleAddress);
	}
	DWORD oldProtect = 0;
	VirtualProtect(&imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], sizeof(IMAGE_DATA_DIRECTORY), PAGE_READWRITE, &oldProtect);

	imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress = (DWORD)((PBYTE)newExportDir - moduleAddress); // Change 1
	imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = sizeof(IMAGE_EXPORT_DIRECTORY) + totalArraySize + totalNameSize; // Change 2

	VirtualProtect(&imageNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT], sizeof(IMAGE_DATA_DIRECTORY), oldProtect, NULL);

	// Check Again
	PIMAGE_DOS_HEADER dosHeader3 = (PIMAGE_DOS_HEADER)moduleAddress;
	PIMAGE_NT_HEADERS imageNTHeaders3 = (PIMAGE_NT_HEADERS)(moduleAddress + dosHeader3->e_lfanew);
	DWORD relativeVirtualAddressExportTable3 = imageNTHeaders3->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; // Change 1
	DWORD sizeExportTable3 = imageNTHeaders3->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size; // Change 2

	PIMAGE_EXPORT_DIRECTORY imageExportDirectory3 = (PIMAGE_EXPORT_DIRECTORY)(moduleAddress + relativeVirtualAddressExportTable3);
	PDWORD nameArray3 = (PDWORD)(moduleAddress + imageExportDirectory3->AddressOfNames);
	PWORD ordinalArray3 = (PWORD)(moduleAddress + imageExportDirectory3->AddressOfNameOrdinals);
	PDWORD addressArray3 = (PDWORD)(moduleAddress + imageExportDirectory3->AddressOfFunctions);
	PCHAR functionName3;
	DWORD functionAddr3;

	for (unsigned int i = 0; i < imageExportDirectory->NumberOfNames; i++) {
		functionName3 = (PCHAR)(moduleAddress + nameArray3[i]);
		functionAddr3 = addressArray3[ordinalArray3[i]];
	}

	for (unsigned int i = 0; i < imageExportDirectory->NumberOfFunctions; i++) {
		functionAddr3 = addressArray3[i];
	}

}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	PBYTE test = (PBYTE)VirtualAlloc(NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
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

