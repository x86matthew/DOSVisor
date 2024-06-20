#include "DosVisor.h"

#ifndef _WIN64
#error Must be compiled as 64-bit
#endif

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("DOSVisor\n");
		printf(" - x86matthew\n\n");
		printf("Usage: %s <exe_name>\n", argv[0]);
		return 1;
	}

	// check mode
	if(strcmp(argv[1], "*log-client*") == 0)
	{
		// log-client mode - connect to the named pipe and wait for data
		LogClient();
	}
	else
	{
		// launch hypervisor
		LaunchHypervisor(argv[1]);
	}

	return 0;
}
