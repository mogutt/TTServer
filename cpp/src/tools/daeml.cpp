#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void CloseAll(int fd)
{
	int fd_limit = sysconf(_SC_OPEN_MAX);

	while (fd < fd_limit)
    {
		close(fd++);
    }
}

int Daemon(int no_chdir, int no_close, int root)
{
	switch (fork())
	{
		case 0:
            break;
		case -1:
            return -1;
		default:
            _exit(0);
	}

	if (setsid() < 0)
    {
		return -1;
    }
	if (!root && (setuid(1) < 0))
    {
		return -1;
    }

	switch (fork())
	{
		case 0:
            break;
		case -1:
            return -1;
		default:
            _exit(0);
	}

	if (!no_chdir)
    {
		chdir("/");
    }

	if (!no_close)
	{
		CloseAll(0);
		dup(0);
        dup(0);
	}

	return 0;
}

#define PRINT(a) a
void PrintUsage(char* name)
{
	printf (
		PRINT("\n ----- \n\n")
		PRINT("Usage:\n")
		PRINT("   	%s program_name \n\n")
		PRINT("Where:\n")
		PRINT("   	%s - Name of this Daemon loader.\n")
		PRINT("   	program_name - Name (including path) of the program you want to load as daemon.\n\n")
		PRINT("Example:\n")
		PRINT("   	%s ./atprcmgr - Launch program 'atprcmgr' in current directory as daemon. \n\n\n\n"),
		name, name, name
		);
}

int main(int argc, char* argv[])
{
	printf(
		PRINT("\n")
		PRINT("Daemon loader\n")
		PRINT("Launch specified program as daemon.\n")
		);
    
	if (argc < 2)
	{
		printf("Missing parameter: daemon program name not specified!\n");
		PrintUsage(argv[0]);
		return 0;
	}

	printf("Loading %s as daemon, please wait...\n\n\n", argv[1]);

	if (Daemon(1, 0, 1) >= 0)
	{
		signal(SIGCHLD, SIG_IGN);
		execv(argv[1], argv + 1);
		printf("Excute daemon programm %s failed.\n", argv[1]);
		return 0;
	}

	printf("Create daemon failed. Please check if you have 'root' privilege.\n");
	return 0;
}


