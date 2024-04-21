#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <Windows.h>

std::vector<PCHAR> ParseInputFile(char* fileName);
BOOL WriteBinary(char* outputFileName, PBYTE fileBuffer, int fileSize);
PBYTE ReadBinary(char* fileName, int& fileSize);
PCHAR ReplacePlaceholder(PCHAR originalContent, PCHAR replacement,const char * placeholder);