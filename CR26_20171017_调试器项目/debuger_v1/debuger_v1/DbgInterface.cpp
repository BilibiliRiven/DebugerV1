#include "stdafx.h"
#include "DbgInterface.h"
#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <vector>

/*
*	�����ڲ�ʹ�õĺ���û������
*	���ܣ���������в���1���ĵ�ַ
*	����ֵ��ʧ�ܷ���-1���ɹ����� �����ĵ�ַ
*/
UINT GetAddress1(char * psz){
	//��ȡ����Ͳ�����
	std::string CmdStr = psz;
	std::vector<std::string> vecSegTag;

	boost::split(vecSegTag, CmdStr, boost::is_any_of(" "));

	//��ȡ������
	char * pTemp = NULL;
	return vecSegTag.size()>1? strtoul(vecSegTag[1].c_str(), &pTemp, 16): -1;
}


int GetNum1(char * psz){
	//��ȡ����Ͳ�����
	std::string CmdStr = psz;
	std::vector<std::string> vecSegTag;

	boost::split(vecSegTag, CmdStr, boost::is_any_of(" "));

	//��ȡ������
	char * pTemp = NULL;
	return vecSegTag.size() > 1 ? strtoul(vecSegTag[1].c_str(), &pTemp, 10) : -1;
}


UINT GetLen(char * psz){
	//��ȡ����Ͳ�����
	std::string CmdStr = psz;
	std::vector<std::string> vecSegTag;

	boost::split(vecSegTag, CmdStr, boost::is_any_of(" "));

	//��ȡ������
	char * pTemp = NULL;
	return strtoul(vecSegTag[2].c_str(), &pTemp, 10);
}


UINT GetType(char * psz){
	//��ȡ����Ͳ�����
	std::string CmdStr = psz;
	std::vector<std::string> vecSegTag;

	boost::split(vecSegTag, CmdStr, boost::is_any_of(" "));

	//��ȡ������
	return vecSegTag[3] == "w" ? BM_WRITE : 0;
}


UINT GetType2(char * psz){
	//��ȡ����Ͳ�����
	std::string CmdStr = psz;
	std::vector<std::string> vecSegTag;

	boost::split(vecSegTag, CmdStr, boost::is_any_of(" "));

	if (vecSegTag[3] == "e" || vecSegTag[3] == "E"){
		return BREAK_TYPE_EXE;
	}
	else if (vecSegTag[3] == "w" || vecSegTag[3] == "W"){
		return BREAK_TYPE_WIT;
	}
	else{
		return BREAK_TYPE_RED;
	} 
}


BOOL StepInto(char *){
	return g_pDbg!=NULL?g_pDbg->DbgSetpInto() : FALSE;
}


BOOL StepOver(char *){
	return g_pDbg != NULL ? g_pDbg->DbgSetpOver() : FALSE;
}


BOOL Run(char * psz){
	int uiAddr = GetAddress1(psz);
	if (uiAddr>0){
		g_pDbg->DbgSetTempBreakPoint((LPVOID)uiAddr);
	}
	return g_pDbg != NULL ? g_pDbg->DbgRun() : FALSE;
}


BOOL CheckRegister(char *){
	if (!g_pDbg){
		return FALSE;
	}
	CONTEXT text;
	BOOL bRet = g_pDbg->DbgGetRegInfo(&text);
	if (!bRet){
		return FALSE;
	}
	// EAX=00000000   EBX=00000000   ECX=B2A10000   EDX=0008E3C8   ESI=FFFFFFFE
	// EDI=00000000   EIP=7703103C   ESP=0018FB08   EBP=0018FB34   DS =0000002B
	// ES =0000002B   SS =0000002B   FS =00000053   GS =0000002B   CS =00000023
	//��ȡEFlags
	EFLAGS& eFlags = *(PEFLAGS)&text.EFlags;


	_tprintf(TEXT("EAX=%08X   EBX=%08X   ECX=%08X   EDX=%08X   ESI=%08X\r\n"),
		text.Eax,
		text.Ebx,
		text.Ecx,
		text.Edx,
		text.Esi);

	_tprintf(TEXT("EDI=%08X   EIP=%08X   ESP=%08X   EBP=%08X   DS =%08X\r\n"),
		text.Edi,
		text.Eip,
		text.Esp,
		text.Ebp,
		text.SegDs);

	_tprintf(TEXT("ES =%08X   SS =%08X   FS =%08X   GS =%08X   CS =%08X\r\n"),
		text.SegEs,
		text.SegSs,
		text.SegFs,
		text.SegGs,
		text.SegCs);

	_tprintf(TEXT("OF   DF   IF   TF   SF   ZF   AF   PF   CF\r\n"));

	_tprintf(TEXT("%02d   %02d   %02d   %02d   %02d   %02d   %02d   %02d   %02d\r\n\r\n"),
		eFlags.dwOF,
		eFlags.dwDF,
		eFlags.dwIF,
		eFlags.dwTF,
		eFlags.dwSF,
		eFlags.dwZF,
		eFlags.dwAF,
		eFlags.dwPF,
		eFlags.dwCF);

	return TRUE;
	return TRUE;
}


#define RemoteOneReadSize 0x60  //һ�ζ�ȡԶ�����ݵĳ���
BOOL CheckMemory(char * psz){
	static UINT dwAddr = 0x400000;
	int iAddr = GetAddress1(psz);
	if (iAddr > 0){
		dwAddr = iAddr;
	}
	DWORD dwRead = 0;
	UCHAR szBuf[RemoteOneReadSize] = { 0 };
	PUCHAR pszBuf = szBuf;

	//��ȡԶ���ڴ���Ϣ
	if (!g_pDbg->DbgReadMemory((LPVOID)dwAddr, szBuf, RemoteOneReadSize, &dwRead))
	{
		printf("��Ч�ڴ��ַ\r\n");
		OutputDebugString(TEXT("ShowRemoteDisAsm����ȡԶ���ڴ�ʧ�ܣ�"));
		return FALSE;
	}

	//����ڴ���Ϣ
	int nCount = dwRead / 0X10;
	for (int i = 0; i < nCount; i++)
	{
		//�����ַ
		_tprintf(TEXT("%08X   "), dwAddr);
		//tcout << ios::hex << dwAddr << TEXT("    ");

		//���ʮ������ֵ
		for (int j = 0; j < 0x10; j++)
		{
			_tprintf(TEXT("%02X "), pszBuf[j]);
			//tcout << ios::hex << pszBuf[j] << TEXT(' ');
		}

		tcout << TEXT("  ");

		//��������ַ���
		for (int n = 0; n < 0x10; n++)
		{
			if (pszBuf[n] < 32 || pszBuf[n] > 127)
				_puttchar(TEXT('.'));
			else
				_puttchar(pszBuf[n]);
		}

		//���س�����
		tcout << endl;

		dwAddr += 0x10;
		pszBuf += 0x10;
	}
	return TRUE;
}


BOOL CheckDisasm(char *){
	return FALSE;
}


BOOL Quite(char * psz){
	ExitProcess(0);
}


BOOL SetBp(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	UINT dwAddr = GetAddress1(psz);
	return g_pDbg->DbgSetInt3BreakPoint((LPVOID)dwAddr, FALSE);
}


BOOL ListBp(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	DWORD dwCount;
	g_pDbg->DbgGetBpInfo(NULL, &dwCount);
	LPVOID * pList = new LPVOID[dwCount];
	g_pDbg->DbgGetBpInfo(pList, &dwCount);
	printf("=====================һ��ϵ�=====================\r\n");
	if (dwCount == 0)
	{
		tcout << TEXT("����") << endl;
	}
	else
	{
		for (int i = 0; i < dwCount; ++i)
		{

			printf("\t\t��ţ�%d\t��ַ��0x%p\r\n", i, pList[i]);
		}
	}

	printf("=====================һ��ϵ�=====================\r\n");
	return TRUE;
}


BOOL Bpc(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	int k = GetNum1(psz);
	return k >= 0 ? g_pDbg->DbgDelBp(k) : FALSE;
}


BOOL SetBm(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	int dwAddr = GetAddress1(psz);
	DWORD dwLen = GetLen(psz);
	DWORD dwType = GetType(psz);

	if (dwAddr < 0){
		return FALSE;
	}
	if (dwLen < 0){
		return FALSE;
	}


	return g_pDbg->DbgSetMemBP((LPVOID)dwAddr, dwLen, dwType);
}


BOOL ListBm(char * psz){
	if (!g_pDbg){
		return FALSE;
	}

	printf("=====================�ڴ�ϵ�=====================\r\n");
	POSITION pos = g_pDbg->m_lstMBp.GetHeadPosition();

	int i = 0;
	while (pos){
		M_BREAK_POINT & bp = g_pDbg->m_lstMBp.GetNext(pos);
		printf("\t\t��ţ�%d\t��ַ��0x%p\t ����%d\t ����%s\r\n", i, bp.dwAddr, bp.wSize, bp.wState == BM_WRITE?"w":"r");
		++i;
	}
	printf("=====================�ڴ�ϵ�=====================\r\n");
	return TRUE;
}


BOOL Bmc(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	int k = GetNum1(psz);
	return k >= 0 ? g_pDbg->DbgDelBm(k) : FALSE;
}


BOOL SetBh(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	UINT dwAddr = GetAddress1(psz);
	DWORD dwLen = GetLen(psz);
	DWORD dwType = GetType2(psz);

	return g_pDbg->DbgSetHBP((LPVOID)dwAddr, dwType, dwLen);
}


BOOL ListBh(char * psz){
	if (!g_pDbg){
		return FALSE;
	}

	printf("=====================Ӳ���ϵ�=====================\r\n");
	for (int i = 0; i < 4; ++i){
		printf("\t\t��ţ�%d\t��ַ��0x%p\t ����%d\t ����%s\r\n", i, g_pDbg->m_lstHbp[i].lpAddress, g_pDbg->m_lstHbp[i].PointLen, g_pDbg->m_lstHbp[i].TB.BreakTyep == BREAK_TYPE_EXE ? "e" : (g_pDbg->m_lstHbp[i].TB.BreakTyep == BREAK_TYPE_WIT ? "w" : "r"));
	}
	printf("=====================Ӳ���ϵ�=====================\r\n");
	return TRUE;
}


BOOL Bhc(char * psz){
	if (!g_pDbg){
		return FALSE;
	}
	int k = GetNum1(psz);
	return k>=0?g_pDbg->DbgDelBh(k):FALSE;
}


BOOL Es(char * psz){
	FILE * pf = fopen("DebugEs.mydbg","b+");
	for (int i = 0; i < g_cmdline_record.size(); ++i){
		printf("%s\r\n", g_cmdline_record[i].c_str());

	}
	return TRUE;
}


BOOL Ls(char * psz){
	return TRUE;
}

BOOL Module(char * psz){
	return TRUE;
}

BOOL Memory(char * psz){
	return TRUE;
}

CMD_FUNC_NODE Options[] = {
	CMD_ENTRY("t", StepInto)
	, CMD_ENTRY("p", StepOver)
	, CMD_ENTRY("g", Run)
	, CMD_ENTRY("r", CheckRegister)
	, CMD_ENTRY("d", CheckMemory)
	, CMD_ENTRY("q", Quite)
	, CMD_ENTRY("bp", SetBp)
	, CMD_ENTRY("bpl", ListBp)
	, CMD_ENTRY("bpc", Bpc)
	, CMD_ENTRY("bm", SetBm)
	, CMD_ENTRY("bml", ListBm)
	, CMD_ENTRY("bmc", Bmc)
	, CMD_ENTRY("bh", SetBh)
	, CMD_ENTRY("bhl", ListBh)
	, CMD_ENTRY("bhc", Bhc)
	, CMD_ENTRY("es", Es)
	, CMD_ENTRY("ls", Ls)
	, CMD_ENTRY("mod", Module)
	, CMD_ENTRY("mem", Memory)
};


int CallFunc(char * psz){
	//��ȡ����Ͳ�����
	CStringA CmdStr = psz;
	int nPos = CmdStr.Find(TEXT(' '));

	//��ȡ����
	CStringA strOrder;
	if (nPos > 0){
		strOrder = CmdStr.Left(nPos);
	}
	else{
		strOrder = CmdStr;
	}
	for (int i = 0; i < sizeof(Options) / sizeof(Options[0]); ++i){


		if (strOrder == Options[i].m_Option){
			if (!Options[i].m_CallBack(psz)){
				printf("����ִ��ʧ�ܣ��������\r\n");
				return FALSE;
			}
			return TRUE;
		}
	}
	printf("�������\r\n");
	return FALSE;
}


std::vector<std::string> g_cmdline_record;


void GetCmdLine(){
	char sz[256];
	while (1){
		printf("<");
		gets_s(sz, 256);
		BOOL bRet = CallFunc(sz);
		if (bRet == TRUE){
			g_cmdline_record.push_back(sz);
		}
	}
}




