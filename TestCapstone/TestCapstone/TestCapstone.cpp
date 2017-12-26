// TestCapstone.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "stdio.h"
#include <inttypes.h>

#include "capstone.h"

#pragma comment(lib, "capstone.lib")


#define CODE "\x55\x48\x8b\x05\xb8\x13\x00\x00"

int _tmain(int argc, _TCHAR* argv[])
{
	csh handle;
	cs_insn * insn;
	size_t count;

	if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK){
		return - 1;
	}

	size_t j;
	count = cs_disasm(handle, (const uint8_t *)CODE, sizeof(CODE) - 1, 0x1000, 0, &insn);
	for (j = 0; j < count; j++){
		printf("0x%"PRIx64":\t%s\t\t%s\n", insn[j].address, insn[j].mnemonic, insn[j].op_str);
	}

	cs_free(insn, count);

	return 0;
}

