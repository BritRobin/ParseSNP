﻿// ParseSNP.cpp : Defines the entry point for the application.
//Visual Studio 2022 build

#include "framework.h"
#include "ParseSNP.h"
#include "SnipParser.h"
#include <shobjidl_core.h>
#include <string>
#include <atlstr.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <Winuser.h>
#include <shlobj_core.h>
#include <filesystem>

#define MAX_LOADSTRING 100
namespace fs = std::filesystem; // In C++17 

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//Global class instance
SnipParser x;
//START:Global returned values
int rs_number = 0; //RS Number 
int position = 0; //relative position
int loadedFiletype = 0; // 1=Ancestory 2=FTNDA 3=23toM3
char chromosome[4] = { NULL,NULL,NULL,NULL };//chromosome number
char allele1 = NULL, allele2 = NULL;
wchar_t global_s[256];
std::wstring currentProject;
std::wstring myPath;
std::wstring SourceFilePath;


HWND ghWnd;
//END:Global returned values

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ProjectDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Deletemsg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Pathogen(HWND, UINT, WPARAM, LPARAM);
HRESULT OnSize(HWND hwndTab, LPARAM lParam);
BOOL OnNotify(HWND hwndTab, HWND hwndDisplay, LPARAM lParam);
HWND DoCreateTabControl(HWND hwndParent);

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
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PARSESNP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
   const int height = 700;// was CW_USEDEFAULT
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, width, height, nullptr, nullptr, hInstance, nullptr);

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
    switch (Message)
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

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
             InvalidateRect(aDiag, NULL, TRUE);
             UpdateWindow(aDiag);
             return TRUE;
             }
            
            }
            return TRUE;
        }
        case IDC_BUTTON_SEARCH:
        {
           TCHAR buffer[15] = { 0 };

              if (GetWindowText(GetDlgItem(aDiag, IDC_EDIT_SEARCH), buffer, 15))
               {
            
                  rs_number = _wtoi(buffer);
                  //populate global values
                  if (x.RsSearch(&rs_number, &chromosome[0], &chromosome[1], &chromosome[2], &chromosome[3], &position, &allele1, &allele2))
                  {
                      //Covert RS number for display
                      std::string s = std::to_string(rs_number);
                      USES_CONVERSION_EX;
                      LPWSTR lp = A2W_EX(s.c_str(), s.length());
                      SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT1), lp);
                      //Covert chromosome for display
                      s = chromosome;
                      lp = A2W_EX(s.c_str(), s.length());
                      SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_CHRNUM), lp);
                      //covert poistion for display
                      s = std::to_string(position);
                      lp = A2W_EX(s.c_str(), s.length());
                      SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_POSIT), lp);
                      s = allele1;
                      lp = A2W_EX(s.c_str(), s.length());
                      SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_ALLES1), lp);
                      s = allele2;
                      lp = A2W_EX(s.c_str(), s.length());
                      SetWindowTextW(GetDlgItem(aDiag, IDC_AllELE2), lp);
                      EnableWindow(GetDlgItem(aDiag, IDC_COPYCLIP), TRUE);
                      EnableWindow(GetDlgItem(aDiag, IDC_COPYPROJ), TRUE);
                      //trigger WM_PAINT              
                      InvalidateRect(aDiag, NULL, TRUE);
                      UpdateWindow(aDiag);
                  }
                  else
                  { //Not Found dialog
                      return FALSE;
                  }
               }
               return TRUE;
        }
        case IDC_COPYPROJ:
        {
         int lindex = -1;
         int lcount = -1;
         //Character allign chr numver and letters
         std::string chr_s= (std::string)(chromosome + NULL);

         if (atoi(chromosome) < 9 && chr_s != "MT") chr_s = " " + chr_s;

         std::string s = " Chromosome: " + chr_s + "  RSID " + " RS" + std::to_string(rs_number) + "  Postion: " + std::to_string(position) + "  Alleles: " + allele1 + "  " + allele2 + "";
         std::wstring str2(s.length(), L' '); // Make room for characters

         // Copy string to wstring.
         std::copy(s.begin(), s.end(), str2.begin());
         wcsncpy_s(global_s,  str2.c_str(),255);

         lcount = SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_GETCOUNT, 0, 0);
         if(lcount > 0)   lindex = SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_FINDSTRING, 0, (LPARAM)global_s); //prevent dulpicates
         if(lindex == -1)  SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_ADDSTRING, 0, (LPARAM)global_s);
         if(lcount == 0) EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_ENABLED);

            //trigger WM_PAINT              
         InvalidateRect(aDiag, NULL, TRUE);
         UpdateWindow(aDiag);
         return TRUE;
        }
        
        case IDC_COPYCLIP:
        {
            if(OpenClipboard(hwnd))
             { 
             std::string s = "RS";
             s += std::to_string(rs_number) + " Chromosome:" + chromosome + " Postion:" + std::to_string(position) + " Alleles: " + allele1 + " " + allele2;
             EmptyClipboard();
             HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
             if (!hg) {
                CloseClipboard();
                return FALSE;
             }
             memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
             GlobalUnlock(hg);
             SetClipboardData(CF_TEXT, hg);
             CloseClipboard();
             GlobalFree(hg);
             return TRUE;
             }
            return FALSE;
        }
        
        default:
            return DefWindowProc(hwnd, Message, wParam, lParam);
        }
        
    }

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
        {   HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
            COINIT_DISABLE_OLE1DDE);
        PWSTR pszFilePath;
        if (SUCCEEDED(hr))
        {
            IFileOpenDialog* pFileOpen;

            // Create the FileOpenDialog object.
            hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

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
                        std::fstream  fstrm;
                        if (!fstrm.bad())
                        {
                            CString temp = "ProjectManifest.ptxt";
                            std::wstring str;
                            std::string   linebuffer;
                            char lbuffer[260];
                            //configPath = currentProject.c_str() + temp;
                            Target = pszFilePath;
                            currentProject = pszFilePath;
                            currentProject = currentProject.substr(0, currentProject.length() - temp.GetLength());//SET CURRENT PROJECT!!!
                            //filestream 
                            fstrm.open(Target, std::ios::in);
                            //Check file was opened  
                            if (fstrm.is_open()) {
                                while (!fstrm.eof())
                                {
                                    //read a line into a temorary buffer
                                    fstrm.getline(lbuffer, 256);//DNA file to open
                                    for (int i = 0; i < 257; i++) {
                                        if (lbuffer[i] == '\n') {
                                            lbuffer[i] = NULL;
                                            break;
                                        }
                                    }
                                    //file name and path wide                         
                                    str = A2T(lbuffer);
                                    fstrm.getline(lbuffer, 256);//DNA file type to open
                                    for (int i = 0; i < 257; i++) {
                                        if (lbuffer[i] == '\n') {
                                            lbuffer[i] = NULL;
                                            break;
                                        }
                                    }
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
                                            ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString);
                                            //New code Beta 0.2
                                            SourceFilePath = str;
                                            loadedFiletype = 1;
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
                                            USES_CONVERSION_EX;
                                            LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                            SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                            //Unpdate ramining disgarded VG code lines
                                            s = std::to_string(untranslated);
                                            lp = A2W_EX(s.c_str(), s.length());
                                            SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                            //Update count and path per normal
                                             //New code Beta 0.2
                                            std::string strx;
                                            strx = x.NCBIBuild();
                                            CA2CT pszConvertedAnsiString(strx.c_str());
                                            ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString);
                                            //New code Beta 0.2
                                            SourceFilePath = str;
                                            loadedFiletype = 2;
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
                                            USES_CONVERSION_EX;
                                            LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                            SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                            //Unpadate ramining disgarded VG code lines
                                            s = std::to_string(untranslated);
                                            lp = A2W_EX(s.c_str(), s.length());
                                            SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                            //Update count and path per normal
                                            //New code Beta 0.2
                                            std::string strx;
                                            strx = x.NCBIBuild();
                                            CA2CT pszConvertedAnsiString(strx.c_str());
                                            ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString);
                                            //New code Beta 0.2
                                            SourceFilePath = str;
                                            loadedFiletype = 3;
                                        }

                                    }

                                    fstrm.getline(lbuffer, 256);
                                    for (int i = 0; i < 257; i++) {
                                        if (lbuffer[i] == '\n') {
                                            lbuffer[i] = NULL;
                                            break;
                                        }
                                    }
                                    xsw = atoi(lbuffer);
                                    if (xsw == 0) {
                                        fstrm.close();
                                        break;
                                    }
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox                          

                                    for (int lst = 0; lst < xsw; lst++) {
                                        fstrm.getline(lbuffer, 256);//read project line
                                        int i = 0; // moved outside loop scope
                                        for (; i < 257; i++) {
                                            if (lbuffer[i] == '\n') {
                                                lbuffer[i] = NULL;
                                                break;
                                            }
                                        }
                                        std::string s = lbuffer;
                                        std::wstring str2(s.length(), L' '); // Make room for characters
                                        // Copy string to wstring.
                                        std::copy(s.begin(), s.end(), str2.begin());
                                        wcsncpy_s(global_s, str2.c_str(), 255);
                                        SendMessage(plst, LB_ADDSTRING, 0, (LPARAM)global_s);
                                        if(lst==1)EnableMenuItem(GetMenu(hWnd), ID_PROJEX, MF_BYCOMMAND | MF_ENABLED);
                                    }
                                    fstrm.close();
                                    InvalidateRect(aDiag, NULL, TRUE);
                                    UpdateWindow(aDiag);
                                    break;
                                }
                                
                              }
                            }
                        }
                    }
                }
            }
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
        if (SourceFilePath.length() != wchar_t(""))
        {   /* Derive the target path from source file and project path */
            std::wstring pathfilname;
            wchar_t filename[256];
            int position, totallength, sublen;
            pathfilname = SourceFilePath;
            totallength = pathfilname.length();
            position = pathfilname.find_last_of(L"\\");
            sublen = totallength - position;
            pathfilname.copy(filename, sublen, position);
            filename[sublen] = NULL;
            pathfilname = currentProject + filename;
            //Copy the original DNA file retaining its original name to the project folder
            LPCWSTR Source = SourceFilePath.c_str();
            LPCWSTR Target = pathfilname.c_str();
            CopyFile(Source, Target, TRUE);
            //Write Project configuration files
            //Open file for write 
            std::fstream  fstrm;
            if (!fstrm.bad())
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
                    _itoa_s(loadedFiletype, tc, 10); //the function to use to load it
                    lbuffer += tc;
                    lbuffer += "\n";
                    //Get Project window entry count
                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                    lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);

                    _itoa_s(lcount, tc, 10);
                    lbuffer += tc;
                    lbuffer += "\n";
                    TCHAR text[256];
                    for (int x = 0, ln = 0; x <= lcount; x++)
                    {
                        ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
                        if (ln > 0 and ln < 256) {
                            SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
                            //checked: the returned text is Zero terminated at tchar[ln] !
                            CT2CA pszConvertedAnsiString(text);
                            lbuffer += pszConvertedAnsiString;
                            lbuffer += "\n";
                        }

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

            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                    IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

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
                            PWSTR pszFilePath;
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
                                    USES_CONVERSION_EX;
                                    LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                    //Update remaining disgarded VG code lines
                                    s = std::to_string(untranslated);
                                    lp = A2W_EX(s.c_str(), s.length());
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                    //Update count and path per normal
                                    //New code Beta 0.2
                                    std::string strx;
                                    strx = x.NCBIBuild();
                                    CA2CT pszConvertedAnsiString(strx.c_str());
                                    ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString);
                                    //New code Beta 0.2
                                    SourceFilePath = pszFilePath;
                                    loadedFiletype = 3;
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_LOAD, MF_BYCOMMAND | MF_ENABLED);
                                    InvalidateRect(aDiag, NULL, TRUE);
                                    UpdateWindow(aDiag);
                                }
                                CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                    CoUninitialize();
                }


            }
            break;
        }
        case ID_FILE_OPENFTDNA:
        {

            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                    IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

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
                            PWSTR pszFilePath;
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
                                    USES_CONVERSION_EX;
                                    LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                                    //Unpadate ramining disgarded VG code lines
                                    s = std::to_string(untranslated);
                                    lp = A2W_EX(s.c_str(), s.length());
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                    //Update count and path per normal
                                    //New code Beta 0.2
                                    std::string strx;
                                    strx = x.NCBIBuild();
                                    CA2CT pszConvertedAnsiString(strx.c_str());
                                    ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString);
                                    //New code Beta 0.2
                                    SourceFilePath = pszFilePath;
                                    loadedFiletype = 2;
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox  
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED); 
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_LOAD, MF_BYCOMMAND | MF_ENABLED);
                                    InvalidateRect(aDiag, NULL, TRUE);
                                    UpdateWindow(aDiag);
                                }
                                CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                    CoUninitialize();
                }

            }
            break;
        }

        case ID_OPENFILE:
        {
            //TEST Code
            //x.FConvert();
            //TEST Code
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                IFileOpenDialog* pFileOpen;

                // Create the FileOpenDialog object.
                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                    IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

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
                            PWSTR pszFilePath;
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
                                    ScreenUpdate(hWnd, count, pszFilePath, pszConvertedAnsiString);
                                    //New code Beta 0.2
                                    SourceFilePath = pszFilePath;
                                    loadedFiletype = 1;
                                    HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                    SendMessage(plst, LB_RESETCONTENT, NULL, NULL);//CLR listbox
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PROJEX, MF_BYCOMMAND | MF_GRAYED);
                                    EnableMenuItem(GetMenu(GetParent(aDiag)), ID_PATHOGENICS_LOAD, MF_BYCOMMAND | MF_ENABLED);
                                    InvalidateRect(aDiag, NULL, TRUE);
                                    UpdateWindow(aDiag);
                                }
                                CoTaskMemFree(pszFilePath);
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                    CoUninitialize();
                }


            }
            break;
        }

        case ID_FILE_EXPORT:
        {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
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
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                x.AncestoryWriter((pszFilePath));
                                CoTaskMemFree(pszFilePath);
                                pItem->Release();
                                InvalidateRect(aDiag, NULL, TRUE);
                                UpdateWindow(aDiag);
                            }
                        }
                    }
                    pFileWrite->Release();
                }
                CoUninitialize();
            }
            break;
        }
        case ID_PROJEX: {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
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
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {//Open file for write 
                                std::fstream  fstrm;
                                if (!fstrm.bad())
                                {   //filestream 
                                    fstrm.open(pszFilePath, std::ios::out);
                                    if (fstrm.is_open()) {
                                        //Get Project window entry count
                                        HWND plst = GetDlgItem(aDiag, IDC_LIST3);
                                        int  lcount;
                                        std::string  lbuffer;
                                        lcount = SendMessage(plst, LB_GETCOUNT, 0, 0);
                                        TCHAR text[256];
                                        for (int x = 0, ln = 0; x <= lcount; x++)
                                        {
                                            ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
                                            if (ln > 0 and ln < 256) {
                                                SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
                                                //checked: the returned text is Zero terminated at tchar[ln] !
                                                CT2CA pszConvertedAnsiString(text);
                                                lbuffer += pszConvertedAnsiString;
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
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), aDiag, About);
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
            InvalidateRect(aDiag, NULL, TRUE);
            UpdateWindow(aDiag);
            break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (aDiag == NULL) //added to stop recreate on redraw
        {
            //Start Based off Drake Wu - MSFT code on Stackoverflow
            RECT cRect;
            RECT Rect = { 0 };
            aDiag = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, (DLGPROC)FormDlgProc);
            if (aDiag != NULL)
            {
                GetWindowRect(aDiag, &Rect);
                GetWindowRect(hWnd, &cRect);
                Rect.right = max(Rect.right, cRect.right + 7);//Windows 10 has thin invisible borders on left, right, and bottom
                Rect.bottom = max(Rect.bottom, cRect.bottom + 7);
                SetWindowPos(aDiag, HWND_TOP, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
                ShowWindow(aDiag, SW_SHOW);
                std::string s = "0";
                USES_CONVERSION_EX;
                LPWSTR lp = A2W_EX(s.c_str(), s.length());
                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp);
                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS), lp);
                SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
               }

            //End Based off Drake Wu - MSFT code on Stackoverflow
        }
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
      }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
 }
//}
void ScreenUpdate(HWND hWnd, int unsigned x, PWSTR FilePath, PWSTR build)
{
    if (x > 0)
    {//Data loaded
        std::string s = std::to_string(x);
        USES_CONVERSION_EX;
        LPWSTR lp = A2W_EX(s.c_str(), s.length());
        SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp);
        EnableWindow(GetDlgItem(aDiag, IDC_BUTTON_SEARCH), TRUE);
        EnableWindow(GetDlgItem(aDiag, IDC_EDIT_SEARCH), TRUE);
        //Set a limit on rs field
        SendMessageW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), EM_SETLIMITTEXT, 9, 0); //bug fix 3/9/21
        //Show source path
        SetWindowTextW(GetDlgItem(aDiag, IDC_SOURCE), FilePath);
        //show NCBI BUILD
        SetWindowTextW(GetDlgItem(aDiag, IDC_BUILD), build);
        EnableMenuItem(GetMenu(hWnd), ID_FILE_EXPORT, MF_BYCOMMAND | MF_ENABLED);
        }
          else {
                 std::string s = "0";
                 USES_CONVERSION_EX;
                 LPWSTR lp = A2W_EX(s.c_str(), s.length());
                 SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp);
                 EnableWindow(GetDlgItem(aDiag, IDC_BUTTON_SEARCH), FALSE);
                 EnableWindow(GetDlgItem(aDiag, IDC_EDIT_SEARCH),   FALSE);
                 //Set a limit on rs field
                 SendMessageW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), EM_SETLIMITTEXT, 9, 0); //bug fux 3/9/21
                 //Show source path
                 SetWindowTextW(GetDlgItem(aDiag, IDC_SOURCE), NULL);  
                 EnableMenuItem(GetMenu(hWnd), ID_FILE_EXPORT, MF_BYCOMMAND | MF_GRAYED);  //ensure export is deactivated
                 }
    //Covert RS number for display
    std::string s = "";
    USES_CONVERSION_EX;
    LPWSTR lp = A2W_EX(s.c_str(), s.length());
    SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT1), lp);
    SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_CHRNUM), lp);
    SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_POSIT), lp);
    SetWindowTextW(GetDlgItem(aDiag, IDC_EDIT_ALLES1), lp);
    SetWindowTextW(GetDlgItem(aDiag, IDC_AllELE2), lp);
    EnableWindow(GetDlgItem(aDiag, IDC_COPYCLIP), FALSE);
    EnableWindow(GetDlgItem(aDiag, IDC_COPYPROJ), FALSE);
}
// Message handler for new project box.
INT_PTR CALLBACK ProjectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have crated a form dialog after the menu
    ShowWindow(hDlg, 5);

    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            TCHAR buffer[256] = { 0 };
            std::wstring temp;
            if (GetWindowText(GetDlgItem(hDlg, IDC_ProjectNm), buffer, 256)) {
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

    switch (message)
    {
    case WM_INITDIALOG:
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
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
//Delete box
INT_PTR CALLBACK Pathogen(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have created a form dialog after the menu
    ShowWindow(hDlg, 5);

    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
            
        case IDC_SAVE: {
            TCHAR buffer[260] = { 0 };
            std::string s_study,s_URL,s_ncbiref;
            if (GetWindowText(GetDlgItem(hDlg, IDC_STUDY), buffer, 255))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_study = pszConvertedAnsiString;
            }
            else  break; //mandatory field
            if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT5), buffer, 255))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_URL += pszConvertedAnsiString;
            }else  break; //mandatory field
            if (GetWindowText(GetDlgItem(hDlg, IDC_NCBIref), buffer, 255))
            {
                CT2CA pszConvertedAnsiString(buffer);
                s_ncbiref += pszConvertedAnsiString;
            }//optional
            if (s_ncbiref.length() == 0) s_ncbiref = "--";

            //name and write file
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                COINIT_DISABLE_OLE1DDE);
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
                            PWSTR pszFilePath;
                            WCHAR ext[5];
                            WCHAR filename[260];

                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            // Display the file name to the user.
                            if (SUCCEEDED(hr))
                            {
                                ext[0] = '.';
                                ext[1] = 'P';
                                ext[2] = 'P';
                                ext[3] = 'I';
                                ext[4] = NULL;
                                StrCpyW(filename, pszFilePath);//we know 
                                wcscat_s(filename, ext);
                                {//Write the file

                                    //Open file for write 
                                    std::fstream  fstrm;
                                    if (!fstrm.bad())
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
                                            for (int x = 0, ln = 0; x <= lcount; x++)
                                            {
                                                ln = SendMessage(plst, LB_GETTEXTLEN, x, NULL);
                                                if (ln > 0 and ln < 256) {
                                                    SendMessage(plst, LB_GETTEXT, x, (LPARAM)text);
                                                    //checked: the returned text is Zero terminated at tchar[ln] !
                                                    CT2CA pszConvertedAnsiString(text);
                                                    lbuffer += pszConvertedAnsiString;
                                                    lbuffer += "\n";
                                                    const char* write_it = lbuffer.c_str();
                                                    fstrm.write(write_it, lbuffer.length());

                                                }
                                            }
                                            fstrm.close();
                                        }
                                    }
                                }
                                CoTaskMemFree(pszFilePath);
                                pItem->Release();
                                InvalidateRect(aDiag, NULL, TRUE);
                                UpdateWindow(aDiag);
                            }
                        }
                    }
                    pFileWrite->Release();
                }
                CoUninitialize();
            }
         break;
        }

        case IDCANCEL://fall tho to exit I'm sure this style is frowd upon
        case IDC_EXITP: { 
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
            }
        case IDC_ENTER: {//The main code, 
            TCHAR buffer[260] = { 0 };
            std::string s_rsid,s_chr,s_gene, s_riskalelle, s_oddsratio;
            //get rsid number
           if (GetWindowText(GetDlgItem(hDlg,IDC_RSIDP), buffer, 15)) 
            {
               std::string s;
               //the dialog only allows numbers to be entered
               CT2CA pszConvertedAnsiString(buffer);
               s = pszConvertedAnsiString;
               s_rsid = "RS" + s;

            } else  break; //mandatory field

           //Get chromosone number
           if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT_CHRNUM), buffer, 15))
            {
               std::string s;
               CT2CA pszConvertedAnsiString(buffer);
               s = pszConvertedAnsiString;

               if ((strtod(s.data(), NULL) > 0 && strtod(s.data(), NULL) < 23) || s == "X" || s == "Y" || s == "MT") {
                   s_chr = s;
               }
               else break; //mandatory field
           }
               //Get Gene optional
           if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT2), buffer, 15))
            {
               CT2CA pszConvertedAnsiString(buffer);
               s_gene = pszConvertedAnsiString;
            }
               
           if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT_ALLES1), buffer, 15))
            {
               std::string s;
               CT2CA pszConvertedAnsiString(buffer);
               s = pszConvertedAnsiString;
               if (s != "A" && s != "C" && s != "G" && s != "T") break; //Mandatory!!
               s_riskalelle = s;
            }

           //Get Odds Ratio optional but should be inclued if available
           if (GetWindowText(GetDlgItem(hDlg, IDC_EDIT3), buffer, 15))
            {
              CT2CA pszConvertedAnsiString(buffer);
              s_oddsratio = pszConvertedAnsiString;
            }
           //NO Missing Fields for file read!
           if (s_gene.length() == 0) s_gene = "----";
           if (s_oddsratio.length() == 0) s_oddsratio = "-.--";

           std::string s;
           s = s_rsid + "  " + s_chr + "  " + s_gene + "  " + s_riskalelle + "  " + s_oddsratio;
           std::wstring str2(s.length(), L' '); // Make room for characters
           int lcount,lindex=-1;


         // Copy string to wstring.
           std::copy(s.begin(), s.end(), str2.begin());
           wcsncpy_s(global_s, str2.c_str(), 255);
           lcount = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETCOUNT, 0, 0);
           if (lcount > 0)   lindex = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_FINDSTRING, 0, (LPARAM)global_s); //prevent dulpicates
           if (lindex == -1)  SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)global_s);
           
        }




        }

        break;



    }
    return (INT_PTR)FALSE;
}