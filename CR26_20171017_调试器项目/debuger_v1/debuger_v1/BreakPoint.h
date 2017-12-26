#pragma once

typedef struct _BREAK_POINT /**保存断点的信息**/
{
	BYTE   Type;       // 断点类型[01:软件断点/02:硬件断点/03:内存断点]
	LPVOID lpAddress;  // 断点所在地址
	BOOL   bIsOnlyOne; // 是否为一次性断点
	BOOL   bActinve;   // 是否激活
	BOOL   PointLen;   // 断点长度
	BOOL   bRecover;
	DWORD  OldAttrbuit;
	union TYPE_BACKUP
	{
		DWORD BreakTyep;	// 中断类型（限硬件、内存断点）	
		BYTE  Backup;		// 保存设置断点前的数据（限软件断点）	
		DWORD dwOldProtect; // 断点处的原有保护属性
	}TB;

}BREAK_POINT, *PBREAK_POINT, TEMP_BREAK_POINT, *PTEMP_BREAK_POINT;


typedef struct tagM_BREAK_POINT
{
	LPVOID dwAddr;
	struct PAGE
	{
		DWORD dwBaseAddr;
		DWORD dwSize;
		DWORD dwProtect;
	}page;
	WORD  wSize;
	WORD  wState;
	BOOL   bIsOnlyOne; // 是否为一次性断点
	BOOL   bActive;   // 是否激活
	BOOL   bRecover;
}M_BREAK_POINT;

#define BM_WRITE    0x0001
#define BM_ENABLE   0x0002
#define BM_REPAIR   0x0004

enum BREAK_TYPE /** 中断类型 **/
{
	BREAK_TYPE_EXE,  // 执行断点
	BREAK_TYPE_WIT,  // 写入断点
	BREAK_TYPE_I_O,  // I/O读写中断
	BREAK_TYPE_RED   // 读取断点
};

enum BP_TYPE    /** 断点类型 **/
{
	BP_TYPE_INT3 = 1,  // 软件断点
	BP_TYPE_HARD = 2,  // 硬件断点
	BP_TYPE_MEM = 3,  // 内存断点
	BP_TYPE_STEP = 4   // 单步断点
};

/************************************************************************/
/* DR7寄存器的位结构                                                    */
/************************************************************************/
typedef struct _DBG_REG7
{
	/*
	// 局部断点(L0~3)与全局断点(G0~3)的标记位
	*/
	unsigned L0 : 1;  // 对Dr0保存的地址启用 局部断点
	unsigned G0 : 1;  // 对Dr0保存的地址启用 全局断点
	unsigned L1 : 1;  // 对Dr1保存的地址启用 局部断点
	unsigned G1 : 1;  // 对Dr1保存的地址启用 全局断点
	unsigned L2 : 1;  // 对Dr2保存的地址启用 局部断点
	unsigned G2 : 1;  // 对Dr2保存的地址启用 全局断点
	unsigned L3 : 1;  // 对Dr3保存的地址启用 局部断点
	unsigned G3 : 1;  // 对Dr3保存的地址启用 全局断点
	/*
	// 【以弃用】用于降低CPU频率，以方便准确检测断点异常
	*/
	unsigned LE : 1;
	unsigned GE : 1;
	/*
	// 保留字段
	*/
	unsigned Reserve1 : 3;
	/*
	// 保护调试寄存器标志位，如果此位为1，则有指令修改条是寄存器时会触发异常
	*/
	unsigned GD : 1;
	/*
	// 保留字段
	*/
	unsigned Reserve2 : 2;
	/*
	// 保存Dr0~Dr3地址所指向位置的断点类型(RW0~3)与断点长度(LEN0~3)，状态描述如下：
	// 断点类型(RW0~3)：
	//     00 : 执行断点，执行到指定地址时中断
	//     01 : 写入断点，向指定地址写入数据时中断
	//     10 : I/O读写中断，目前CPU不支持
	//     11 : 读取断点，在指定地址读取数据时中断
	// 断点长度(LEN0~3)：
	//     00 : 1字节
	//     01 : 2字节
	//     10 : 8字节
	//     11 : 4字节
	*/
	unsigned RW0 : 2;  // 设定Dr0指向地址的断点类型
	unsigned LEN0 : 2;  // 设定Dr0指向地址的断点长度
	unsigned RW1 : 2;  // 设定Dr1指向地址的断点类型
	unsigned LEN1 : 2;  // 设定Dr1指向地址的断点长度
	unsigned RW2 : 2;  // 设定Dr2指向地址的断点类型
	unsigned LEN2 : 2;  // 设定Dr2指向地址的断点长度
	unsigned RW3 : 2;  // 设定Dr3指向地址的断点类型
	unsigned LEN3 : 2;  // 设定Dr3指向地址的断点长度
}DBG_REG7, *PDBG_REG7;


/************************************************************************/
/* DR6寄存器的位结构														*/
/************************************************************************/
typedef struct _DBG_REG6
{
	/*
	//     断点命中标志位，如果位于DR0~3的某个断点被命中，则进行异常处理前，对应
	// 的B0~3就会被置为1。
	*/
	unsigned B0 : 1;  // Dr0断点触发置位
	unsigned B1 : 1;  // Dr1断点触发置位
	unsigned B2 : 1;  // Dr2断点触发置位
	unsigned B3 : 1;  // Dr3断点触发置位
	/*
	// 保留字段
	*/
	unsigned Reserve1 : 9;
	/*
	// 其它状态字段
	*/
	unsigned BD : 1;  // 调制寄存器本身触发断点后，此位被置为1
	unsigned BS : 1;  // 单步异常被触发，需要与标志寄存器(EFLAGS)的TF联合使用
	unsigned BT : 1;  // 此标记与TSS的T标志联合使用，用于接收CPU任务切换异常
	/*
	// 保留字段
	*/
	unsigned Reserve2 : 16;
}DBG_REG6, *PDBG_REG6;


/************************************************************************/
/* EFLAGS寄存器的位结构                                                 */
/************************************************************************/
//标志寄存器
typedef struct _tagEFlags
{
	DWORD dwCF : 1;  //32    0
	DWORD UnUse3 : 1;  //31    1
	DWORD dwPF : 1;  //30    2
	DWORD UnUse2 : 1;  //29    3
	DWORD dwAF : 1;  //28    4
	DWORD UnUse1 : 1;  //27    5
	DWORD dwZF : 1;  //26    6
	DWORD dwSF : 1;  //25    7
	DWORD dwTF : 1;  //24    8
	DWORD dwIF : 1;  //23    9
	DWORD dwDF : 1;  //22    10
	DWORD dwOF : 1;  //21    11
	DWORD UnUse : 20; //20    12

}EFLAGS, *PEFLAGS;