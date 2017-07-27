// rerun.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "..\\..\\Exemples\\zinc\\zstrings.h"
#include "..\\..\\Exemples\\zinc\\zini.h"
#include "shlobj.h"
#include "autobuild.h"

#pragma comment (lib, "shell32.lib")

#define RR_SHOW_CMDLN		0x00000001	// show cmdline before run
#define RR_SHOW_FAILED		0x00000002	// show error on failed run
#define RR_BREAK_ON_ERROR	0x00000004

KEYWORD_NODE param_list[] =
{
					{"here%",					nullstring},		// 0
					{"cmdln%",					nullstring},		// 1
					{"windir%",					nullstring},		// 2
					{"sysdir%",					nullstring},		// 3
					{"progdir%",				nullstring},		// 4
					{"deskdir%",				nullstring},		// 5
					{"mydocdir%",				nullstring},		// 6
					{"profdir%",				nullstring},		// 7
					{"startdir%",				nullstring},		// 8
					{NULL,						nullstring}
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	LPINI_NODE ini_node = NULL;
	int exit_code = 0;
	unsigned long options;
	unsigned long i;

	mystring tempstr;
	char workdir[MAX_PATH+1];
	char *cmd;
	char *file;
	unsigned long end;

	//if(GetModuleFileName(hInstance, tempstr.buffer(), tempstr.buffersize()) == 0)

	if(GetModuleFileName(hInstance, workdir, MAX_PATH) == 0)
	{
		MessageBox(NULL, "Can't get base path.", "rerun (build "__BUILD_STR__"): Load error", MB_ICONERROR | MB_OK);
		return -1;
	}

	//tempstr.recalclen();
	//tempstr.setdata(alloc_filepath(tempstr.c_str(), FPI_DIR | FPI_FNAME));
	//tempstr += ".ini";

	param_list[0].szValue = alloc_filepath(workdir, FPI_DIR);
	if(lpCmdLine)
	{
		param_list[1].szValue = alloc_string(lpCmdLine);
	}

	param_list[2].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[2].szValue)
	{
		GetWindowsDirectory(param_list[2].szValue, MAX_PATH);
	}

	param_list[3].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[3].szValue)
	{
		GetSystemDirectory(param_list[3].szValue, MAX_PATH);
	}

	param_list[4].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[4].szValue)
	{
		SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, param_list[4].szValue);
	}

	param_list[5].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[5].szValue)
	{
		SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, param_list[5].szValue);
	}

	param_list[6].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[6].szValue)
	{
		SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, param_list[6].szValue);
	}

	param_list[7].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[7].szValue)
	{
		SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, param_list[7].szValue);
	}

	param_list[8].szValue = (char *) zalloc(MAX_PATH+1);
	if(param_list[8].szValue)
	{
		SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, SHGFP_TYPE_CURRENT, param_list[8].szValue);
	}

	file = alloc_filepath(workdir, FPI_DIR | FPI_FNAME);
	alloc_strcat(&file, ".conf");
	if(ini_load(&ini_node, file) != INI_SUCCESS)
	{
		tempstr = "ini load error: ";
		tempstr += file;
		free_str(file);
		MessageBox(NULL, tempstr.c_str(), "Load error", MB_ICONERROR | MB_OK);
		goto lb_exit;
	}
	free_str(file);

	//tempstr.setdata(ini_alloc_get_string(ini_node, "General", "redir_to", "c:\\windows\\system32\\calc.exe"));
	options = ini_get_long(ini_node, "General", "options", RR_SHOW_CMDLN | RR_SHOW_FAILED);

	i = 1;
	while(1)
	{
		tempstr = "redir_to_";
		tempstr += i++;
		cmd = ini_alloc_get_string(ini_node, "General", tempstr.c_str(), NULL);
		if(isempty(cmd))
		{
			break;
		}

		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		replace_variable(&cmd, param_list);

		if(*cmd == '\"')
		{
			end = _skipstring(cmd);
			if(end)
			{
				file = alloc_strncpy(cmd + 1, end - 1);
			}
		}
		else
		{
			file = strchr(cmd, ' ');
			if(file)
			{
				file = alloc_strncpy(cmd, file - cmd);
			}
			else
			{
				file = alloc_string(cmd);
			}
		}

		/*
		if(strchr(lpredir, ' '))
		{
			tempstr = "\"";
			tempstr += lpredir;
			tempstr += "\"";
		}
		else
		{
			tempstr = lpredir;
		}

		if(lpCmdLine)
		{
			tempstr += " ";
			tempstr += lpCmdLine;
		}
		*/

		if(options & RR_SHOW_CMDLN)
		{
			//MessageBox(NULL, tempstr.c_str(), "Information", MB_ICONINFORMATION | MB_OK);
			tempstr = "Will be executed:\nFile: ";
			tempstr += file;
			tempstr += "\nParamsLine: ";
			tempstr += cmd;
			MessageBox(NULL, tempstr.c_str(), "rerun (build "__BUILD_STR__"): Information", MB_ICONINFORMATION | MB_OK);
		}

		//if(CreateProcess((options & RR_FULL_CMDLN)?NULL:lpredir, (options & RR_FULL_CMDLN)?tempstr.c_str():lpCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		//if(!CreateProcess(lpredir, tempstr.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		end = CreateProcess(trim(file), trim(cmd), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

		free_str(file);
		free_str(cmd);

		if(!end)
		{
			exit_code = -1;
			if(options & RR_SHOW_FAILED)
			{
				tempstr = "GetLastError return code: ";
				tempstr += (int) GetLastError();
				MessageBox(NULL, tempstr.c_str(), "rerun (build "__BUILD_STR__"): Execution error", MB_ICONERROR | MB_OK);
			}
			if(options & RR_BREAK_ON_ERROR)
			{
				break;
			}
		}
		else
		{
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

lb_exit:
	ini_free(&ini_node);

	i = 0;
	while(param_list[i].szKey)
	{
		free_str(param_list[i].szValue);
		i++;
	}

	return exit_code;
}

