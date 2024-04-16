#include "AuxFunctions.h"

std::vector<PCHAR> ParseInputFile(char* fileName) {
    std::vector<PCHAR> returnList;
    std::string line;
	std::ifstream file(fileName);
    // Check if the file is open
    if (!file.is_open()) {
        std::cout << "[!] Can't open the input file: " << fileName << std::endl;
        return returnList;
    }

    // Read each line of the file
    while (std::getline(file, line)) {
        // Allocate memory for char array using HeapAlloc
        char* lineCopy = (PCHAR) (HeapAlloc(GetProcessHeap(), 0, (line.length() + 1) * sizeof(char)));
        std::strcpy(lineCopy, line.c_str());
        returnList.push_back(lineCopy); // Store the line in the vector
    }
    return returnList;
}