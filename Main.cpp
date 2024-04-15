#include <Windows.h>
#include <iostream>



void PrintBanner() {
	const char* banner =
	"   __                       _          _     _                  \n"      
	"  /__\\_  ___ __   ___  _ __| |_  /\\  /(_) __| | ___ _ __      \n"
	" /_\\ \\ \\/ / '_ \\ / _ \\| '__| __|/ /_/ / |/ _` |/ _ \\ '__| \n"
	"//__  >  <| |_) | (_) | |  | |_/ __  /| | (_| |  __/ |          \n"
	"\\__/ /_/\\_\\ .__/ \\___/|_|   \\__\\/ /_/ |_|\\__,_|\\___|_|  \n"
	"	  |_|                                                        \n"
	"		     by @R0h1rr1m							             \n";
	std::cout << banner << std::endl;
}

int main(int argc, char* argv[]) {
	PrintBanner();
	return 0;
}