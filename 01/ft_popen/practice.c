#include <unistd.h>
#include <stdlib.h>

int	ft_popen(const char *file, char *const argv[1], char type)
{
	if (!file || !argv || (type != 'r' && type != 'w'))
		return -1;

	int	fds[2];
	if (pipe(fds) == -1)	return -1;

	pid_t	pid = fork();

	if (pid == -1)
	{
		close(fds[0]);
		close(fds[1]);
		return -1;
	}

	if (pid == 0)
	{
		if (type == 'r')
		{
			if (dup2(fds[1], STDOUT_FILENO) == -1)
				exit(1);
		}
		else
		{
			if(dup2(fds[0], STDIN_FILENO) == -1)
				exit(1);
		}
		close(fds[0]);
		close(fds[1]);
		execvp(file, argv);
	}

	if (type == 'r')
	{
		close(fds[1]);
		return fds[0];
	}
	else 
	{
		close(fds[0]);
		return fds[1];
	}
}

