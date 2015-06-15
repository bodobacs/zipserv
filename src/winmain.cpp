// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#define _WINSOCKAPI_

#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <sstream>

#include "resource.h"

#include "zsrv.h"

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("win32app");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Zserv");

HINSTANCE hInst;

using namespace std;

czsrv server;
int portnumber = 19000;
std::string zipname;

unsigned __stdcall server_thread(void *pargs)
{

	server.init(zipname, portnumber);

	if(server.open_zipfile())
	{
		server.run_server();
		server.close_zipfile();
	}

	_endthreadex(0);
	return 0;

}

INT_PTR CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
	 case WM_INITDIALOG:
	  return (TRUE);

	  case WM_COMMAND:
	  	if(LOWORD(wParam) == ID_START)
		{
			HANDLE myhandle = (HANDLE)_beginthreadex(0, 0, &server_thread, 0, 0, 0);
			CloseHandle(myhandle);

			return (TRUE);
		}

		if(LOWORD(wParam) == ID_STOP)
		{
			server.stop();	
			return (TRUE);
		}

		if(LOWORD(wParam) == ID_SELECT)
		{
			OPENFILENAME ofn;
			char szFileName[MAX_PATH] = "";

			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
			ofn.hwndOwner = NULL;
			ofn.lpstrFilter = "Zip files (*.zip)\0*.zip\0All Files (*.*)\0*.*\0";
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
			ofn.lpstrDefExt = "txt";

			if(GetOpenFileName(&ofn))
			{
				zipname.assign(szFileName);
				SetDlgItemText(hDlg, ID_ZIPNAME, zipname.c_str());
			}
		}

		if (LOWORD(wParam) == ID_BROWSER)
		{
			std::stringstream ss;
			ss << "http://localhost:" << portnumber;

			ShellExecute(NULL, "open", ss.str().c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
			return (TRUE);
		}

		if (LOWORD(wParam) == IDOK)
		{
		       EndDialog(hDlg, TRUE);
		       return (TRUE);
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
		       EndDialog(hDlg, FALSE);
		       return (TRUE);
		}
		break;
	 }

	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	DialogBox( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WndProc );

	return NULL;
}

