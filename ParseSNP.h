#pragma once

#include "resource.h"
#include <string>

	

void ScreenUpdate(HWND hWnd,int unsigned x, PWSTR FilePath, PWSTR build, char sx);
bool PrintWithDialog(const std::string& str);
void PathoPrint(void);