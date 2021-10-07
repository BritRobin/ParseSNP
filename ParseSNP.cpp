// ParseSNP.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ParseSNP.h"
#include "SnipParser.h"
#include <shobjidl_core.h>
#include <string>
#include <atlstr.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <WinUser.h>



#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//Global class instance
SnipParser x;
//START:Global returned values
int rs_number = 0; //RS Number 
int position = 0; //relative position
char chromosome[4] = { NULL,NULL,NULL,NULL };//chromosome number
char allele1 = NULL, allele2 = NULL;
wchar_t global_s[256];
//END:Global returned values

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
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
                 
                  }
               }
               return TRUE;
        }
        case IDC_COPYPROJ:
        {
         int lindex = -1;
         int lcount = 0;
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
         if(lindex==-1)  SendMessage(GetDlgItem(aDiag, IDC_LIST3), LB_ADDSTRING, 0, (LPARAM)global_s);

            //trigger WM_PAINT              
         InvalidateRect(aDiag, NULL, TRUE);
         UpdateWindow(aDiag);
         break;
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
        return TRUE;
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
                                    //Unpadate ramining disgarded VG code lines
                                    s = std::to_string(untranslated);
                                    lp = A2W_EX(s.c_str(), s.length());
                                    SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT_TRANS2), lp);
                                    //Update count and path per normal
                                    ScreenUpdate(hWnd, count, pszFilePath);
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
                               if(x.FTDNA(pszFilePath))
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
                                
                                ScreenUpdate(hWnd, count, pszFilePath);
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
                                    ScreenUpdate(hWnd, count, pszFilePath);
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
                hr = CoCreateInstance(::CLSID_FileSaveDialog, NULL, CLSCTX_ALL, ::IID_IFileSaveDialog,  reinterpret_cast<void**>(&pFileWrite) );

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
                            }
                        }
                    }
                    pFileWrite->Release();
                }
                CoUninitialize();
            }
            break;
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
        //Start Based off Drake Wu - MSFT code on Stackoverflow
        RECT cRect;
        RECT Rect = { 0 };
        if (aDiag == NULL) //added to stop recreate on redraw
        {
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
                //Tab_Controls = DoCreateTabControl(aDiag);
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
void ScreenUpdate(HWND hWnd, int unsigned x, PWSTR FilePath)
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
        SendMessageW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), EM_SETLIMITTEXT, 9, 0); //bug fux 3/9/21
        //Show source path
        SetWindowTextW(GetDlgItem(aDiag, IDC_SOURCE), FilePath);
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
    EnableWindow(GetDlgItem(aDiag, IDC_COPYCLIP), FALSE);
    EnableWindow(GetDlgItem(aDiag, IDC_COPYPROJ), FALSE);
    //trigger WM_PAINT              
    InvalidateRect(aDiag, NULL, TRUE);
    UpdateWindow(aDiag);
}



// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    //Because we have crated a form dialog after the menu
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



////// TEMP TEST //////
// Creates a tab control, sized to fit the specified parent window's client
//   area, and adds some tabs. 
// Returns the handle to the tab control. 
// hwndParent - parent window (the application's main window). 
// 
////// TEMP TEST ///////*
/*
HWND DoCreateTabControl(HWND hwndParent)
{
    RECT rcClient;
    INITCOMMONCONTROLSEX icex;
    
    TCITEM tie;
    TCHAR achTemp[256];  // Temporary buffer for strings.


   icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icex.dwICC = ICC_TAB_CLASSES;
   InitCommonControlsEx(&icex);

    // Get the dimensions of the parent window's client area, and 
    // create a tab control child window of that size. Note that hInst
    // is the global instance handle.
  

    GetClientRect(hwndParent, &rcClient);
   //position:  rect.left, rect.top
// Messy and I do not know why it works!!!
   long lTop, lBotton, lRight, lLeft;
   lTop = 176;
   lBotton = rcClient.bottom - (long)200;
   lRight = rcClient.right - (long)128;
   lLeft = 10;
   //WS_EX_CONTROLPARENT, WC_TABCONTROL
  //hwndTab = CreateWindow( WC_TABCONTROL, L"", WS_EX_CONTROLPARENT, | WS_CLIPSIBLINGS | WS_VISIBLE, lLeft, lTop, lRight, lBotton, hwndParent, nullptr, hInst, nullptr);
   hwndTab = CreateWindowEx(WS_EX_CONTROLPARENT, WC_TABCONTROL, 0, WS_VISIBLE | WS_CHILD, lLeft, lTop, lRight, lBotton, hwndParent, HMENU(600), 0, 0);

    if (hwndTab == NULL)
    {
        return NULL;
    }

    // Add tabs for each day of the week. 
    tie.mask = TCIF_TEXT | TCIF_IMAGE;
    tie.iImage = -1;
    tie.pszText = achTemp;

    //TAB 1

    LoadStringW(hInst, IDS_PROJECTAB, achTemp, sizeof(achTemp) / sizeof(achTemp[0]));


    if (TabCtrl_InsertItem(hwndTab, IDS_PROJECTAB, &tie) == -1)
    {
        DestroyWindow(hwndTab);
        return NULL;
    }

   

   hwndTab_1 = CreateWindowW(WC_STATIC,
        L"",
       WS_CHILD | WS_VISIBLE | WS_BORDER,
        20, 20,
        lRight - 40,
        lBotton - 40,
        hwndTab,
        NULL,
        hInst,
        NULL);
    //TAB 2

    LoadStringW(hInst, IDS_MULTITAB, achTemp, sizeof(achTemp) / sizeof(achTemp[0]));
    if (TabCtrl_InsertItem(hwndTab, IDS_MULTITAB, &tie) == -1)
    {
        DestroyWindow(hwndTab);
        return NULL;
    }

    return hwndTab;
}

// Creates a child window (a static control) to occupy the tab control's 
//   display area. 
// Returns the handle to the static control. 
// hwndTab - handle of the tab control. 
// 
HWND DoCreateDisplayWindow(HWND hwndTab)
{
    HWND hwndStatic = CreateWindow(WC_STATIC, L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        100, 100, 100, 100,        // Position and dimensions; example only.
        hwndTab, NULL, hInst,    // hInst is the global instance handle
        NULL);
    return hwndStatic;
}

*/