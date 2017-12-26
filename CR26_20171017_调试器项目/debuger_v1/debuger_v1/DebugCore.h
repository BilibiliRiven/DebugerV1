#pragma once
#include "capstone.h"
#include "BpManager.h"
#include <tlhelp32.h>
#include <string>
#include <vector>
using namespace std;
/*
*  CDebugCore 是调试器核心功能类。
*  提供调试器的核心API调用，也就是
*  MCV中的M。
*/
extern BYTE _int3;

typedef struct tag_MODULE_INFO{
	wstring ModName;
	HANDLE hMod;
	DWORD dwModSize;
}MODULE_INFO, *PMODULE_INFO;

class CDebugCore
{
	friend CBpManager;
	enum DBGP_STATUS
	{
		DBGP_SUSPEND,
		DBGP_RUN,
		DBGP_EXIT,
		DBGP_NONE
	} m_DbgProcStatus;
	DBGP_STATUS DbgProcessRun(){ return m_DbgProcStatus = DBGP_RUN; }
	DBGP_STATUS DbgProcessSuspend(){ return m_DbgProcStatus = DBGP_SUSPEND; }
	csh		m_handle;														
	cs_insn * m_pinsn;														// capston反汇编引擎提供的反汇编类	
	UINT	m_count;														// 这个用来计算发挥变代码的条数

public:
	PROCESS_INFORMATION	m_stProcessInfo;									// 存放被调试进程的相关信息
	CString m_cstrFileName;													// 存放被调试进程的文件名
	DEBUG_EVENT m_stDbgEvent;												// 存放被调试进程进程，产生的调试事件
	HANDLE	m_WaitCommand;													// 一个事件信号量，用于阻塞，等待命令行输入
	BOOL	m_bSystemException;												// 判断是否是系统断点
	BOOL	m_bStepHandle;													// 判断是否是人手工造成的单步
	BOOL	m_bStepInto;													// 判断是否有单步
	TEMP_BREAK_POINT m_TempBreakPoint;										// 临时的断点
	CList<BREAK_POINT, BREAK_POINT&> m_lstBp;								// 断点表
	CList<M_BREAK_POINT, M_BREAK_POINT &> m_lstMBp;							// 内存断点表
	CArray<MODULEENTRY32, MODULEENTRY32> m_aryMod;							// 模块息表
	CArray<MEMORY_BASIC_INFORMATION, MEMORY_BASIC_INFORMATION> m_aryMem;    // 内存基本信息表
	BREAK_POINT m_lstHbp[4];												// 硬件断点表
	BOOL  m_IsRecoverHBP;													// 是否需要恢复硬件断点
	int   m_nrDReg;															// 保存需要恢复的寄存器编号
	BOOL  m_IsRecoverBp;													// 是否需要恢复断点
	BOOL  m_IsRecoverMBp;													// 是否需要恢复内存断点
	BOOL  m_IsGo;															// 恢复断点后是否应该直接跑起来
	vector<MODULE_INFO> m_ModList;
public:
	CONTEXT m_stRegContext;													// 这里存放寄存器的信息
																			// 调DbgUpdateRegContext可以更新寄存器
																			// 信息
	/*
	*  功能：在这个函数主要完成初始化工作
	*/
	CDebugCore();
	/*
	*  功能：在这个函数主要完成资源的释放工作
	*/
	~CDebugCore();
	/*
	*	功能：以调试方式运行PE文件，返回线程句柄返回值为0时，函数失败
	*   Parameter1：文件路径(包含文件名)
	*	Return Value：文件执行成功返回TRUE，失败返回FALSE
	*/
	BOOL DbgPeFile(CString & rstrFilePath);
	/*
	*	功能：手工产生一个单步歩过的命令
	*	Return Value: 命令执行成功返回TRUE, 命令执行失败返回FALSE
	*/
	BOOL DbgSetpOver();
	/*
	*  功能：手工产生一个单步歩入的命令
	*  Return Value: 命令执行成功返回TRUE, 命令执行失败返回FALSE
	*/
	BOOL DbgSetpInto();
	/*
	*  功能：非手工设置一个单步步入断点，CPU执行一次指令，通常用于断点恢复
	*  Return Value: 命令执行成功返回TRUE, 命令执行失败返回FALSE
	*/
	BOOL DbgSetStep();
	/*
	*  功能：这个函数功能没有实现
	*/
	int DbgBPList();

private:
	/*
	*  功能：线程回调函数，循环接收调试事件并处理调试事件
	*  参数：当前类对象的指针
	*/
	static DWORD ThreadDebugEventProc(LPVOID pThis);
	/*
	*  下面一组函数是会调试事件的处理函数
	*/
	int OnExceptionDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnCreateThreadDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnCreateProcessDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnExitProcessDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnExitThreadDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnLoadDllDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnUnloadDllDebugEvent(DEBUG_EVENT * pDebugEvent);
	int OnOutputDebugStringEvent(DEBUG_EVENT * pDebugEvent);
	int OnRipEvent(DEBUG_EVENT * pDebugEvent);
public:
	/*
	*  下面3个函数，是对异常调试事件的非类处理函数
	*/
	int OnExceptionBreakPoint(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo);
	int OnExceptionAccessViolation(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo);
	int OnExceptionSingleStep(LPEXCEPTION_DEBUG_INFO pExceptionDebugInfo);
	/*
	*   功能：提供反汇编的相关信息。
	*  参数1：是需要返汇编内容的首地址
	*  参数2：是反汇编多少个字节
	*  参数3：传出参数，成功反汇编的指令条数
	*  参数4：用来CapStone提供结构体，用来获取反汇编的信息
	*  返回值：函数执行失败返回值是0，成功返回值非零
	*/
	int DbgGetAsm(LPVOID lpvAddr, UINT uiBuffSize = 256, UINT  * piCount = 0, const cs_insn ** pinsn = 0);
	/*
	*	功能：检查被调试程序是否挂起
	*	返回值：TURE被调试程序挂起，可以进行操作，FLASE不能进行操作
	*/
	BOOL DbgIsSuspend(){ return m_DbgProcStatus == DBGP_SUSPEND ? TRUE : FALSE; }
	/*
	*	功能：操作失败的提示信息，主要用于调试
	*/
	void DbgWarningProcRun(){ printf("操作失败,调试程序正在运行\r\n"); }
private:
	/*
	*	功能：更新寄存器信息
	*	参数为TRUE时，将寄存器信息更新到m_stRegContext结构体中
	*	参数为FLASE时，将结构体的内容，更细到CPU寄存器中
	*/
	BOOL DbgUpdateRegContext(BOOL bUpdate);
public:
	/*
	*	功能：改写内存中的数据
	*	Parameter1：lpvAddr 修改位置的首地址
	*	Parameter2：pBuff 新内容的缓冲区首地址
	*	Parameter3：uiBuffSize 新内容缓冲区大小
	*	Return Value：函数功能执行成功返回TRUE，失败返回FALSE
	*/
	BOOL DbgWriteMemory(LPVOID lpvAddr, BYTE * pBuff, UINT uiBuffSize, DWORD * pReadBytes = NULL);
	/*
	*	功能：读内存中的数据
	*	Parameter1：lpvAddr 读取位置的首地址
	*	Parameter2：pBuff 缓冲区首地址
	*	Parameter3：uiBuffSize 缓冲区大小
	*	Return Value：函数功能执行成功返回TRUE，失败返回FALSE
	*/
	BOOL DbgReadMemory(LPVOID lpvAddr, BYTE * pBuff, UINT uiBuffSize, DWORD * pReadBytes = NULL);
	/*
	*	功能：向指定的内存位置写入CC
	*	参数1：指定位置的内存地址
	*	返回值：成功返回非零值，失败返回0值
	*/
	BOOL SetInt3(LPVOID lpvAddr);
	/*
	*	功能：向指定的内存位置设置断点
	*	参数1：指定位置的内存地址
	*	参数2：传入传出参数，返回断点的信息
	*	返回值：成功返回非零值，失败返回0值
	*/
	BOOL SetInt3(LPVOID lpvAddr, BREAK_POINT & bpinfo);
	/*
	*	功能：判断是否int3是调试器断点
	*	Parameter1：lpvAddr 断点地址
	*	Parameter2：bRet 参数是TURE，从m_lstBp删除断点并且恢复覆盖，参数为假只恢复覆盖，不删除断点
	*	Return Value：是返回断点位置，不是就返回NULL
	*/
	POSITION DbgIsDbgInt3(LPVOID lpvAddr);
	/*
	*	功能：删除Int3断点
	*	参数1：断点在断点列表中的位置
	*	参数2：TRUE是真删除，将其从断点列表中完全删除。FALSE假删除，将其从临时删除，并没有从断点列表中删除
	*	返回值：成功返回TURE，失败返回FALSE
	*/
	BOOL DbgDelInt3BP(POSITION pos, BOOL bDel);
	/*
	*	功能：设置一个Int3断点，根据同时设置断点类型
	*	参数1：int3断点的位置
	*	参数2：是否为临时断点
	*/
	BOOL DbgSetInt3BreakPoint(LPVOID lpvAddr, BOOL bOnceTime);
	/*
	*	功能：设置一个Int3临时断点，根据同时设置断点类型
	*	参数1：int3断点的位置
	*	参数2：是否为临时断点
	*	返回值：成功返回TRUE，失败返回FALSE
	*/
	BOOL DbgSetTempBreakPoint(LPVOID lpvAddr);
	/*
	*	功能：删除一个Int3临时断点
	*	返回值：成功返回TURE，失败返回FASE
	*/
	BOOL DbgDelTempBreakPoint();
	/*
	*	功能：手动设置，让被调试程序运行起来
	*	返回值：成功返回TRUE，失败返回FALSE
	*/
	BOOL DbgRun();
	/*
	*	功能：设置硬件断点
	*	参数1：硬件断点地址
	*	参数2：硬件断点的长度
	*	参数3：硬件断点的长度
	*/
	BOOL DbgSetHBP(LPVOID lpvAddr, int iType, UINT uiLen);
	/*
	*	功能：获得剩余未使用的硬件断点寄存器
	*	return value: 返回0 1 2 3 对应可以使用的HBP寄存器，当返回-1表示无寄存器可以使用。
	*/
	int DbgGetHbpReg();
private:
	/*
	*	功能：扫描硬件断点表，将需要恢复的硬件断点恢复
	*	返回值：恢复成功返回TURE，失败返回FALSE
	*/
	BOOL DbgRecoverHBP();
public:
	/*
	*	功能：扫描INT3断点表，将需要恢复的硬件断点恢复
	*	返回值：恢复成功返回TURE，失败返回FALSE
	*/
	BOOL DbgRecoverBP();
	/*
	*	返回值：恢复成功返回TURE，失败返回FALSE
	*	功能：扫描内存断点表，将需要恢复的内存断点恢复
	*/
	BOOL DbgSetMemBP(LPVOID lpvAddr, UINT uLen, BYTE btType);
	/*
	*	功能：检查地址的合法性
	*	返回值：合法功返回TURE，失败返回FALSE
	*/
	BOOL DbgIsInvalidAddr(LPVOID lpvAddr);
	/*
	*	功能：检查断点是否发生覆盖
	*	返回值：合法功返回TURE，失败返回FALSE
	*/
	BOOL DbgIsMemBpCovered(LPVOID lpvAddr, UINT uLen);
	/*
	*	功能：更新内存信息
	*	返回值：
	*/
	BOOL DbgUpdateMem();
	/*
	*   功能：遍历内存断点表，检查地址是否落在内存断点对应的分页内。
	*   参数1：被检查的地址
	*	参数2：bActive为真，只检查激活的断点。
	*   返回值：如果地址并没有落在断点内，返回FLASE，否则返回TURE。同时通过指针，
	*	传出内存断点原来的分页信息
	*/
	int DbgFindBmPage(LPVOID lpvAddr, BOOL bActive, M_BREAK_POINT::PAGE * pPage);
	/*
	*   功能：遍历内存断点表，检查地址是否落在内存断点对应的分页内。
	*   参数1：被检查的地址
	*	参数2：bActive为真，只检查激活的断点。
	*   返回值：如果地址并没有落在断点内，返回NUL，否则返回断点
	*	在断点表中的位置。同时通过指针，传出内存断点的结构体信息
	*/
	POSITION DbgFindBmInPageEX(LPVOID lpvAddr, bool bActive, M_BREAK_POINT::PAGE * pPage = NULL);
	/*
	*   功能：检查地址lpvAddr是否落在内存断点的范围内。
	*	参数1：被检查的地址
	*   参数2：bActive为TRUE要求断点被激活，bActive为FLASE要求断点
	*   返回值：断点在断点表中的位置，失败返回NULL
	*/
	POSITION DbgFindMemBp(LPVOID lpvAdd, BOOL bActive);
	/*
	*   功能：恢复内存断点
	*   返回值：函数执行成功返回TRUE，函数执行失败返回FALSE
	*/
	BOOL DbgRecoverMBp();
	/*
	*   功能：恢复内存断点
	*   返回值：函数执行成功返回TRUE，函数执行失败返回FALSE
	*/
	void DbgUpdateBmPage();
	/*
	*	功能：遍历内存信息，检查制定地址是否在内存分页中
	*   参数1：查询的地址
	*	参数2：传入传出参数，查询地址所在分页的分页信息
	*/
	BOOL CDebugCore::DbgFindPage(LPVOID lpvAddr, MEMORY_BASIC_INFORMATION * pMbi);
	/*
	*	功能：过得寄存器的信息
	*   参数1：传入传出参数，用来传出寄存器的信息
	*	返回值：函数执行成功返回TURE，执行失败返回FALSE
	*/
	BOOL DbgGetRegInfo(CONTEXT * pContext);
	/*
	*	功能：获得FS有关寄存器的信息
	*   返回值：FS寄存器值
	*/
	DWORD DbgGetFs();
	/*
	*   功能：获得断点的基本信息
	*	参数1：传入传出参数，指向一个指针数组，用来存放断点的地址
	*   参数2：传入传出参数，用来计算断点
	*   成功：返回TURE，失败返回FALSE
	*/
	BOOL DbgGetBpInfo(LPVOID * pAddr, DWORD * pdwCount);
	/*
	*	功能：根据断点编号，删除断点
	*	参数1：断点编号
	*/
	BOOL DbgDelBp(int k);
	/*
	*	功能：根据断点编号，删除内存断点
	*	参数1：断点编号
	*/
	BOOL DbgDelBm(int k);
	/*
	*	功能：根据断点编号，删除硬件断点
	*	参数1：断点编号
	*/
	BOOL DbgDelBh(int k);
	/*
	*	功能：跟新模块信息
	*	参数1：获得模块快照
	*/
	BOOL UpdateModule(int k);
};

