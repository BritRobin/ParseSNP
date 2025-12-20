// ParseSNP.cpp : Defines the entry point for the application.
//Visual Studio 2022 build

#include "framework.h"
#include "ParseSNP.h"
#include "SnipParser.h"
#include <shobjidl_core.h>
#include <string>
#include <atlstr.h>
#include <atlconv.h>  // For CT2CA
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <Winuser.h>
#include <shlobj_core.h>
#include <shtypes.h>
#include <filesystem>
#include <shellapi.h>
#include <sstream>
#include <commdlg.h>
#include "MD5.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <winspool.h>
#pragma comment(lib, "winspool.lib")
#include <memory>
#include <vector>
//temp
#include <iostream>

#define MAX_LOADSTRING 100
namespace fs = std::filesystem; // In C++17 

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//Global class instance
SnipParser x;
//START:Global returned values
HWND ghWnd = nullptr;   // Add initialization
HWND aDiag = nullptr;   // Add initialization
int rs_number = 0;      // RS Number 
int position = 0;       // relative position
int loadedFiletype = 0; // 1=Ancestory 2=FTNDA 3=23toM3
int mergeLoad = 0;      // 1=Ancestory 2=FTNDA 4=23toM3
int mergeTotal = 0;
char chromosome[8] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };//chromosome number
char allele1 = NULL, allele2 = NULL;
wchar_t global_s[260];
std::wstring currentProject;
std::wstring myPath;
std::wstring SourceFilePath;
//END:Global returned values

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ProjectDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Deletemsg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Pathogen(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MergeWarnmsg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MergeAbortmsg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MergeReportmsg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SearchFailmsg(HWND, UINT, WPARAM, LPARAM);
//bool RawTextToPrinter(const std::string& text);
static bool IsLocaleUS();
static int GetTextWidth(HDC hdc, const std::wstring& s);
bool PrintPlainText(const std::wstring& text);
std::wstring GetCurrentDateString(void);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PARSESNP, szWindowClass, MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PARSESNP));
  

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PARSESNP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);//FIX Weird backgroud 0.2beta
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PARSESNP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_PARSESNP));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   const int width = 1000;
   const int height = 700;

   //Thanks to DeepSeek for centering code
   // Calculate centered position
   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);
   int x = (screenWidth - width) / 2;
   int y = (screenHeight - height) / 2;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, x, y, width, height, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
       return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   PWSTR   ppszPath;    // variable to receive the path memory block pointer.

   HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &ppszPath);

   //if first run create parsesnp projects folder!
   if (SUCCEEDED(hr)) {
       std::wstring addit = L"\\ParseSNP\\Projects";
       myPath = ppszPath;// +,";      // make a local copy of the path
       myPath = myPath + addit;
  
      try {
           fs::create_directories(myPath);
       }
       catch (std::exception& e) { // Not using fs::filesystem_error since std::bad_alloc can throw too.
           std::cout << e.what() << std::endl;
       }
           CoTaskMemFree(ppszPath);    // free up the path memory block
   }
   return TRUE;
}

//Form Control Message Handler
INT_PTR CALLBACK FormDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    HANDLE hBmp;
    switch (Message)
    {

    case WM_INITDIALOG:
    {   
        HBITMAP hBmp = (HBITMAP)LoadImage(GetModuleHandleA(nullptr), MAKEINTRESOURCE(IDB_BITMAP2), IMAGE_BITMAP, 81, 24, LR_DEFAULTCOLOR);
        //check operation suceeded
        if(hBmp != NULL) SendMessage(GetDlgItem(hwnd, IDC_BUTTON1), (UINT)BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBmp);
        
        // Store the bitmap handle so we can delete it later
        SetProp(hwnd, L"BUTTONBITMAP", hBmp);

        LOGFONT lf = { 0 };
        GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
        lf.lfHeight = 16;
        _tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Courier New"));
        HFONT hFont = CreateFontIndirect(&lf);

        if (hFont) {
            // Store font in window property
            SetProp(hwnd, L"LIST2FONT", hFont);
            SendDlgItemMessage(hwnd, IDC_LIST2, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        return TRUE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        
        case IDC_BUTTON1: {
            TCHAR buffer[16] = { 0 };

            if (GetWindowText(GetDlgItem(aDiag, IDC_EDIT1), buffer, _countof(buffer)))
            {
                int local_rs_number;
                local_rs_number = _wtoi(buffer);
                if (local_rs_number > 0) {
                    std::wstring urlsnp = L"https://www.snpedia.com/index.php/RS";
                    urlsnp += (buffer);
                    //Try to find SNP on SNPedia               
                    if ( ((INT_PTR)ShellExecute(NULL, TEXT("open"), urlsnp.c_str(), NULL, NULL, SW_SHOWNORMAL)) <= 32) return FALSE ;
                    else return TRUE;
                }
            }   
        }
        break;
        case IDC_LIST2:
        {
            switch (HIWORD(wParam))
            {
                //as an entry been double clicked
            case LBN_DBLCLK:
                int signed lcount = 0;
                int gselected;
                int local_rs_number = 0;
                HWND plst = GetDlgItem(aDiag, IDC_LIST2);
                lcount = (int)SendMessage(plst, LB_GETCOUNT, 0, 0);
                if (lcount > 0 && lcount != -1)//Ensure on left double click there are to lookup
                {
                    TCHAR buffer[256];
                    gselected = SendMessage(plst, LB_GETCURSEL, 0, 0);
                    //get selected text
                    SendMessage(plst, LB_GETTEXT, (WPARAM)gselected, (LPARAM)buffer);
                    if (buffer[0] == L'R' && buffer[1] == L'S') {

                        //selected is an RS nummber
                        char lookup[16] = { 0 }, lbuffer[16] = { 0 };
                        CT2CA pszConvertedAnsiString(buffer);
                        PSTR a;
                        a = StrStrA(pszConvertedAnsiString, "N/A");

                        //Moved up for none available data
                        memcpy_s(lbuffer, 15, pszConvertedAnsiString, 15);
                        for (int i = 2; i < 18; i++)
                        {
                            if (lbuffer[i] == L' ') break;
                            if (isdigit((unsigned char)lbuffer[i])) {
                                lookup[i - 2] = lbuffer[i];
                            }
                        }
                        local_rs_number = atoi(lookup);
                        //if the rsid is not in our dataset don't search for it again!
                        if(a != (PSTR)0) {//11-13-2025 This could be confusing if a previous result is left loaded!
                            //Covert RS number for display
                            std::string s = std::to_string(local_rs_number);
							USES_CONVERSION_EX; //Added NULL pointer check 11-22-2025
                            LPWSTR lp = A2W_EX(s.c_str(), s.length());
                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT1), lp);
                            s = (std::string)"--";
                            lp = A2W_EX(s.c_str(), s.length());
                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_CHRNUM), lp);
                            s = (std::string)"0";
                            lp = A2W_EX(s.c_str(), s.length());
                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_POSIT), lp);
                            s = (std::string)"-";
                            lp = A2W_EX(s.c_str(), s.length());
                            if (lp != NULL) {
                                SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_ALLES1), lp);
                                SetWindowTextW(GetDlgItem(aDiag, IDC_AllELE2), lp);
                            }
                            //trigger WM_PAINT              
                            InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                            UpdateWindow(aDiag);
                            break;
                         }
                        //RSID IS in our dataset copy the local variable to the global!
                        rs_number = local_rs_number; //Local to Global
                        //if we have a number perform search!
                        if (rs_number > 0)
                        {
                            //populate global values
                            if (x.RsSearch(&rs_number, &chromosome[0], &chromosome[1], &chromosome[2], &chromosome[3], &position, &allele1, &allele2))
                            {
                                //Covert RS number for display
                                std::string s = std::to_string(rs_number);
								USES_CONVERSION_EX; //Added NULL pointer check 11-22-2025
                                LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT1), lp);
                                //Covert chromosome for display
                                s = chromosome;
                                lp = A2W_EX(s.c_str(), s.length());
                                if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_CHRNUM), lp);
                                //covert poistion for display
                                s = std::to_string(position);
                                lp = A2W_EX(s.c_str(), s.length());
                                if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_POSIT), lp);
                                s = allele1;
                                lp = A2W_EX(s.c_str(), s.length());
                                if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_ALLES1), lp);
                                //Fix 'X' chromosone in men as ancestoy dna files populate both  alleles as it make searches easier?
                                if (chromosome[0] == 'X' && x.sex() == 'M') allele2 = '-';
                                s = allele2;
                                lp = A2W_EX(s.c_str(), s.length());
                                if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_AllELE2), lp);
                                EnableWindow(GetDlgItem(aDiag, IDC_COPYCLIP), TRUE);
                                EnableWindow(GetDlgItem(aDiag, IDC_COPYPROJ), TRUE);
                                //trigger WM_PAINT              
                                InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                UpdateWindow(aDiag);
                                
                            }
                        }
                    }
                }
             return TRUE;
            }
            return TRUE;
        }
        case IDC_LIST3:
        {
          switch (HIWORD(wParam))
            { 
             //as an enry been double clicked
             case LBN_DBLCLK:
             int signed lcount = 0;
             int gselected;
             HWND plst = GetDlgItem(aDiag, IDC_LIST3);
             lcount = (int)SendMessage(plst, LB_GETCOUNT, 0, 0);
             if(lcount > 0 && lcount != -1)//Ensure on left double click there are entries to delete!
             { 
                if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), aDiag, Deletemsg) == TRUE) {
                 gselected = SendMessage(plst, LB_GETCURSEL, 0, 0);
                 //Delete entry an redraw to update
                 SendMessage(GetDlgItem(hwnd, IDC_LIST3), LB_DELETESTRING, gselected, 0);
                 if (gselected == 0) EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                } //trigger WM_PAINT  
             InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
             UpdateWindow(aDiag);
             return TRUE;
             }
            
            }
            return TRUE;
        }
        case IDC_BUTTON_SEARCH:
        {
           TCHAR buffer[16] = { 0 };

              if (GetWindowText(GetDlgItem(aDiag, IDC_EDIT_SEARCH), buffer, _countof(buffer)))
               {
                  //new in ver 4.6
                  TCHAR cleanbuffer[15] = { 0 };
                  //Ctrl+V can paste alpha 
                  for (int pos = 0,cln=0; pos < 15; pos++)
                  {
                      if (isdigit((unsigned char)buffer[pos]))
                        {
                         cleanbuffer[cln] = buffer[pos];
                         cln++;
                         cleanbuffer[cln] = '\0';//terminate the string
                        }
                  }
                  //re-write the correct numeric part back to the search box
                  LPWSTR clnd = cleanbuffer;
                  SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), clnd);

                  rs_number = _wtoi(cleanbuffer);//now search sanitized input

                  //populate global values
                  if (x.RsSearch(&rs_number, &chromosome[0], &chromosome[1], &chromosome[2], &chromosome[3], &position, &allele1, &allele2))
                  {
                      //Covert RS number for display
                      std::string s = std::to_string(rs_number);
					  USES_CONVERSION_EX; //Added NULL pointer check 11-22-2025
                      LPWSTR lp = A2W_EX(s.c_str(), s.length());
                      if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT1), lp);
                      //Covert chromosome for display
                      s = chromosome;
                      lp = A2W_EX(s.c_str(), s.length());
                      if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_CHRNUM), lp);
                      //covert poistion for display
                      s = std::to_string(position);
                      lp = A2W_EX(s.c_str(), s.length());
                      if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_POSIT), lp);
                      s = allele1;
                      lp = A2W_EX(s.c_str(), s.length());
                      if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_ALLES1), lp);
                      //Fix 'X' chromosone in men as ancestoy dna files populate both  alleles as it make searches easier?
                      if (chromosome[0] == 'X' && x.sex() == 'M') allele2 = '-';
                      s = allele2;
                      lp = A2W_EX(s.c_str(), s.length());
                      if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_AllELE2), lp);
                      EnableWindow(GetDlgItem(aDiag, IDC_COPYCLIP), TRUE);
                      EnableWindow(GetDlgItem(aDiag, IDC_COPYPROJ), TRUE);
                      //trigger WM_PAINT              
                      InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                      UpdateWindow(aDiag);
                  }
                  else
                  { //Not Found dialog
                      DialogBox(hInst, MAKEINTRESOURCE(IDD_NOTFOUND), aDiag, SearchFailmsg);
                      return FALSE;
                  }
               }
               return TRUE;
        }
        case IDC_COPYPROJ:
        {
         TCHAR buffer[16] = { 0 };
		 //Stop non found references from pathogenics being added
         if (GetWindowText(GetDlgItem(aDiag, IDC_EDIT1), buffer, _countof(buffer)))
         {
             int local_rs_number;
             local_rs_number = _wtoi(buffer);
             if (local_rs_number != rs_number) return FALSE;
         }
         int lindex = -1;
         int lcount = -1;
         //Character allign chr numver and letters
         std::string chr_s= (std::string)(chromosome + NULL);

         if (atoi(chromosome) < 9 && chr_s != "MT") chr_s = " " + chr_s;

         std::string s = " Chromosome: " + chr_s + "  RSID " + " RS" + std::to_string(rs_number) + "  Position: " + std::to_string(position) + "  Alleles: " + allele1 + "  " + allele2 + "";
         std::wstring str2(s.length(), L' '); // Make room for characters

         // Copy string to wstring.
		 //Added proper conversion for non-ASCII characters AI suggested code nence the formatting change
         std::wstring ws;
         int len = MultiByteToWideChar(
             CP_ACP, 0,
             s.c_str(), -1,
             nullptr, 0
         );

         ws.resize(len - 1);

         MultiByteToWideChar(
             CP_ACP, 0,
             s.c_str(), -1,
             ws.data(), len
         );
         wcsncpy_s(
             global_s,
             _countof(global_s),
             ws.c_str(),
             _TRUNCATE
         );

         lcount = SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_GETCOUNT, 0, 0);
         if(lcount > 0)   lindex = SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_FINDSTRING, 0, (LPARAM)global_s); //prevent dulpicates
         if(lindex == -1)  SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_ADDSTRING, 0, (LPARAM)global_s);
         if(lcount == 0) EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_ENABLED);

            //trigger WM_PAINT              
         InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
         UpdateWindow(aDiag);
         return TRUE;

        }
        } // End of LOWORD(wParam) switch
        break;
    } // End of WM_COMMAND case
    case WM_DESTROY:
    {
         // Cleanup code - moved to proper location
         HBITMAP hBmp = (HBITMAP)RemoveProp(hwnd, L"BUTTONBITMAP");
        if (hBmp) {
          DeleteObject(hBmp);
         }

         HFONT hFont = (HFONT)RemoveProp(hwnd, L"LIST2FONT");
        if (hFont) {
          DeleteObject(hFont);
         }

        break;
    }

    default:
    return FALSE;
    }
 return TRUE;
 }
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
         // other commands
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case ID_FILE_LOADPROJECT:
        {   
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        PWSTR pszFilePath = nullptr;
        if (SUCCEEDED(hr))
        {
            IFileOpenDialog* pFileOpen = nullptr;

            // Create the FileOpenDialog object.
            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

            if (SUCCEEDED(hr))
            {
                LPCWSTR a = L"Project Folder";

                COMDLG_FILTERSPEC rgSpec[] =
                {
                    {a, L"*.ptxt"},
                };
                //set file type options
                hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                // Show the Open dialog box.
                hr = pFileOpen->Show(NULL);

                // Get the file name from the dialog box.
                if (SUCCEEDED(hr))
                {
                    IShellItem* pItem;
                    hr = pFileOpen->GetResult(&pItem);
                    if (SUCCEEDED(hr))
                    {
                       
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr))
                        { 
                            USES_CONVERSION;
                            LPCWSTR Target;
                            CString temp = "ProjectManifest.ptxt";
                            std::wstring str;
                            std::string  linebuffer;
                            char lbuffer[260];
                            //configPath = currentProject.c_str() + temp;
                            Target = pszFilePath;
                            currentProject = pszFilePath;
                            currentProject = currentProject.substr(0, currentProject.length() - temp.GetLength());//SET CURRENT PROJECT!!!
                            //filestream 
                            std::fstream  fstrm;
                            fstrm.open(Target, std::ios::in);
                            //Check file was opened  
                            if (fstrm.is_open()) {
                                while (fstrm.getline(lbuffer, 256))//read a line into a temorary buffer
                                {
                                    //DNA file to open
                                    //file name and path wide                         
                                    str = A2T(lbuffer);
                                    fstrm.getline(lbuffer, 256);//DNA file type to open
                                    int xsw = atoi(lbuffer);

                                    // 1=Ancestory 2=FTNDA 3=23toM3
                                    if (xsw == 1) {
                                        int unsigned count;
                                        if (x.Ancestory((PWSTR)str.c_str()))
                                        {
                                            LPWSTR lp = const_cast<LPTSTR>(TEXT("0"));
                                            SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                            SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                            //Update count and path per normal
                                            count = x.SNPCount();
                                            //New code Beta 0.2
                                            std::string strx;
                                            strx = x.NCBIBuild();
                                            CA2CT pszConvertedAnsiString(strx.c_str());
                                            HWND plst = GetDlgItem(aDiag, IDC_LIST2);
                                            SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR pathy listbox
                                            //New code Beta 0.2
                                            SourceFilePath = str;
                                            loadedFiletype = 1;
                                            mergeLoad = mergeTotal = 0;
                                            ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString, x.sex());
                                        }
                                    }
                                    else  if (xsw == 2) {
                                        unsigned int count;
                                        unsigned int tranlated;
                                        unsigned int untranslated;
                                        if (x.FTDNA((PWSTR)str.c_str()))
                                        {
                                            count = x.SNPCount();
                                            tranlated = x.IllumTransVG();
                                            untranslated = x.IllumUntransVG();
                                            //Updated translated VG to RSID
                                            std::string s = std::to_string(tranlated);
											USES_CONVERSION_EX;//Added NULL pointer check 11-22-2025
                                            LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                            //Unpdate ramining disgarded VG code lines
                                            s = std::to_string(untranslated);
                                            lp = A2W_EX(s.c_str(), s.length());
                                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                            //Update count and path per normal
                                             //New code Beta 0.2
                                            std::string strx;
                                            strx = x.NCBIBuild();
                                            CA2CT pszConvertedAnsiString(strx.c_str());
                                            HWND plst = GetDlgItem(aDiag, IDC_LIST2);
                                            SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR pathy listbox
                                            //New code Beta 0.2
                                            SourceFilePath = str;
                                            loadedFiletype = 2;
                                            mergeLoad = mergeTotal = 0;
                                            ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString, x.sex());
                                        }
                                    }
                                    else if (xsw == 3) {
                                        unsigned int count;
                                        unsigned int tranlated;
                                        unsigned int untranslated;
                                        if (x.f23andMe((PWSTR)str.c_str()))
                                        {
                                            count = x.SNPCount();
                                            tranlated = x.IllumTransVG();
                                            untranslated = x.IllumUntransVG();
                                            //Updated translated VG to RSID
                                            std::string s = std::to_string(tranlated);
											USES_CONVERSION_EX;//Added NULL pointer check 11-22-2025
                                            LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                            //Undate remaining disgarded VG code lines
                                            s = std::to_string(untranslated);
                                            lp = A2W_EX(s.c_str(), s.length());
                                            if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                            //Update count and path per normal
                                            //New code Beta 0.2
                                            std::string strx;
                                            strx = x.NCBIBuild();
                                            CA2CT pszConvertedAnsiString(strx.c_str());
                                            HWND plst = GetDlgItem(aDiag, IDC_LIST2);
                                            SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR pathy listbox
                                             //New code Beta 0.2
                                            SourceFilePath = str;
                                            loadedFiletype = 3;
                                            mergeLoad = mergeTotal = 0;
                                            ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString, x.sex());
                                        }

                                    }

                                    fstrm.getline(lbuffer, 256);
                                    xsw = atoi(lbuffer);
                                    if (xsw == 0) {
                                        fstrm.close();
                                        if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
                                        CoUninitialize(); //Needed to be moved here
                                        break;
                                    }

                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox                          

                                    for (int lst = 0; lst < xsw; lst++) {
                                        fstrm.getline(lbuffer, 256);//read project line
                                        int i = 0; // moved outside loop scope
                                        std::string s = lbuffer;
                                        std::wstring str2(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        if(lst==1)EnableMenuItem(GetMenu(hWnd), ID_PROJEX, MF_BYCOMMAND | MF_ENABLED);
                                    }
                                    fstrm.close();
                                    if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
                                    CoUninitialize(); //Needed to be moved here
                                    InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                    UpdateWindow(aDiag);
                                    break;
                                }
                                
                              }
                            }
                        
                    }
                    
                    
                }
            }
            if(pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object 12/20/2025
            CoUninitialize();//Was Missing
        }
        break;
        }
        case ID_FILE_SAVEPROJECT:
        { 
        if (currentProject.length() == 0) {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), aDiag, ProjectDlgProc);
            if (currentProject.length() == 0) break;
            //create project folder
            fs::create_directory(currentProject);
        }
        //Copy Source File
        if (SourceFilePath.length() > 0)
        {   /* Derive the target path from source file and project path */
            // Extract filename safely using filesystem
			//ChatGPT4 suggested method start
            std::filesystem::path fullpath(SourceFilePath);
            std::wstring filename = fullpath.filename().wstring();   // Always safe
			//ChatGPT4 suggested method end
            // Build final project file path
            std::wstring pathfilname = currentProject + filename;
            //Copy the original DNA file retaining its original name to the project folder
            LPCWSTR Source = SourceFilePath.c_str();
            LPCWSTR Target = pathfilname.c_str();
            if(mergeTotal==0)  //new for ver 0.3 beta write merged files as exported ancestory files with original name
             { 
               CopyFile(Source, Target, TRUE);
              } else {
                       x.AncestoryWriter((PWSTR)Target);
                       loadedFiletype = 1;
                      }
            //Write Project configuration files
            //Open file for write 
            std::fstream  fstrm;
            {
                CString temp = "\\ProjectManifest.ptxt", configPath;
                std::string  lbuffer, linebuffer;
                int lcount;
                char tc[3];
                configPath = currentProject.c_str() + temp;
                Target = configPath;
                //filestream 
                fstrm.open(Target, std::ios::out);
                //Check file was opened  
                if (fstrm.is_open()) {
                    // Convert a TCHAR string to a LPCSTR
                    CT2CA pszConvertedAnsiString(pathfilname.c_str());
                    lbuffer = pszConvertedAnsiString; //the full path to the file to load
                    lbuffer += "\n";
                    //Protection for alteration to the code overflowing tc[] buffer should ALWAYS work in existing code
                    if (loadedFiletype < 9) _itoa_s(loadedFiletype, tc, 10); //the function to use to load it
                    else {
                           tc[0] = '1';
                           tc[1] = '\0';
                          }
                    lbuffer += tc;
                    lbuffer += "\n";
                    //Get Project window entry count
                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                    lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);
                    _itoa_s(lcount, tc, 10);
                    lbuffer += tc;
                    lbuffer += "\n";
                    TCHAR text[256];
                    
                    for (int x = 0; x < lcount; x++)
                    {
                        int ln = (int)SendMessage(plst, LB_GETTEXTLEN, x, 0);
                        if (ln == LB_ERR) continue;

                        if (ln >= (int)_countof(text))
                            ln = (int)_countof(text) - 1;

                        SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
                        text[ln] = TEXT('\0');   // REQUIRED

                        CT2CA pszConvertedAnsiString(text);
                        lbuffer += pszConvertedAnsiString;
                        lbuffer += "\n";
                    }
                    const char* write_it = lbuffer.c_str();
                    fstrm.write(write_it, lbuffer.length());
                    fstrm.close();
                }
            }
        }
        break;
        }
        case ID_OPEN23: //almost idenitcal format same code handles both
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                unsigned int count;
                                unsigned int tranlated;
                                unsigned int untranslated;
                                if (x.f23andMe(pszFilePath))
                                {
                                    count = x.SNPCount();
                                    tranlated = x.IllumTransVG();
                                    untranslated = x.IllumUntransVG();
                                    //Updated translated VG to RSID
                                    std::string s = std::to_string(tranlated);
									USES_CONVERSION_EX; //Added NULL pointer check 11-22-2025
                                    LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                    if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                    //Update remaining disgarded VG code lines
                                    s = std::to_string(untranslated);
                                    lp = A2W_EX(s.c_str(), s.length());
                                    if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                    //Update count and path per normal
                                    //New code Beta 0.2
                                    std::string strx;
                                    strx = x.NCBIBuild();
                                    CA2CT pszConvertedAnsiString(strx.c_str());
                                    //New code Beta 0.2
                                    SourceFilePath = pszFilePath;
                                    loadedFiletype = 3;
                                    mergeLoad = mergeTotal = 0;
                                    ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString, x.sex());
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                    InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                    UpdateWindow(aDiag);
                                }
                                if (pszFilePath) if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
				CoUninitialize(); //Needed to be moved here
            }
            break;
        }
        case ID_FILE_OPENFTDNA:
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                unsigned int count;
                                unsigned int tranlated;
                                unsigned int untranslated;
                                if (x.FTDNA(pszFilePath))
                                {
                                    count = x.SNPCount();
                                    tranlated = x.IllumTransVG();
                                    untranslated = x.IllumUntransVG();
                                    //Updated translated VG to RSID
                                    std::string s = std::to_string(tranlated);
									USES_CONVERSION_EX;//Added NULL pointer check 11-22-2025
                                    LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                    if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                    //Unpadate ramining disgarded VG code lines
                                    s = std::to_string(untranslated);
                                    lp = A2W_EX(s.c_str(), s.length());
                                    if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                    //Update count and path per normal
                                    //New code Beta 0.2
                                    std::string strx;
                                    strx = x.NCBIBuild();
                                    CA2CT pszConvertedAnsiString(strx.c_str());
                                    //New code Beta 0.2
                                    SourceFilePath = pszFilePath;
                                    loadedFiletype = 2;
                                    mergeLoad = mergeTotal = 0;
                                    ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString, x.sex());
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox  
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED); 
                                    InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                    UpdateWindow(aDiag);
                                }
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
				CoUninitialize();//needed to be moved here
            }
            break;
        }
        case ID_OPENFILE:
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                int unsigned count;
                                if (x.Ancestory(pszFilePath))
                                {
                                    LPWSTR lp = const_cast<LPTSTR>(TEXT("0"));
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                    //Update count and path per normal
                                    count = x.SNPCount();
                                    //New code Beta 0.2
                                    std::string strx;
                                    strx = x.NCBIBuild();
                                    CA2CT pszConvertedAnsiString(strx.c_str());
                                    //New code Beta 0.2
                                    SourceFilePath = pszFilePath;
                                    loadedFiletype = 1;
                                    mergeLoad = mergeTotal = 0;
                                    ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString, x.sex());
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                    InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                    UpdateWindow(aDiag);                                    
                                }
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
                CoUninitialize(); //was in the wrong place
            }
        }
        break;
        case ID_FILE_MERGEANCESTORYDNATXTFILE: {

            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                DialogBox(hInst, MAKEINTRESOURCE(IDD_MERWAR), aDiag, MergeWarnmsg);
                                int unsigned count;
                                x.MergeAncestory(pszFilePath);
                               
                                  if (x.MergeState())
                                    {
                                      DialogBox(hInst, MAKEINTRESOURCE(IDD_MERGABORT), aDiag, MergeAbortmsg);
                                    }
                                  else {
                                     
                                      //merge successfull
                                      DialogBox(hInst, MAKEINTRESOURCE(IDD_MERREPORT), aDiag, MergeReportmsg);
                                      mergeLoad = mergeLoad | 1;
                                  }
                                  LPWSTR lp = const_cast<LPTSTR>(TEXT("0"));
                                  SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                  SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                  //Update count and path per normal
                                  count = x.SNPCount();
                                //  PWSTR src = const_cast<PWSTR>(SourceFilePath.c_str());//Retain original name WORKS FINE BUT IS BAD PRACTICE
                                  std::wstring ws = SourceFilePath;   // local copy
                                  PWSTR src = ws.data();              // writable buffer
                                  ScreenUpdate(hWnd, count, src, NULL, x.sex());
                                  EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                  InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                  UpdateWindow(aDiag);
                                  if (pszFilePath) CoTaskMemFree(pszFilePath);
                                  pItem->Release();
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
                CoUninitialize(); //needed to be moved here
            }
        }
        break;
        case ID_FILE_MERGE23TOMETXTFILE: {

            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                DialogBox(hInst, MAKEINTRESOURCE(IDD_MERWAR), aDiag, MergeWarnmsg);
                                int unsigned count;
                                x.Mergef23andMe(pszFilePath);

                                if (x.MergeState())
                                {
                                    DialogBox(hInst, MAKEINTRESOURCE(IDD_MERGABORT), aDiag, MergeAbortmsg);
                                }
                                else {

                                    //merge successfull
                                    DialogBox(hInst, MAKEINTRESOURCE(IDD_MERREPORT), aDiag, MergeReportmsg);
                                    mergeLoad = mergeLoad | 4;
                                }
                                LPWSTR lp = const_cast<LPTSTR>(TEXT("0"));
                                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                //Update count and path per normal
                                count = x.SNPCount();
                                PWSTR src = const_cast<PWSTR>(SourceFilePath.c_str());//Retain original name
                                ScreenUpdate(hWnd, count, src, NULL, x.sex());
                                EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                UpdateWindow(aDiag);
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
				CoUninitialize(); //needed to be moved here
            }
        }
        break;
        case ID_FILE_MERGEFTDNA: {

            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                DialogBox(hInst, MAKEINTRESOURCE(IDD_MERWAR), aDiag, MergeWarnmsg);
                                int unsigned count;
                                x.MergeFTDNA(pszFilePath);

                                if (x.MergeState())
                                {
                                    DialogBox(hInst, MAKEINTRESOURCE(IDD_MERGABORT), aDiag, MergeAbortmsg);
                                }
                                else {
                                      //merge successfull
                                      DialogBox(hInst, MAKEINTRESOURCE(IDD_MERREPORT), aDiag, MergeReportmsg);
                                      mergeLoad = mergeLoad | 2;
                                }
                                LPWSTR lp = const_cast<LPTSTR>(TEXT("0"));
                                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                //Update count and path per normal
                                count = x.SNPCount();
                                PWSTR src = const_cast<PWSTR>(SourceFilePath.c_str());//Retain original name
                                ScreenUpdate(hWnd, count, src, NULL, x.sex());
                                EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
                                UpdateWindow(aDiag);
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
				CoUninitialize(); //needed to be moved here
            }
        }
        break;
        case ID_FILE_EXPORT:
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileWrite = nullptr;
                hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                    IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileWrite));

                if (SUCCEEDED(hr))
                {
                    COMDLG_FILTERSPEC rgSpec[] = { {L"Text Files", L"*.txt"}, {L"All Files", L"*.*"} };
                    pFileWrite->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
                    pFileWrite->SetDefaultExtension(L"txt");  // Helpful addition

                    // Using NULL instead of hWnd because modal dialogs sometimes hang
                    // in WndProc. NULL works reliably, even if it appears behind.
                    // Users can Alt+Tab if needed. Reliability > theoretical correctness.
                    hr = pFileWrite->Show(NULL);

                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem = nullptr;
                        hr = pFileWrite->GetResult(&pItem);
                        if (SUCCEEDED(hr) && pItem)  // Added && pItem check
                        {
                            PWSTR pszFilePath = nullptr;  // Initialize
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            if (SUCCEEDED(hr) && pszFilePath)  // Added && pszFilePath check
                            {
                                x.AncestoryWriter(pszFilePath);
                                CoTaskMemFree(pszFilePath);
                                InvalidateRect(aDiag, NULL, FALSE);
                                UpdateWindow(aDiag);
                            }

                            pItem->Release();  // CRITICAL: Release pItem!
                        }
                    }

                    if (pFileWrite) pFileWrite->Release();
                }
                CoUninitialize();  // Always called (moved outside if)
            }
        }
        break;
        case ID_PROJEX: {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileWrite;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(::CLSID_FileSaveDialog, NULL, CLSCTX_ALL, ::IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileWrite));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"Text Files";
                    LPCWSTR b = L"All Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.txt"},
                        {b, L"*.*" },
                    };
                    //set file type options
                    hr = pFileWrite->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileWrite->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileWrite->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {//Open file for write 
                                std::fstream  fstrm;
                                {   //filestream 
                                    fstrm.open(pszFilePath, std::ios::out);
                                    if (fstrm.is_open()) {
                                        //Get Project window entry count
                                        HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                        int  lcount = 0;
                                        std::string  lbuffer = "";
                                        lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);
                                        TCHAR text[256];
										for (int x = 0, ln = 0; x < lcount; x++)//fixed now taking one entry less
                                        {
                                            ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
                                            if (ln > 0 and ln < 256) {
                                                SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
												text[ln] = TEXT('\0'); //Ensure Zero terminated
                                                //checked: the returned text is Zero terminated at tchar[ln] !
                                                CT2CA pszConvertedAnsiString(text);
                                                lbuffer = pszConvertedAnsiString;
                                                lbuffer += "\n";
                                                const char* write_it = lbuffer.c_str();
                                                fstrm.write(write_it, lbuffer.length());

                                            }
                                        }
                                        fstrm.close();
                                    }
                                }

                            }
                        }
                    }
                    pFileWrite->Release();
                }
                CoUninitialize();
            }
        }
        break;
        case ID_PATHOGENICS_CREATE: {
             DialogBox(hInst, MAKEINTRESOURCE(136), aDiag, Pathogen);
        }
        break;
        case ID_PATHOGENICS_LOAD: {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"PPI file";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.PPI"},
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);
                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;

                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                //Wide Char Safer Loading code AI's suggested
                                char lbuffer[260] = { 0 };
                                size_t charsConverted = 0;
                                char filename[260] = { 0 };
                                wcstombs_s(&charsConverted, filename, sizeof(filename), pszFilePath, _TRUNCATE);
                                /*open file*/ //2025
                                std::fstream  fstrm;
                                {
                                    //filestream 
                                    fstrm.open(pszFilePath, std::ios::in);
                                    //Check file was opened  
                                    if (fstrm.is_open()) {
                                        //reset display area
                                        HWND plst = GetDlgItem(aDiag, IDC_LIST2);
										//REMOVED Resource leaking Font creation each load!
                                        SendMessage(plst, LB_RESETCONTENT, NULL, NULL); //Clear ListBox
                                        //Read first reference lines
                                        fstrm.getline(lbuffer, 256);
                                        std::string s = "Title: ";
                                        s += lbuffer;
                                        std::wstring str2(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        //Read first reference lines
                                        fstrm.getline(lbuffer, 256);
                                        s = "Source: ";
                                        s += lbuffer;
                                        str2.resize(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);

                                        //Read first reference lines
                                        fstrm.getline(lbuffer, 256);
                                        s = "NCBI Reference: ";
                                        s += lbuffer;
                                        str2.resize(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        //init redraw
                                        s = "-- Comparison With Loaded SNP Data --";
                                        str2.resize(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        float* fp;
                                        float  sumoddsratio = 0.0;
                                        fp = &sumoddsratio;
                                        EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_EXPORTRESULTSTO, MF_BYCOMMAND | MF_ENABLED);

                                        while (fstrm.getline(lbuffer, 256))//Read next line in
                                        {
                                            int rsid=0;
                                            char riskallele='\0';
                                            float oddsratio = 0.0;
                                                                                        
                                            //Read first reference lines
                                            for (int i = 0; i < 257;) {
                                                //***Obtain RSid number***
                                                if (lbuffer[i++] == 'R' && lbuffer[i++] == 'S')
                                                {
                                                    char number[255];
                                                    int n = 0;
                                                    while ((int)(lbuffer[i]) > 47 && (int)(lbuffer[i]) < 58)
                                                    {
                                                        number[n] = lbuffer[i];
                                                        i++;
                                                        n++;
                                                    }
                                                    number[n] = '\0';
                                                    rsid = atoi(number);                                              
                                                }
                                                /*skip spaces, could be wrtten in one line but this is more readable*/
                                                while (lbuffer[i] == ' ')
                                                {
                                                    i++;
                                                }
                                                /*skip Chromosone*/
                                                while (lbuffer[i] != ' ')
                                                {
                                                    i++;
                                                }
                                                /*skip spaces, could be wrtten in one line but this is more readable*/
                                                while (lbuffer[i] == ' ')
                                                {
                                                    i++;
                                                }
                                                /*Skip Gene*/
                                                while (lbuffer[i] != ' ')
                                                {
                                                    i++;
                                                }
                                                /*skip spaces, could be wrtten in one line but this is more readable*/
                                                while (lbuffer[i] == ' ' || lbuffer[i]=='[')
                                                {
                                                    i++;
                                                }
                                                //if thechar is ivalid it will do nothing so no need to check it
                                                riskallele=lbuffer[i];
                                                i++;
                                                /*skip spaces, could be wrtten in one line but this is more readable*/
                                                while (lbuffer[i] == ' ' || lbuffer[i] == ']')
                                                {
                                                    i++;
                                                }
                                                /*Obtain Odds Ration if entered!*/
                                                {
                                                    char number[255];
                                                    int n = 0;
                                                    // getline null-terminates, so:
                                                    while (i < sizeof(lbuffer) && lbuffer[i] != '\0' && (((int)(lbuffer[i]) >= '0' && (int)(lbuffer[i]) <= '9') || lbuffer[i] == '.') && n < 254)
                                                    {
                                                        number[n] = lbuffer[i];
                                                        i++;
                                                        n++;
                                                    }
                                                    number[n] = '\0';
                                                    if (strlen(number) > 0) oddsratio = atof(number);
                                                    else oddsratio = 0.0;
                                                }
												while (lbuffer[i] != '\0'){//std::fstream::getline() stops at '\n' consuming it and replacing with NULL
                                                     i++;
                                                }
                                                break;
                                            }
                                            std::string ret_result;
                                            if (rsid > 0) {
                                                ret_result = x.PathogenicCall(rsid, riskallele, oddsratio, fp);
                                                s = lbuffer;
                                                s += "   " + ret_result;
                                                str2.resize(s.length(), L' '); // Make room for characters
                                                // Copy string to wstring.
                                                std::copy(s.begin(), s.end(), str2.begin());
                                                wcsncpy_s(global_s, str2.c_str(), 255);
                                                SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                            }
                                        }
                                        s = " ";
                                        str2.resize(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        std::ostringstream ss;
                                        ss << sumoddsratio;
                                        std::string sf(ss.str());
                                        s = "Cumulative OR (?) " + sf;
                                        str2.resize(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        std::string hash;
                                        MD5 md5;
                                        hash = md5.digestFile(filename);
                                        std::transform(hash.begin(), hash.end(), hash.begin(), ::toupper);
                                        s = "PPI File's MD5 = ";
                                        s = s + hash;
                                        str2.resize(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
										InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background 
                                        UpdateWindow(aDiag);
                                        RECT rc;
                                        GetWindowRect(plst, &rc);
                                        RedrawWindow(plst, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
                                    }
                                }
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                                EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PRINT_RESULTS, MF_BYCOMMAND | MF_ENABLED); //Enable Print Option on Sucessful load
								EnableMenuItem(GetMenu(GetParent(aDiag)), ID_CLEARRESULTS, MF_BYCOMMAND | MF_ENABLED); //Enable Clear Results option
                            }
                        }
                    }
                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; }; //another COM object release fix
				CoUninitialize(); //needed to be moved here
            }
        }
        break;
        case ID_CLEARRESULTS: {// Clear Pathogenic Results
			HWND plst = GetDlgItem(aDiag, IDC_LIST2);//Get Pathogenic List Box
            SendMessage(plst, LB_RESETCONTENT, NULL, NULL); //Clear ListBox
			EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PRINT_RESULTS, MF_BYCOMMAND | MF_DISABLED); //Disable print on clear
            EnableMenuItem(GetMenu(GetParent(aDiag)), ID_CLEARRESULTS, MF_BYCOMMAND | MF_DISABLED); //Disable clear results on clear 
            EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_EXPORTRESULTSTO, MF_BYCOMMAND | MF_DISABLED); //Disable export to export to text of clear
		}
		break;
        case ID_PATHOGENICS_EXPORTRESULTSTO: {
                HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileWrite;

                    // Create the FileOpenDialog object.
                    hr = CoCreateInstance(::CLSID_FileSaveDialog, NULL, CLSCTX_ALL, ::IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileWrite));

                    if (SUCCEEDED(hr))
                    {
                        LPCWSTR a = L"Text Files";
                        LPCWSTR b = L"All Files";
                        COMDLG_FILTERSPEC rgSpec[] =
                        {
                            {a, L"*.txt"},
                            {b, L"*.*" },
                        };
                        //set file type options
                        hr = pFileWrite->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                        // Show the Open dialog box.
                        hr = pFileWrite->Show(NULL);

                        // Get the file name from the dialog box.
                        if (SUCCEEDED(hr))
                        {
                            IShellItem* pItem;
                            hr = pFileWrite->GetResult(&pItem);
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFilePath = nullptr;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                                // Display the file name to the user.
                                if (SUCCEEDED(hr))
                                {//Open file for write 
                                    std::fstream  fstrm;
                                    {   //filestream 
                                        fstrm.open(pszFilePath, std::ios::out);
                                        if (fstrm.is_open()) {
                                            //Get Pathagenic window entry count
                                            HWND plst = GetDlgItem(aDiag, IDC_LIST2);
                                            int  lcount;
                                            bool beginFormating = false;
                                            std::string  lbuffer;
                                            lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);
                                            TCHAR text[260] = {'0'};
											for (int x = 0, ln = 0; x < lcount; x++) //fixed now taking one entry less
                                            {
                                                ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
                                                if (ln > 0 and ln < 256) {
                                                    SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
													text[ln] = TEXT('\0'); //Ensure Zero terminated
                                                    //checked: the returned text is Zero terminated at tchar[ln] !
                                                    CT2CA pszConvertedAnsiString(text);
                                                    lbuffer = pszConvertedAnsiString;
                                                    lbuffer += "\n";
                                                    const char* write_it = lbuffer.c_str();
                                                    fstrm.write(write_it, lbuffer.length());
                                                    if (lbuffer._Starts_with("NCBI")) {
                                                        beginFormating = true;
                                                        fstrm.write("\n", 1);
                                                    }
                                                }
                                            }
                                            fstrm.close();
                                        }
                                    }

                                }
                            }
                        }
                        pFileWrite->Release();
                    }
                    CoUninitialize();
                }
            }
        break;
        case ID_PRINT_RESULTS:
			//Print functions write by AI becuase I'm lazy
            PathoPrint();
		break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
        break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            }
     } 
     break; 
    case WM_SIZE:
            InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
           // UpdateWindow(aDiag);            
            break;
    case WM_CREATE:  //fixed up using DeepSeek code anaylysis
        // Create the dialog
        aDiag = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, (DLGPROC)FormDlgProc);
        if (aDiag != NULL)
        {
            // Position and show the dialog
            RECT Rect;
			if (GetWindowRect(aDiag, &Rect) != NULL) {//sanity check
               SetWindowPos(aDiag, HWND_TOP, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
               ShowWindow(aDiag, SW_SHOW);
               }

            std::string s = "0";
            USES_CONVERSION_EX;
            LPWSTR lp = A2W_EX(s.c_str(), s.length());
			if (lp != NULL) { //sanity check
                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp);
                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
            }
        }
        break;
  
    case WM_ERASEBKGND:
        return 1; // We handle background in WM_PAINT
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Fill background
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW));

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_MOVE:
        RECT rc;
        GetWindowRect(hWnd, &rc);
        RedrawWindow(hWnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
 }

void ScreenUpdate(HWND hWnd, int unsigned x, PWSTR FilePath, PWSTR build, char sx)
{
    if (x > 0)
    {//Data loaded
        std::string s = std::to_string(x);
        USES_CONVERSION_EX;
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        LPWSTR fp = A2W_EX("Female", 6);
        LPWSTR mp = A2W_EX("Male", 4);
		if (lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp); //add null lp check
        EnableWindow(GetDlgItem(aDiag, IDC_BUTTON_SEARCH), TRUE);
        EnableWindow(GetDlgItem(aDiag, IDC_EDIT_SEARCH), TRUE);
        //Set a limit on rs field
        SendMessageW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), EM_SETLIMITTEXT, 9, 0); //bug fix 3/9/21
        //Show source path
        SetWindowTextW(GetDlgItem(aDiag, IDC_SOURCE), FilePath);
        //show NCBI BUILD
        SetWindowTextW(GetDlgItem(aDiag, IDC_BUILD), build);
        if (sx == 'F' && fp !=NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_SEX), fp); //add null fp check
                else if(mp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_SEX), mp); //add null lp check
        EnableMenuItem(GetMenu(hWnd), ID_FILE_EXPORT, MF_BYCOMMAND | MF_ENABLED);
        //optimized ver 0.3 beta
        HWND plst = GetDlgItem(aDiag, IDC_LIST2);
        SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR pathy listbox
        EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_LOAD, MF_BYCOMMAND | MF_ENABLED);
        //*** Merge code menu ***
        if (loadedFiletype == 1) {
            if (mergeLoad & 4) EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGE23TOMETXTFILE, MF_BYCOMMAND | MF_DISABLED);
            else EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGE23TOMETXTFILE, MF_BYCOMMAND | MF_ENABLED);
            if (mergeLoad & 2) EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEFTDNA, MF_BYCOMMAND | MF_DISABLED);
            else EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEFTDNA, MF_BYCOMMAND | MF_ENABLED);
         } else if (loadedFiletype == 2) {
             if (mergeLoad & 4) EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGE23TOMETXTFILE, MF_BYCOMMAND | MF_DISABLED);
             else EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGE23TOMETXTFILE, MF_BYCOMMAND | MF_ENABLED);
             if(mergeLoad & 1) EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEANCESTORYDNATXTFILE, MF_BYCOMMAND | MF_DISABLED);
             else EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEANCESTORYDNATXTFILE, MF_BYCOMMAND | MF_ENABLED);
         } else if (loadedFiletype == 3) {
             if (mergeLoad & 1) EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEANCESTORYDNATXTFILE, MF_BYCOMMAND | MF_DISABLED);
             else EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEANCESTORYDNATXTFILE, MF_BYCOMMAND | MF_ENABLED);
             if (mergeLoad & 2) EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEFTDNA, MF_BYCOMMAND | MF_DISABLED);
             else EnableMenuItem(GetMenu(hWnd), ID_FILE_MERGEFTDNA, MF_BYCOMMAND | MF_ENABLED);
           }
        } else {
                 std::string s = "0";
                 mergeLoad = mergeTotal = 0;
                 USES_CONVERSION_EX;
                 LPWSTR lp = A2W_EX(s.c_str(), s.length());
                 if(lp != NULL) SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp);
                 EnableWindow(GetDlgItem(aDiag, IDC_BUTTON_SEARCH), FALSE);
                 EnableWindow(GetDlgItem(aDiag, IDC_EDIT_SEARCH),   FALSE);
                 //Set a limit on rs field
                 SendMessageW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), EM_SETLIMITTEXT, 9, 0); //bug fux 3/9/21
                 //Show source path
                 SetWindowTextW(GetDlgItem(aDiag, IDC_SOURCE), NULL); 
                 SetWindowTextW(GetDlgItem(aDiag, IDC_SEX), NULL);
                 EnableMenuItem(GetMenu(hWnd), ID_FILE_EXPORT, MF_BYCOMMAND | MF_GRAYED);  //ensure export is deactivated
                 EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_LOAD, MF_BYCOMMAND | MF_GRAYED);
    }
    //Covert RS number for display
    std::string s = "";
    USES_CONVERSION_EX;
    LPWSTR lp = A2W_EX(s.c_str(), s.length());
	if (lp != NULL) {//sanity check
        SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT1), lp);
        SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_CHRNUM), lp);
        SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_POSIT), lp);
        SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_ALLES1), lp);
        SetWindowTextW(GetDlgItem(aDiag, IDC_AllELE2), lp);
    }
    EnableWindow(GetDlgItem(aDiag, IDC_COPYCLIP), FALSE);
    EnableWindow(GetDlgItem(aDiag, IDC_COPYPROJ), FALSE);
    InvalidateRect(aDiag, NULL, FALSE); // FALSE = don't erase background
    UpdateWindow(aDiag);
}
/*ChatGPT generated printing function I just could deal with 
Windows UNICODE BS when I could just use plain text printing in ASCII before.
I requsted the use of A4 standard with Letter for US hence the function below*/
// Helper: returns true if user's locale country is "US"
static bool IsLocaleUS()
{
    wchar_t buf[8] = { 0 };
    // LOCALE_USER_DEFAULT works on older systems; LOCALE_SISO3166CTRYNAME returns "US", "GB", etc.
    if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, buf, (int)_countof(buf)) > 0) {
        return (_wcsicmp(buf, L"US") == 0);
    }
    return false; // default to A4 if unsure
}

// Helper: safe GetTextExtent wrapper
static int GetTextWidth(HDC hdc, const std::wstring& s)
{
    if (s.empty()) return 0;
    SIZE sz = { 0,0 };
    // GetTextExtentPoint32W returns TRUE on success
    GetTextExtentPoint32W(hdc, s.c_str(), (int)s.size(), &sz);
    return sz.cx;
}

// Main printing function (updated)
bool PrintPlainText(const std::wstring& text)
{
    WCHAR printerName[256];
    DWORD size = ARRAYSIZE(printerName);

    if (!GetDefaultPrinterW(printerName, &size))
        return false;

    // Create a printer DC for the default printer
    HDC hdc = CreateDCW(NULL, printerName, NULL, NULL);
    if (!hdc)
        return false;

    // --- Modify printer DEVMODE to set paper size (A4 vs Letter) ---
    HANDLE hPrinter = NULL;
    if (OpenPrinterW(printerName, &hPrinter, NULL)) {
        // Get size of DEVMODE
        LONG dmSize = DocumentPropertiesW(NULL, hPrinter, printerName, NULL, NULL, 0);
        if (dmSize > 0) {
            // allocate DEVMODE
            std::unique_ptr<DEVMODEW, decltype(&GlobalFree)> pDevMode(
                (DEVMODEW*)GlobalAlloc(GPTR, dmSize), &GlobalFree);
            if (pDevMode) {
                // Fill current devmode
                LONG res = DocumentPropertiesW(NULL, hPrinter, printerName, pDevMode.get(), NULL, DM_OUT_BUFFER);
                if (res == IDOK) {
                    // choose paper size
                    if (IsLocaleUS()) {
                        pDevMode->dmPaperSize = DMPAPER_LETTER;   // US Letter
                    }
                    else {
                        pDevMode->dmPaperSize = DMPAPER_A4;       // A4 elsewhere
                    }
                    pDevMode->dmFields |= DM_PAPERSIZE;

                    // Apply the DEVMODE to the HDC
                    // ResetDC returns nonzero on success
                    ResetDCW(hdc, pDevMode.get());
                    // Note: we do not free pDevMode explicitly (unique_ptr will)
                }
            }
        }
        ClosePrinter(hPrinter);
    }

    // --- Start the document ---
    DOCINFOW di = { 0 };
    di.cbSize = sizeof(DOCINFOW);
    di.lpszDocName = L"Plain Text Print";

    if (StartDocW(hdc, &di) <= 0) {
        DeleteDC(hdc);
        return false;
    }

    if (StartPage(hdc) <= 0) {
        EndDoc(hdc);
        DeleteDC(hdc);
        return false;
    }

    // Get DPI and page metrics
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
    int pageWidth = GetDeviceCaps(hdc, HORZRES);
    int pageHeight = GetDeviceCaps(hdc, VERTRES);

    // Use a readable font size (12 pt). Adjust if you prefer 11 or 14.
    const int pointSize = 12;
    int fontHeight = -MulDiv(pointSize, dpiY, 72); // negative for character height

    LOGFONTW lf = { 0 };
    lf.lfHeight = fontHeight;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = CLEARTYPE_QUALITY; // readable
    lf.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    wcscpy_s(lf.lfFaceName, L"Consolas"); // monospaced; fallback if not installed

    HFONT hFont = CreateFontIndirectW(&lf);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    // 1 inch margins
    int marginX = dpiX * 1;
    int marginY = dpiY * 1;

    // printable area
    int left = marginX;
    int right = pageWidth - marginX;
    int top = marginY;
    int bottom = pageHeight - marginY;
    int maxWidth = right - left;
    int maxY = bottom;

    // Get line height
    TEXTMETRICW tm = { 0 };
    GetTextMetricsW(hdc, &tm);
    int lineHeight = tm.tmHeight + tm.tmExternalLeading;
    if (lineHeight <= 0) lineHeight = MulDiv(pointSize, dpiY, 72) + 2;

    // Helper to output a single line and paginate when necessary
    auto emitLine = [&](const std::wstring& ln, int& curY) {
        TextOutW(hdc, left, curY, ln.c_str(), (int)ln.size());
        curY += lineHeight;
        if (curY + lineHeight > maxY) {
            // new page
            EndPage(hdc);
            StartPage(hdc);
            SelectObject(hdc, hFont); // re-select font on new page
            curY = top;
        }
        };

    // Now do robust wrapping:
    // We process input text by paragraphs (split on '\n'), then wrap tokens by spaces.
    int curY = top;
    size_t pos = 0;
    const size_t N = text.size();

    while (pos < N) {
        // extract one paragraph (up to next '\n')
        size_t nl = text.find(L'\n', pos);
        std::wstring para;
        if (nl == std::wstring::npos) {
            para = text.substr(pos);
            pos = N;
        }
        else {
            para = text.substr(pos, nl - pos);
            pos = nl + 1; // skip newline
        }

        // Trim possible '\r' (windows CRLF)
        // Not strictly necessary because we split on '\n', but be safe
        if (!para.empty() && para.back() == L'\r') para.pop_back();

        // If empty paragraph, emit a blank line
        if (para.empty()) {
            emitLine(L"", curY);
            continue;
        }

        // Tokenize by spaces/tabs while preserving tokens (URLs have no spaces)
        size_t i = 0;
        std::wstring currentLine;
        while (i < para.size()) {
            // skip leading spaces - but if we want to preserve single leading spaces, adjust
            while (i < para.size() && (para[i] == L' ' || para[i] == L'\t')) ++i;
            if (i >= para.size()) break;

            // find next whitespace to get token
            size_t j = i;
            while (j < para.size() && para[j] != L' ' && para[j] != L'\t') ++j;

            std::wstring token = para.substr(i, j - i);
            i = j;

            // Measure if token fits on currentLine
            std::wstring candidate = currentLine.empty() ? token : (currentLine + L" " + token);
            int candWidth = GetTextWidth(hdc, candidate);

            if (candWidth <= maxWidth) {
                // it fits
                currentLine = std::move(candidate);
            }
            else {
                // it doesn't fit
                if (currentLine.empty()) {
                    // current line empty but token too long => force-break token
                    size_t start = 0;
                    while (start < token.size()) {
                        // find the max substring length that fits
                        size_t end = token.size();
                        size_t lo = start, hi = end;
                        size_t best = start;
                        // binary search for largest fit to reduce GetTextExtent calls
                        while (lo < hi) {
                            size_t mid = (lo + hi + 1) / 2;
                            int w = GetTextWidth(hdc, token.substr(start, mid - start));
                            if (w <= maxWidth) {
                                best = mid;
                                lo = mid;
                            }
                            else {
                                hi = mid - 1;
                            }
                        }
                        if (best == start) {
                            // even a single char doesn't fit (very narrow maxWidth?). Force at least 1 char
                            best = start + 1;
                        }
                        std::wstring piece = token.substr(start, best - start);
                        emitLine(piece, curY);
                        start = best;
                    }
                    // after breaking token, currentLine stays empty
                }
                else {
                    // flush currentLine
                    emitLine(currentLine, curY);
                    // start new currentLine with token (it may still be too long; next loop iteration will handle force-break)
                    currentLine = L"";
                    // try placing token again by resetting i back to token start
                    // but simpler: set currentLine = token and let next iteration detect overflow
                    if (GetTextWidth(hdc, token) <= maxWidth) {
                        currentLine = token;
                    }
                    else {
                        // force-break token now (same logic as above)
                        size_t start = 0;
                        while (start < token.size()) {
                            size_t end = token.size();
                            size_t lo = start, hi = end;
                            size_t best = start;
                            while (lo < hi) {
                                size_t mid = (lo + hi + 1) / 2;
                                int w = GetTextWidth(hdc, token.substr(start, mid - start));
                                if (w <= maxWidth) {
                                    best = mid;
                                    lo = mid;
                                }
                                else {
                                    hi = mid - 1;
                                }
                            }
                            if (best == start) best = start + 1;
                            std::wstring piece = token.substr(start, best - start);
                            emitLine(piece, curY);
                            start = best;
                        }
                        currentLine.clear();
                    }
                }
            }
        } // end tokens loop

        // flush any remaining current line
        if (!currentLine.empty()) {
            emitLine(currentLine, curY);
        }

        // after paragraph, advance a blank line (preserve paragraph spacing)
        // This is optional; if you don't want extra blank line, remove the next line
        // emitLine(L"", curY);
    }

    // End of document cleanup
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    EndPage(hdc);
    EndDoc(hdc);
    DeleteDC(hdc);

    return true;
}



void PathoPrint()
{
    HWND plst = GetDlgItem(aDiag, IDC_LIST2);

    if (!plst) {
        MessageBox(NULL, L"List box not found", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    int lcount = (int)SendMessage(plst, LB_GETCOUNT, 0, 0);
    if (lcount == LB_ERR) {
        MessageBox(NULL, L"Failed to get list box count", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (lcount < 1) {
        MessageBox(NULL, L"List box is empty", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // --- Build everything as WIDE TEXT ---
    std::wstring text;
    text += L"========================================\n";
    text += L"        PATHOGENIC SNP REPORT\n";
    text += L"========================================\n\n";

    // Wide date function needed
    text += L"Date: ";
    text += GetCurrentDateString();   // ← implement this returning std::wstring
    text += L"\n";

    text += L"Total Variants: " + std::to_wstring(lcount) + L"\n\n";
    text += L"----------------------------------------\n\n";

    // Collect listbox strings (wide)
    for (int x = 0; x < lcount; x++)
    {
        int len = (int)SendMessage(plst, LB_GETTEXTLEN, x, 0);
        if (len <= 0 || len > 2048) continue;

        std::wstring buffer(len, L'\0');

        if (SendMessage(plst, LB_GETTEXT, x, (LPARAM)buffer.data()) != LB_ERR)
        {
            // Add "n. item text"
            text += std::to_wstring(x + 1);
            text += L". ";
            text += buffer;
            text += L"\n";
        }
    }

    text += L"\n----------------------------------------\n";
    text += L"End of Report\n";

    // Print using your wide-printing function
    if (!PrintPlainText(text)) {
        MessageBox(NULL, L"Printing failed", L"Error", MB_OK | MB_ICONERROR);
    }
}

std::wstring GetCurrentDateString(void)
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
	std::string timeStr;
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
	timeStr = buf;
    int len = MultiByteToWideChar(CP_UTF8, 0, timeStr.c_str(), -1, NULL, 0);
    std::wstring result(len - 1, L'\0');  // minus null terminator
    MultiByteToWideChar(CP_UTF8, 0, timeStr.c_str(), -1, &result[0], len);
    return result;
}
// Message handler for new project box.
INT_PTR CALLBACK ProjectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);

    switch (message)
    {
    case WM_INITDIALOG:
         return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            TCHAR buffer[256] = { 0 };
            std::wstring temp;
            if (GetWindowText(GetDlgItem(hDlg, IDC_ProjectNm), buffer, _countof(buffer))) {
               temp = buffer;
               for(int x=0;x<temp.length();)
               {   //check for invalid characters for a directory name i know alph numeric is strict but i'm to lasy to check all allowed vs invalid
                   if (!isalnum(buffer[x]))  return (INT_PTR)FALSE;
                   x++;
               }
       
               std::wstring directory;
               for (const auto& entry : fs::directory_iterator(myPath)){
                   directory = entry.path();
                   //check for directory already existing
                   if(directory == temp) return (INT_PTR)FALSE;;
               }
                currentProject = myPath + L"\\" + temp;
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            return (INT_PTR)FALSE;
        }
        else  if(LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK Deletemsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
     UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);
    HANDLE hicon = LoadImageW(hInst, MAKEINTRESOURCEW(IDI_PARSESNP), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
    switch (message)
    {
    case WM_INITDIALOG:
        //Set Icon
        //Moved here to fix crash!
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
        //Set Icon
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else
            if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)FALSE;
            }
        break;

    case WM_DESTROY: //Thanks DeepSeek for this fix!
        // Clean up the icon
        HICON hicon = (HICON)GetProp(hDlg, L"DIALOG_ICON");
        if (hicon) {
            DestroyIcon(hicon);
            RemoveProp(hDlg, L"DIALOG_ICON");
        }
        break;

    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK MergeAbortmsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);
    //Set Icon
    HANDLE hicon = LoadImageW(hInst, MAKEINTRESOURCEW(IDI_PARSESNP), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
    //Set Icon
    switch (message)
    {
    case WM_INITDIALOG: {
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
        std::string s = "The Merge operation was aborted due to too many differences between existing SNP alleles.\nNo changes have been made.";
		USES_CONVERSION_EX; //Added Null check
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_STATIC), lp);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;

    case WM_DESTROY: //Thanks DeepSeek for this fix!
        // Clean up the icon
        HICON hicon = (HICON)GetProp(hDlg, L"DIALOG_ICON");
        if (hicon) {
            DestroyIcon(hicon);
            RemoveProp(hDlg, L"DIALOG_ICON");
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK MergeReportmsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);
    //Set Icon
    HANDLE hicon = LoadImageW(hInst, MAKEINTRESOURCEW(IDI_PARSESNP), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
    //Set Icon
    switch (message)
    {
    case WM_INITDIALOG: {
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
        std::string s = "Merge Completed Successfully!\n\nThe Merge results ONLY exist loaded in memory!\nYou should save the data by exporting as an ancestory file or saving as a project which will create an ancestory file in the project folder.";
		USES_CONVERSION_EX;//Added Null check
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_STATIC), lp);
        std::ostringstream ss;
        ss << x.MergeProcessed();
        std::string sf(ss.str());
        lp = A2W_EX(sf.c_str(), sf.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_EDITTOTAL), lp);
        mergeTotal += x.merged();
        std::ostringstream sx;
        sx << x.merged();
        std::string se(sx.str());
        lp = A2W_EX(se.c_str(), se.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_MERGED), lp);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;

    case WM_DESTROY: //Thanks DeepSeek for this fix!
        // Clean up the icon
        HICON hicon = (HICON)GetProp(hDlg, L"DIALOG_ICON");
        if (hicon) {
            DestroyIcon(hicon);
            RemoveProp(hDlg, L"DIALOG_ICON");
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK MergeWarnmsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);
    //Set Icon
    HANDLE hicon = LoadImageW(hInst, MAKEINTRESOURCEW(IDI_PARSESNP), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
    //Set Icon
    switch (message)
    {
    case WM_INITDIALOG: {
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
        std::string s = "**You Have Initiated a Merge!**\n\nNote: There is a lot of data to compare and the program may become unresposive for several minutes.\nAlso be aware that the function is indended to be used to merge two files of the same person, if two different subject's files are merged the code will detect the many differences and abort the merge.\n\nClick OK to proceed!";
		USES_CONVERSION_EX;//Added Null check
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_STATIC), lp);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;

    case WM_DESTROY: //Thanks DeepSeek for this fix!
        // Clean up the icon
        HICON hicon = (HICON)GetProp(hDlg, L"DIALOG_ICON");
        if (hicon) {
            DestroyIcon(hicon);
            RemoveProp(hDlg, L"DIALOG_ICON");
        }
        break;
    }
    return (INT_PTR)FALSE;
}
// Message handler for about box.

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);
  
    switch (message)
    {
    case WM_INITDIALOG: {
        // Center the dialog on its parent window
        HWND hwndParent = GetParent(hDlg);
        RECT rcDialog, rcParent;

        // Get dialog size
        GetWindowRect(hDlg, &rcDialog);
        int dlgWidth = rcDialog.right - rcDialog.left;
        int dlgHeight = rcDialog.bottom - rcDialog.top;

        // Get parent window rect (or screen if no parent)
        if (hwndParent) {
            GetWindowRect(hwndParent, &rcParent);
        }
        else {
            // If no parent, center on screen
            rcParent.left = 0;
            rcParent.top = 0;
            rcParent.right = GetSystemMetrics(SM_CXSCREEN);
            rcParent.bottom = GetSystemMetrics(SM_CYSCREEN);
        }

        // Calculate centered position
        int parentWidth = rcParent.right - rcParent.left;
        int parentHeight = rcParent.bottom - rcParent.top;
        int xx = rcParent.left + (parentWidth - dlgWidth) / 2;
        int yy = rcParent.top + (parentHeight - dlgHeight) / 2;
        ShowWindow(hDlg, SW_SHOW);
        // Move the dialog
        SetWindowPos(hDlg, NULL, xx, yy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        std::string s = "ParseSNP Version: ";
        s += x.PVer();
        //Updated Icon code to prevent resource leaks
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PARSESNP));
        if (hIcon) {
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SetProp(hDlg, L"DIALOG_ICON", hIcon);
        }
        //Updated Icon code to prevent resource leaks
		USES_CONVERSION_EX;//Added Null check
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_STATICVER), lp);
        s = x.PAbout();
        lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_ABTTXT), lp);
        const HFONT font = (HFONT)SendDlgItemMessage(hDlg, IDC_STATIC_LNK, WM_GETFONT, 0, 0);
        LOGFONT fontAttributes = { 0 };
        ::GetObject(font, sizeof(fontAttributes), &fontAttributes);
        fontAttributes.lfUnderline = TRUE;
        HFONT x = CreateFontIndirect(&fontAttributes);
        if (x != NULL) {
            SendDlgItemMessage(hDlg, IDC_STATIC_LNK, WM_SETFONT, (WPARAM)x, 1);
            SetProp(hDlg, L"UNDERLINE_FONT", x);
        }
        return (INT_PTR)TRUE;
    }
  
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDOK:
        case IDCANCEL: {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
         }
        case IDC_STATIC_LNK: {
            if ((INT_PTR)ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/BritRobin/ParseSNP"), NULL, NULL, SW_SHOWNORMAL) <= 32) return (INT_PTR)FALSE;/* error */
            else return (INT_PTR)TRUE;
         }
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        // Make the hyperlink blue
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_STATIC_LNK))
        {
            SetTextColor((HDC)wParam, RGB(0, 0, 255));
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (INT_PTR)GetSysColorBrush(COLOR_BTNFACE);
        }
        break;
    }
    case WM_DESTROY:
    {
        // Clean up the icon
        HICON hIcon = (HICON)RemoveProp(hDlg, L"DIALOG_ICON");
        if (hIcon) {
            DestroyIcon(hIcon);
        }

        // Clean up the created font
        HFONT hUnderlineFont = (HFONT)RemoveProp(hDlg, L"UNDERLINE_FONT");
        if (hUnderlineFont) {
            DeleteObject(hUnderlineFont);
        }
        return TRUE;
    }

    }
    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK SearchFailmsg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);

    switch (message)
    {
    case WM_INITDIALOG: {
        std::string s = "RSID: ";
        std::string rs_string = "0";
		std::string e = " was not found.";

        //Updated Icon code to prevent resource leaks
        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PARSESNP));
        if (hIcon) {
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SetProp(hDlg, L"DIALOG_ICON", hIcon);
        }

        //Updated Icon code to prevent resource leaks
        USES_CONVERSION_EX;//Added Null check
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_RSIDNF1), lp);
        rs_string = std::to_string(rs_number);
        lp = A2W_EX(rs_string.c_str(), rs_string.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_RSIDNF), lp);
        lp = A2W_EX(e.c_str(), e.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_RSIDNF3), lp);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId) {
        case IDOK:
        case IDCANCEL:
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
       }
        break;
    }
    case WM_DESTROY:
    {
        // Clean up the icon
        HICON hIcon = (HICON)RemoveProp(hDlg, L"DIALOG_ICON");
        if (hIcon) {
            DestroyIcon(hIcon);
        }

        // Clean up the created font
        HFONT hUnderlineFont = (HFONT)RemoveProp(hDlg, L"UNDERLINE_FONT");
        if (hUnderlineFont) {
            DeleteObject(hUnderlineFont);
        }
        return TRUE;
    }


    }
    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK Pathogen(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);

    switch (message)
    {
    case WM_INITDIALOG: {
        std::string s;
        LPCWSTR lp;
        //Updated Icon code to prevent resource leaks
        HICON hicon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PARSESNP));
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
        //Updated Icon code to prevent resource leaks
        USES_CONVERSION_EX;//Added Null check
        s = "All fields not marked with an asterisks are mandatory.\nNon-mandatory fields not entered are replaced with dashes.\nYou should reference the source URL of the data you enter.\nYou can delete an entry from the list by double clicking it.\nWhen a .PPI file is created a .MD5 file will be created containg its MD5 hash.  A .PPI file created from valid data and run against an acurate sequence should still be seen as indicative not diagnostic!\n\n** If you have genetic medical worries you should speak with a Dr or Genetic counselor! **";
        lp = A2W_EX(s.c_str(), s.length());
        if (lp != NULL) SetWindowTextW(GetDlgItem(hDlg, IDC_INFOP), lp);
        { //Ver 3.0 Beta - re-written so it make sense! FIXED IN 4.1
            LOGFONT lf;
            LOGFONT fontAttributes = { 0 };
            const HFONT font = (HFONT)SendDlgItemMessage(hDlg, IDC_LIST1, WM_GETFONT, 0, 0);
            ::GetObject(font, sizeof(fontAttributes), &fontAttributes);
            memcpy_s(&lf, sizeof(fontAttributes), &fontAttributes, sizeof(lf));
            lf.lfHeight = 16;                      // request a 16 pixel-height font
            _tcsncpy_s(lf.lfFaceName, LF_FACESIZE, _T("Courier New"), 11); // request a face name "Courier New"
            HFONT x = CreateFontIndirect(&lf);
            if (x != NULL) {
                SendDlgItemMessage(hDlg, IDC_LIST1, WM_SETFONT, (WPARAM)x, 1); //Set new font if valid
                SetProp(hDlg, L"PATHOGEN_FONT", x); // Store for cleanup
            } //Ver 3.0 Beta - re-written so it make sense! FIXED IN 4.1
            return (INT_PTR)TRUE;
        }
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDC_SAVE: {

            TCHAR buffer[260] = { 0 };
            std::string s_study, s_URL, s_ncbiref;

            if (GetWindowText(GetDlgItem(hDlg, IDC_STUDY), buffer, _countof(buffer)))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_study = pszConvertedAnsiString;
            }
            else  break; //mandatory field

            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT5), buffer, _countof(buffer)))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_URL += pszConvertedAnsiString;
            }
            else  break; //mandatory field

            if (GetWindowText(GetDlgItem(hDlg, IDC_NCBIref), buffer, _countof(buffer)))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_ncbiref += pszConvertedAnsiString;
            }//optional
            if (s_ncbiref.length() == 0) s_ncbiref = "--";

            //name and write file
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileWrite;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(::CLSID_FileSaveDialog, NULL, CLSCTX_ALL, ::IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileWrite));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"PPI Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.PPI"},
                    };
                    //set file type options
                    hr = pFileWrite->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileWrite->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileWrite->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            WCHAR filename[260];
                            WCHAR ext[5];
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                ext[0] = '.';
                                ext[1] = 'P';
                                ext[2] = 'P';
                                ext[3] = 'I';
                                ext[4] = '\0';
                                StrCpyW(filename, pszFilePath);//we know 
                                wcscat_s(filename, ext);
                                {//Write the file
                                    //Open file for write 
                                    std::fstream  fstrm;
                                    {   //filestream 
                                        //fstrm.open(pszFilePath, std::ios::out);
                                        fstrm.open(filename, std::ios::out);
                                        if (fstrm.is_open()) {
                                            HWND plst = GetDlgItem(hDlg, IDC_LIST1);
                                            int  lcount;
                                            std::string  lbuffer;
                                            //Write Overview
                                            s_study += "\n";
                                            const char* write_it = s_study.c_str();
                                            fstrm.write(write_it, s_study.length());
                                            //Write the URL to the study the file is based on
                                            s_URL += "\n";
                                            write_it = s_URL.c_str();
                                            fstrm.write(write_it, s_URL.length());
                                            //NCBI Ref model major revision
                                            s_ncbiref += "\n";
                                            write_it = s_ncbiref.c_str();
                                            fstrm.write(write_it, s_ncbiref.length());
                                            lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);
                                            TCHAR text[256];
											for (int x = 0, ln = 0; x < lcount; x++)//fix one less read
                                            {
                                                ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
												if (ln > 0 and ln < 256) {
                                                    SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
                                                    text[ln] = TEXT('\0'); //Ensure null termination
                                                    //checked: the returned text is Zero terminated at tchar[ln] !
                                                    CT2CA pszConvertedAnsiString(text);
                                                    lbuffer = pszConvertedAnsiString;
                                                    lbuffer += "\n";
                                                    const char* write_it = lbuffer.c_str();
                                                    fstrm.write(write_it, lbuffer.length());
                                                }
                                            }
                                            fstrm.close();
                                            //Open file for write 
                                            std::fstream  fstrmd5;
                                            if (!fstrmd5.bad())
                                            {   //filestream 
                                                MD5 md5;
                                                ext[0] = '.';
                                                ext[1] = 'M';
                                                ext[2] = 'D';
                                                ext[3] = '5';
                                                ext[4] = '\0';
                                                std::string hash;
                                                size_t dest;
                                                size_t srce;
                                                size_t charsConverted = 0;
                                                srce = wcslen(filename);
                                                dest = srce + 1;
                                                char file[260];
                                                wcstombs_s(&charsConverted, file, dest, (wchar_t const*)filename, srce);
                                                hash = md5.digestFile(file);
                                                std::transform(hash.begin(), hash.end(), hash.begin(), ::toupper);
                                                StrCpyW(filename, pszFilePath); //we know 
                                                wcscat_s(filename, ext);
                                                fstrmd5.open(filename, std::ios::out);
                                                if (fstrmd5.is_open()) {
                                                    fstrmd5.write(hash.c_str(), hash.length());
                                                    fstrmd5.close();
                                                }
                                            }
                                        }
                                    }
                                }
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                    pFileWrite->Release();
                }
                CoUninitialize();
            }
            break;
        }
        case IDC_SAVE_DRAFT: {
            TCHAR buffer[260] = { 0 };
            std::string s_study, s_URL, s_ncbiref;

            if (GetWindowText(GetDlgItem(hDlg, IDC_STUDY), buffer, _countof(buffer)))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_study = pszConvertedAnsiString;
            }

            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT5), buffer, _countof(buffer)))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_URL += pszConvertedAnsiString;
            }

            if (GetWindowText(GetDlgItem(hDlg, IDC_NCBIref), buffer, _countof(buffer)))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_ncbiref += pszConvertedAnsiString;
            }
            if (s_ncbiref.length() == 0) s_ncbiref = "--";

            //name and write file
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileWrite = NULL;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(::CLSID_FileSaveDialog, NULL, CLSCTX_ALL, ::IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileWrite));

                if (SUCCEEDED(hr))
                {

                    LPCWSTR a = L"DRFT Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.DRFT"},
                    };
                    //set file type options
                    pFileWrite->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

                    // Show the Open dialog box.
                    hr = pFileWrite->Show(NULL);

                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileWrite->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;
                            WCHAR ext[6];
                            WCHAR filename[260];

                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                ext[0] = '.';
                                ext[1] = 'D';
                                ext[2] = 'R';
                                ext[3] = 'F';
                                ext[4] = 'T';
                                ext[5] = NULL;
                                StrCpyW(filename, pszFilePath);//we know 
                                wcscat_s(filename, ext);
                                {//Write the file
                                    //Open file for write 
                                    std::fstream  fstrm;
                                    {   //filestream 
                                        //fstrm.open(pszFilePath, std::ios::out);
                                        fstrm.open(filename, std::ios::out);
                                        if (fstrm.is_open()) {
                                            HWND plst = GetDlgItem(hDlg, IDC_LIST1);
                                            int  lcount;
                                            std::string  lbuffer;
                                            //Write Overview
                                            s_study += "\n";
                                            const char* write_it = s_study.c_str();
                                            fstrm.write(write_it, s_study.length());
                                            //Write the URL to the study the file is based on
                                            s_URL += "\n";
                                            write_it = s_URL.c_str();
                                            fstrm.write(write_it, s_URL.length());
                                            //NCBI Ref model major revision
                                            s_ncbiref += "\n";
                                            write_it = s_ncbiref.c_str();
                                            fstrm.write(write_it, s_ncbiref.length());
                                            lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);
                                            TCHAR text[256];
											for (int x = 0, ln = 0; x < lcount; x++)//fix one less read
                                            {
                                                ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
                                                if (ln > 0 and ln < 256) {
                                                    SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
                                                    text[ln] = TEXT('\0'); //Ensure null termination
                                                    //checked: the returned text is Zero terminated at tchar[ln] !
                                                    CT2CA pszConvertedAnsiString(text);
                                                    lbuffer = pszConvertedAnsiString;
                                                    lbuffer += "\n";
                                                    const char* write_it = lbuffer.c_str();
                                                    fstrm.write(write_it, lbuffer.length());
                                                }
                                            }
                                            fstrm.close();
                                        }
                                    }
                                }
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }
                    pFileWrite->Release();
                }
                CoUninitialize();
            }
            break;
        }
        case IDC_LOAD_DRAFT: {
            HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr))
                {
                    LPCWSTR a = L"DRFT Files";
                    COMDLG_FILTERSPEC rgSpec[] =
                    {
                        {a, L"*.DRFT"},
                    };
                    //set file type options
                    hr = pFileOpen->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
                    // Show the Open dialog box.
                    hr = pFileOpen->Show(NULL);
                    // Get the file name from the dialog box.
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);
                        if (SUCCEEDED(hr))
                        {
                            PWSTR pszFilePath = nullptr;

                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                char lbuffer[260];
                                /*open file*/
                                std::fstream  fstrm;
                                {   //filestream 
                                    fstrm.open(pszFilePath, std::ios::in);
                                    //Check file was opened  
                                    if (fstrm.is_open()) {
                                        //reset display area
                                        HWND plst = GetDlgItem(hDlg, IDC_LIST1);
                                        SendMessage(plst, LB_RESETCONTENT, NULL, NULL); //Clear ListBox
                                        //Read first reference lines
                                        fstrm.getline(lbuffer, 256);
                                        std::string s;
                                        s = lbuffer;
                                        std::wstring str2(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SetWindowText(GetDlgItem(hDlg, IDC_STUDY), global_s);

                                        //Read first reference lines
                                        fstrm.getline(lbuffer, 256);
                                        s = lbuffer;
                                        str2.resize(s.length(), L' '); // Make room for characters                  // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SetWindowText(GetDlgItem(hDlg, IDC_EDIT5), global_s);

                                        //Read first reference lines
                                        fstrm.getline(lbuffer, 256);
                                        s = lbuffer;
                                        str2.resize(s.length(), L' '); // Make room for characters                                            // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SetWindowText(GetDlgItem(hDlg, IDC_NCBIref), global_s);
                                        while (fstrm.getline(lbuffer, 256))
                                        {
                                            s = lbuffer;
                                            if (s.length() > 5) {
                                                str2.resize(s.length(), L' '); // Make room for characters
                                                // Copy string to wstring.
                                                std::copy(s.begin(), s.end(), str2.begin());
                                                wcsncpy_s(global_s, str2.c_str(), 255);
                                                SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                            }
                                        }
                                    }
                                }
                                if (pszFilePath) CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                    }

                }
				if (pFileOpen) { pFileOpen->Release(); pFileOpen = nullptr; };//Release
                CoUninitialize(); //needed to be moved here
            }
        }
                           break;
        case IDCANCEL://fall tho to exit I'm sure this style is fround upon
        case IDC_EXITP: {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        case IDC_ENTER: {//The main code, 
            TCHAR buffer[260] = { 0 };
            std::string s_rsid, s_chr, s_gene, s_riskallele, s_oddsratio;
            //get rsid number
            if (GetWindowText(GetDlgItem(hDlg, IDC_RSIDP), buffer, 15)) //at 260 no danger of buffer overflow from UNICODE etc 
            {
                std::string s;
                //the dialog only allows numbers to be entered
                CT2CA pszConvertedAnsiString(buffer);
                s = pszConvertedAnsiString;
                s_rsid = "RS" + s;

            }
            else  break; //mandatory field

            //Get chromosone number
            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT_CHRNUM), buffer, 15))//at 260 no danger of buffer overflow from UNICODE etc
            {
                std::string s;
                CT2CA pszConvertedAnsiString(buffer);
                s = pszConvertedAnsiString;
				//Per Deepseek add check for empty string and made the valid numeric range more obvoius to read >=1 && <=22 rather than >0 && <23 which is the same logic but harder to parse visually
                if (s != "" && ((strtod(s.data(), NULL) >= 1 && strtod(s.data(), NULL) <= 22) || s == "X" || s == "Y" || s == "MT"))
                {
                    s_chr = s;
                }
                else break; //mandatory field
            }
            //Get Gene optional
            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT2), buffer, 15))//at 260 no danger of buffer overflow from UNICODE etc
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_gene = pszConvertedAnsiString;
            }

            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT_ALLES1), buffer, 15))//at 260 no danger of buffer overflow from UNICODE etc
            {
                std::string s;
                CT2CA pszConvertedAnsiString(buffer);
                s = pszConvertedAnsiString;
                if (s != "A" && s != "C" && s != "G" && s != "T") break; //Mandatory!!
                s_riskallele = " [" + s + "] ";
            }

            //Get Odds Ratio optional but should be inclued if available
            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT3), buffer, 15))//at 260 no danger of buffer overflow from UNICODE etc
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_oddsratio = pszConvertedAnsiString;
            }
            //NO Missing Fields for file read!
            if (s_gene.length() == 0) s_gene = "----";
            if (s_oddsratio.length() == 0) s_oddsratio = "-.--";

            //format for display
            if (s_rsid.length() < 10) {
                int y = 10 - s_rsid.length();
                for (int x = 0; x < y; x++) {
                    s_rsid = s_rsid + " "; //add padding spaces
                }
            }
            if (s_chr.length() < 2) s_chr = s_chr + " ";
            std::string s;
            s = s_rsid + "  " + s_chr + "  " + s_gene + "  " + s_riskallele + "  " + s_oddsratio;
            std::wstring str2(s.length(), L' '); // Make room for characters
            int lcount, lindex = -1;


            // Copy string to wstring.
            std::copy(s.begin(), s.end(), str2.begin());
            wcsncpy_s(global_s, str2.c_str(), 255);
            lcount = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETCOUNT, 0, 0);
            if (lcount > 0)   lindex = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_FINDSTRING, 0, (LPARAM)global_s); //prevent dulpicates
            if (lindex == -1)  SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)global_s);
            //clear entered data
            LPWSTR lp = (const_cast <LPTSTR>(L""));
            SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT3), lp);
            SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_ALLES1), lp);
            SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT2), lp);
            SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_CHRNUM), lp);
            SetWindowTextW(GetDlgItem(hDlg, IDC_RSIDP), lp);
        }
                      break;
        case IDC_LIST1:
        {
            switch (HIWORD(wParam))
            {
                //as an entry been double clicked
            case LBN_DBLCLK:
                int signed lcount = 0;
                int gselected = -1;
                HWND plst = GetDlgItem(hDlg, IDC_LIST1);
                lcount = (int)SendMessage(plst, LB_GETCOUNT, 0, 0);
                if (lcount > 0 && lcount != -1)//Ensure on left double click there are entries to delete!
                {
                    if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, Deletemsg) == TRUE) {
                        gselected = SendMessage(plst, LB_GETCURSEL, 0, 0);
                        //Delete entry an redraw to update
                        if (gselected != -1) SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_DELETESTRING, gselected, 0);
                    } //trigger WM_PAINT  
                    InvalidateRect(hDlg, NULL, TRUE);
                    UpdateWindow(hDlg);
                    return TRUE;
                }

            }
            return TRUE;
        }
        break;
        }
        break;
    }
    break;
case WM_DESTROY: {
                  // Clean up the font we created
                  HFONT hFont = (HFONT)RemoveProp(hDlg, L"PATHOGEN_FONT");
                  if (hFont) {
                                DeleteObject(hFont);
                              }

                   // Clean up the icon (your existing code)
                   HICON hicon = (HICON)RemoveProp(hDlg, L"DIALOG_ICON");
                   if (hicon) {
                                DestroyIcon(hicon);
                               }
                   return TRUE;
                  }    
	 break;//the break is redundant here but left incase the code is altered

    }
  return (INT_PTR)FALSE;
}

