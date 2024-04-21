#pragma once
#include <iostream>
#include "Structs.h"

void PrintBanner();
void PrintHelp(char* binaryName);
bool ParseArgs(int argc, char* argv[], OPTIONS& configurationOptions);