#pragma once
#include "DebugCore.h"
#include <vector>
#include <iostream>
extern CDebugCore * g_pDbg;

#define MAX_STR 32
typedef BOOL (*PCMD_FUNC)(char *);

typedef struct _CMD_FUNC_NODE
{
	char m_Option[MAX_STR];
	PCMD_FUNC m_CallBack;
}CMD_FUNC_NODE;

typedef union _FLAGREG
{
	DWORD dwFlag;
	struct
	{
		DWORD CF : 1;
	DWORD:1;
		DWORD PF : 1;
	DWORD:1;
		DWORD AF : 1;
	DWORD:1;
		DWORD ZF : 1;
		DWORD SF : 1;
		DWORD TF : 1;
		DWORD IF : 1;
		DWORD DF : 1;
		DWORD OF : 1;
	};
}FLAGREG;

using namespace std;
#ifdef _UNICODE
#define tcout wcout
#define tcin  wcin

#else
#define tcout cout
#define tcin  cin

#endif


void GetCmdLine();
extern std::vector<std::string> g_cmdline_record;
#define CMD_ENTRY(szCmd, fun) {szCmd, (PCMD_FUNC)fun}

extern CMD_FUNC_NODE Options[];
