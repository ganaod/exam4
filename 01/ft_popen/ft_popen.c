#include <unistd.h>
#include <stdlib.h>

/* 
.file: name of executable to launch
.argv: arg array (execvp style)
. type: 
	r: read from cmd
	w: write to cmd 
fd: connected to cmd	

ALGO
1. create pipe for comms
2. fork to create child proc
3. in child: redirect stdin/out & execute cmd
4. in parent: ret appropriate pipe end */
int	fd_popen(const char *file, char *const argv[1], char type)
{
	// 1. validate input
	// no NULL-ptrs & valid type
	if (!file || !argv || (type != 'r' && type != 'w'))
		 return -1;

	// 2. create pipe for unidirectional communication
	// array for pipe ends. will hold fd[0]: read, fd[1]: write
	int	fds[2];
	if (pipe(fds) == -1)	return -1;


	// 3. fork: create worker process
	pid_t	pid = fork();

	if (pid == -1)	// fork err
	{
		close(fds[0]);
		close(fds[1]);
		return -1;
	}

	// 4. child setup: transform into tool prog with redirected IO
	if (pid == 0)	// child process, will become tool prog
	{
		if (type == 'r')
		{
			/* want to read from cmd
			.cmd should write to its stdout
			.redirect cmd's stdout to pipe write end
			.parent reads from read end */
			if(dup2(fds[1], STDOUT_FILENO) == -1)
				exit(1);
		}
		else	// type == 'w'
		{
			/* want to write to cmd 
			.cmd should read from its stdin
			.redirect cmd's stdin to pipe read end
			.parent will write to write end */
			if(dup2(fds[0], STDIN_FILENO) == -1)
				exit(1);
		}

		// close both orig pipe ends (already redirected)
		close(fds[0]);
		close(fds[1]);

		/* execute cmd:
		.execvp searches for executable in PATH 
		.replaces proc image, transforms into tool prog
		.never returns on success */
		execvp(file, argv);	
	}


	/* 5. parent process
	.close pipe end not needed
	.return end used for comms */
	if (type == 'r')
	{
		/* parent wants to read from cmd 
		.close write end (child uses it) 
		.ret read end so parent can read */
		close(fds[1]);
		return fds[0];
	}
	else	// type == 'w'
	{
		/* parent want to write to cmd 
		.close read end (child using) 
		.ret write end so parent can write */
		close(fds[0]);
		return fds[1];
	}
}

/* err & resource management

system err handling:
.pipe() fail: no fildes available
.fork() fail: no mem/processes
.dup2 fail: invalid descriptors
.execvp fail: cmd not found 

fildes management:
.crucial to close unnecessary descriptors
.avoid leaks
.close in correct order to avoid deadlocks 

zombie processes:
.this implementation doesn't handle wait()
.in complete implementation, would need ft_pclose
.store PIDs to wait() for them later */