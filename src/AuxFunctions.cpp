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

PBYTE ReadBinary(char* fileName, int& fileSize) {
	PBYTE fileBuffer;
	// Get a file handle
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		std::cout << "CreateFileA Error: " << GetLastError() << std::endl;
		return NULL;
	}
	// Get size of that file
	fileSize = GetFileSize(fileHandle, NULL);
	if (fileSize == INVALID_FILE_SIZE) {
		std::cout << "GetFileSize Error: " << GetLastError() << std::endl;
		return NULL;
	}
	// Allocate a data buffer
	fileBuffer = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, fileSize+1);
	if (fileBuffer == NULL) {
		std::cout << "HeapAlloc Error: " << GetLastError() << std::endl;
		return NULL;
	}
	// Read the file and put into the buffer
	if (ReadFile(fileHandle, fileBuffer, fileSize, NULL, NULL) == FALSE) {
		std::cout << "ReadFile Error: " << GetLastError() << std::endl;
		return NULL;
	}
	CloseHandle(fileHandle);
    fileBuffer[fileSize] = '\0';
	// Return the buffer
	return fileBuffer;
}


BOOL WriteBinary(char* outputFileName, PBYTE fileBuffer, int fileSize) {
    HANDLE fileHandle = CreateFileA(outputFileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cout << "CreateFileA Error: " << GetLastError() << std::endl;
        return FALSE;
    }
    BOOL writeResult = WriteFile(fileHandle, fileBuffer, fileSize, NULL, NULL);
    if (writeResult == FALSE) {
        std::cout << "WriteFile Error: " << GetLastError() << std::endl;
        return FALSE;
    }
    CloseHandle(fileHandle);
    return TRUE;
}


PCHAR ReplacePlaceholder(PCHAR originalContent, PCHAR replacement, const char* placeholder) {
    // Calculate the length of the new string after replacement
    int originalContentLength = strlen(originalContent);
    int replacementLength = strlen(replacement);
    int placeholderLength = strlen(placeholder);

    int newLength = originalContentLength + (replacementLength - placeholderLength);

    // Allocate memory for the new string using HeapAlloc
    char* newMemory = (char*)HeapAlloc(GetProcessHeap(), 0, (newLength + 1) * sizeof(char));

    if (newMemory == NULL) {
        std::cout << "HeapAlloc Error: " << GetLastError() << std::endl;
        return NULL;
    }

    // Scan the original string and replace placeholders
    int i = 0, j = 0;
    while (originalContent[i] != '\0') {
        // Check if the current position matches the placeholder
        if (strncmp(&originalContent[i], placeholder, placeholderLength) == 0) {
            // Copy the replacement string into the new string
            strncpy(&newMemory[j], replacement, replacementLength);
            i += placeholderLength; // Move the index in the original string
            j += replacementLength; // Move the index in the new string
        }
        else {
            // Copy the character from the original string
            newMemory[j] = originalContent[i];
            i++;
            j++;
        }
    }
    newMemory[j] = '\0'; // Null-terminate the new string
    HeapFree(GetProcessHeap(), 0, originalContent);
    return newMemory;
}

PCHAR AppendToOutput(PCHAR output, PCHAR text) {
    // Calculate the length of the current output and the text to be appended
    int outputLen = output ? strlen(output) : 0;
    int textLen = strlen(text);
    char* newOutput = NULL;
    if (output == NULL) {
        newOutput = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (outputLen + textLen + 2) * sizeof(char));

    }
    else {
        // Allocate memory for the new output string
        newOutput = (char*)HeapReAlloc(GetProcessHeap(), 0, output, (outputLen + textLen + 2) * sizeof(char));
    }
    if (newOutput == NULL) {
        std::cout << "HeapReAlloc Error: " << GetLastError() << std::endl;
        return NULL;
    }
    // Append the text and a newline character to the new output string
    strcat(newOutput, text);
    return newOutput;
}