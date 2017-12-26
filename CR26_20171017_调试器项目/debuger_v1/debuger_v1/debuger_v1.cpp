// debuger_v1.cpp : �������̨Ӧ�ó������ڵ㡣
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

// Ψһ��Ӧ�ó������

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
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����:  MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  �ڴ˴�ΪӦ�ó������Ϊ��д���롣
			// ��Ctrl+C������ͣ����
			SetConsoleCtrlHandler(HandlerRoutine, TRUE);
			Dbgmain();
		}
	}
	else
	{
		// TODO:  ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����:  GetModuleHandle ʧ��\n"));
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
			// ��֪��������ʲôǮ
		}
		return TRUE;
	}
	return FALSE;
}

BOOL GetExcCmd(CString& strCmd);


void cmdline();




// �����ǵ���������ں���
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




// �����ļ�ѡ���
BOOL GetExcCmd(CString& strCmd)
{


	// Display the Open dialog box. 
	CString filter;
	filter = "��ִ��(*.exe)";
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