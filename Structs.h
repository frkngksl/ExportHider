#pragma once
#include<Windows.h>

typedef struct {
    LPSTR inputPath;
    LPSTR outputPath;
    LPSTR dllName;
    int numberOfOtherFunctions;
} OPTIONS;