// ParseSNP.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "ParseSNP.h"
#include "SnipParser.h"
#include <shobjidl_core.h>
#include <string>
#include <atlstr.h>
//#include <wingdi.h>
#include <windows.h>
#include <tchar.h>
//trmp
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
//END:Global returned values

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK NotF(HWND, UINT, WPARAM, LPARAM);

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

    wcex.cbSize = sizeof(WNDCLASSEX);

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

   const int width = 985;
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
           TCHAR buffer[10] = { 0 };

           
              if (GetWindowText(GetDlgItem(aDiag, IDC_EDIT_SEARCH), buffer, 10))
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
                      DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOGNOTF), aDiag, NotF);
                  }
               }
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
            return FALSE;
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
            case ID_OPENFILE:
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
                                    int unsigned count;
                                    x.Ancestory(pszFilePath);
                                    
                                    count = x.SNPCount();
                                  if (count > 0)
                                    {//Data loaded
                                      std::string s = std::to_string(count);
                                      USES_CONVERSION_EX;
                                      LPWSTR lp = A2W_EX(s.c_str(), s.length());
                                      SetWindowTextW(GetDlgItem(aDiag, IDC_COUNT), lp);
                                       EnableWindow(
                                            GetDlgItem(aDiag, IDC_BUTTON_SEARCH),
                                            TRUE);
                                       EnableWindow(
                                           GetDlgItem(aDiag, IDC_EDIT_SEARCH),
                                           TRUE);
                                       //Set a limit on rs field
                                       SendMessageW(GetDlgItem(aDiag, IDC_EDIT_SEARCH), EM_SETLIMITTEXT, 8, 0);
                                       //Show source path
                                       SetWindowTextW(GetDlgItem(aDiag, IDC_SOURCE), pszFilePath);
                                       CoTaskMemFree(pszFilePath);
                                       //trigger WM_PAINT              
                                       InvalidateRect(aDiag, NULL, TRUE);
                                       UpdateWindow(aDiag);
                                    
                                  }

                                }
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                    CoUninitialize();
                }          

      
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

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        //Start Based off Drake Wu - MSFT code on Stackoverflow
        RECT cRect;
        RECT Rect = { 0 };
        if(aDiag == NULL) //added to stop recreate on redraw
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

// Message handler for about box.
INT_PTR CALLBACK NotF(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
//test
