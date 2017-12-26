#ifndef MY_SNAPSHOOT
#define MY_SNAPSHOOT
#include <windows.h>
#include <vector>
#include <string>
#include <assert.h>

using namespace std;








typedef struct _PEB {               // Size: 0x1D8
	UCHAR           InheritedAddressSpace;
	UCHAR           ReadImageFileExecOptions;
	UCHAR           BeingDebugged;              		//Debug运行标志
	UCHAR           SpareBool;
	HANDLE          Mutant;
	HINSTANCE       ImageBaseAddress;           		//程序加载的基地址
	struct _PEB_LDR_DATA    *Ldr;               	//Ptr32 _PEB_LDR_DATA
	DWORD  *ProcessParameters;
	ULONG           SubSystemData;
	HANDLE          DefaultHeap;
	KSPIN_LOCK      FastPebLock;
	ULONG           FastPebLockRoutine;
	ULONG           FastPebUnlockRoutine;
	ULONG           EnvironmentUpdateCount;
	ULONG           KernelCallbackTable;
	LARGE_INTEGER   SystemReserved;
	struct _PEB_FREE_BLOCK  *FreeList;
	ULONG           TlsExpansionCounter;
	ULONG           TlsBitmap;
	LARGE_INTEGER   TlsBitmapBits;
	ULONG           ReadOnlySharedMemoryBase;
	ULONG           ReadOnlySharedMemoryHeap;
	ULONG           ReadOnlyStaticServerData;
	ULONG           AnsiCodePageData;
	ULONG           OemCodePageData;
	ULONG           UnicodeCaseTableData;
	ULONG           NumberOfProcessors;
	LARGE_INTEGER   NtGlobalFlag;              	// Address of a local copy
	LARGE_INTEGER   CriticalSectionTimeout;
	ULONG           HeapSegmentReserve;
	ULONG           HeapSegmentCommit;
	ULONG           HeapDeCommitTotalFreeThreshold;
	ULONG           HeapDeCommitFreeBlockThreshold;
	ULONG           NumberOfHeaps;
	ULONG           MaximumNumberOfHeaps;
	ULONG           ProcessHeaps;
	ULONG           GdiSharedHandleTable;
	ULONG           ProcessStarterHelper;
	ULONG           GdiDCAttributeList;
	KSPIN_LOCK      LoaderLock;
	ULONG           OSMajorVersion;
	ULONG           OSMinorVersion;
	USHORT          OSBuildNumber;
	USHORT          OSCSDVersion;
	ULONG           OSPlatformId;
	ULONG           ImageSubsystem;
	ULONG           ImageSubsystemMajorVersion;
	ULONG           ImageSubsystemMinorVersion;
	ULONG           ImageProcessAffinityMask;
	ULONG           GdiHandleBuffer[0x22];
	ULONG           PostProcessInitRoutine;
	ULONG           TlsExpansionBitmap;
	UCHAR           TlsExpansionBitmapBits[0x80];
	ULONG           SessionId;
} PEB, *PPEB;


typedef struct _PEB_LDR_DATA
{
	ULONG         Length;                             // 00h
	BOOLEAN       Initialized;                        // 04h
	PVOID         SsHandle;                           // 08h

	LIST_ENTRY    InLoadOrderModuleList;      	    // 0ch	
	// 010h

	LIST_ENTRY    InMemoryOrderModuleList;            // 14h
	// 18h

	LIST_ENTRY    InInitializationOrderModuleList;    // 1Ch
	// 20h
}PEB_LDR_DATA, *PPEB_LDR_DATA;





typedef struct _UNICODE_STRING
{
	WORD Len;
	WORD Size;
	char * pUnicode;
}UNICODE_STRING, PUNICODE_STRING;


typedef struct _LDR_MODULE
{
	LIST_ENTRY        InLoadOrderModuleList;            // 00h
	// 004

	LIST_ENTRY        InMemoryOrderModuleList;          // 08h
	// 0Ch

	LIST_ENTRY        InInitializationOrderModuleList;  // 10h      
	// 14h

	PVOID             BaseAddress;                      // 18h      
	PVOID             EntryPoint;                       // 1ch
	ULONG             SizeOfImage;                      // 20h

	UNICODE_STRING    FullDllName;                      // 28h	

	UNICODE_STRING    BaseDllName;                      // 30h	 

	ULONG             Flags;                            // 34h
	SHORT             LoadCount;                        // 38h
	SHORT             TlsIndex;                         // 3ah
	HANDLE            SectionHandle;                    // 3ch
	ULONG             CheckSum;                         // 40h
	ULONG             TimeDateStamp;                    // 44h
	// 48h
} LDR_MODULE, *PLDR_MODULE;

typedef vector<MODULE_INFO> MOD_SNAPSHOOT;
int MyGetSnapshoot(MOD_SNAPSHOOT * pSnapShoot);
int MyNextModule(MOD_SNAPSHOOT * pSnapShoot, int k, MODULE_INFO * pMode);
#endif