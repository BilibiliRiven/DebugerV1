// debuger_v1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "debuger_v1.h"
#include "DebugCore.h"

#include <windows.h>
#include <stdio.h>
#include "DbgInterface.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDebugCore * g_pDbg;

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

void Dbgmain();
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  更改错误代码以符合您的需要
			_tprintf(_T("错误:  MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  在此处为应用程序的行为编写代码。
			// 用Ctrl+C可以暂停程序
			SetConsoleCtrlHandler(HandlerRoutine, TRUE);
			Dbgmain();
		}
	}
	else
	{
		// TODO:  更改错误代码以符合您的需要
		_tprintf(_T("错误:  GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}


BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT)
	{
		if (g_pDbg)
		{
			// 不知道这里做什么钱
		}
		return TRUE;
	}
	return FALSE;
}

BOOL GetExcCmd(CString& strCmd);


void cmdline();




// 这里是调试器的入口函数
void Dbgmain(){
	CString strCmd;
	CStringA strcmda;
	do
	{
		printf("Press 1 to Debug\r\n");
		printf("Press 0 to Exit\r\n");

		int nInput = 1;
		scanf("%d", &nInput);

		switch (nInput)
		{
		case 0:
			return;
		}

		GetExcCmd(strCmd);
		if (strCmd.IsEmpty()){
			continue;
		}

		BOOL bRet = 0;
		CDebugCore dbgcore;
		g_pDbg = &dbgcore;

		bRet = g_pDbg->DbgPeFile(strCmd);
		if (bRet){
			GetCmdLine();
		}
		else{
			continue;
		}

		strCmd.SetString(L"");
	} while (1);

	return;
}




// 弹出文件选择框
BOOL GetExcCmd(CString& strCmd)
{


	// Display the Open dialog box. 
	CString filter;
	filter = "可执行(*.exe)";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
	if (dlg.DoModal() == IDOK)
	{
		CString str;
		str = dlg.GetPathName();
		strCmd = str;
	}
	return TRUE;
}




void cmdline(){
	int FunctionCode = -1;
	CString StrCmd;
	while (true){
		
	}
}