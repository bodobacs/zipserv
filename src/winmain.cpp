// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#define WIN32_LEAN_AND_LEAN
//#define _WINSOCKAPI_

#include "winsock2.h"
#include <windows.h>

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <sstream>

#include "resource.h"

#include "zsrv.h"

#define WM_SERVER WM_APP + 1

HINSTANCE hInst;
HANDLE server_thread_handle = NULL;

HWND g_hwnd = NULL;

bool running = false;
using namespace std;

czsrv server;
int portnumber = 19000;
std::string zipname;

unsigned __stdcall server_thread(void *pargs)
{
	if(!running)
	{
		if(server.open_archive(zipname) && server.init(portnumber))
		{
			running = true;
			SendMessage(g_hwnd, WM_SERVER, 0, 0);

			std::stringstream ss; ss << "Server running. Port: " << portnumber << std::endl;
			SetDlgItemText(g_hwnd, ID_STATUS, ss.str().c_str());

			SetDlgItemInt(g_hwnd, ID_PORT, portnumber, FALSE);

			while(server.run_server());
			server.cleanup();
		}else{
			portnumber = (portnumber + 1 < 19100) ? portnumber + 1 : 19000;
		}

		std::stringstream ss; ss << "Server stopped" << portnumber << std::endl;
		SetDlgItemText(g_hwnd, ID_STATUS, ss.str().c_str());

		running = false;
		SendMessage(g_hwnd, WM_SERVER, 0, 0);
	}

	return 0;

}

INT_PTR CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	g_hwnd = hDlg;
	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, ID_PORT, portnumber, FALSE);
		return (TRUE);

	case WM_SERVER:
		if(running)
		{
			EnableWindow( GetDlgItem( hDlg, ID_BROWSER), TRUE);
			EnableWindow( GetDlgItem( hDlg, ID_PORT), FALSE);
			
		}else{
			CloseHandle(server_thread_handle);
			server_thread_handle = NULL;

			EndDialog(hDlg, TRUE);
		}
	  return (TRUE);
			
	  case WM_COMMAND:
	  	switch LOWORD(wParam){
		/*	case ID_START:
				return (TRUE);

			case ID_STOP:
				SetDlgItemText(hDlg, ID_STOP, "Stopping server ...");
				server.stop();	
				return (TRUE);
		*/	case ID_SELECT:
				{
					OPENFILENAME ofn;
					char szFileName[MAX_PATH] = "";

					ZeroMemory(&ofn, sizeof(ofn));

					ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
					ofn.hwndOwner = NULL;
					ofn.lpstrFilter = "Known file formats: .zip, .chm\0*.zip;*.chm\0Zip files (*.zip)\0*.zip\0Chm files (*.chm)\0*.chm\0All Files (*.*)\0*.*\0";
					ofn.lpstrFile = szFileName;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
					ofn.lpstrDefExt = "txt";

					if(GetOpenFileName(&ofn))
					{
						zipname.assign(szFileName);
						if(zipname.length() > 0)
						{
							SetDlgItemText(hDlg, ID_ZIPNAME, zipname.c_str());
							if(!running)
							{
								server_thread_handle = (HANDLE)_beginthreadex(0, 0, &server_thread, 0, 0, 0);
							}else{
								if(!server.open_archive(zipname)) EnableWindow( GetDlgItem( hDlg, ID_BROWSER), FALSE);
							}
						}
					}
				}
			   return (TRUE);

			case ID_BROWSER:
			       {
					std::stringstream ss;
					ss << "http://localhost:" << portnumber;

					ShellExecute(NULL, "open", ss.str().c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
					return (TRUE);
			       }

			case IDOK:
			case IDCANCEL:
				if(running)
				{	
					server.stop();
					SetDlgItemText(hDlg, IDOK, "Stopping server ...");

					//disable all buttons
					EnableWindow( GetDlgItem( hDlg, ID_SELECT), FALSE);
					EnableWindow( GetDlgItem( hDlg, ID_PORT), FALSE);
					EnableWindow( GetDlgItem( hDlg, ID_BROWSER), FALSE);
					EnableWindow( GetDlgItem( hDlg, IDOK ), FALSE);
				}else{
					EndDialog(hDlg, TRUE);
				}

			   return (TRUE);
		}//WM_COMMAND switch
		break;
	 }//message

	return FALSE;
}

#ifdef WIN32

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	DialogBox( hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WndProc );

	return 0;
}

#else
//this makes a console and a window

int main()
{
	return _tWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOW);
}

#endif

