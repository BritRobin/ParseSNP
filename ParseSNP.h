#pragma once

#include "resource.h"
#include <string>

void ScreenUpdate(HWND hWnd,int unsigned x, PWSTR FilePath, PWSTR build, char sx);
bool PrintWithDialog(const std::string& str);
void PathoPrint(void);
//more defensive code for invalid files
static constexpr int MAIN_TOTAL_BUFFER_SIZE = MAX_PATH;
static constexpr int MAIN_READ_LIMIT = 256;
//more defensive code for invalid files	