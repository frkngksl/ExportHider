#pragma once
#include <iostream>

void PrintBanner();
void PrintHelp(char* binaryName);
bool ParseArgs(int argc, char* argv[], OPTIONS& configurationOptions);