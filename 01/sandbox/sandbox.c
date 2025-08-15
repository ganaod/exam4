/*
 * EXERCISE: SANDBOX
 * 
 * DESCRIPTION:
 * Create a "sandbox" that executes a function and determines if it's "good" or "bad".
 * A function is bad if: segfault, abort, exit code != 0, or timeout.
 * 
 * KEY CONCEPTS:
 * 1. SIGNALS: Handle SIGALRM for timeout
 * 2. FORK: Execute function in separate process
 * 3. WAITPID: Get information about how the process terminated
 * 4. ALARM: Set timeout for the function
 * 5. SIGNAL HANDLING: Configure signal handlers
 * 
 * ALGORITHM:
 * 1. Fork child process to execute the function
 * 2. In parent: set alarm and wait with waitpid
 * 3. Analyze how the process terminated (normal, signal, timeout)
 * 4. Return 1 (good), 0 (bad), or -1 (error)
 */

 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdbool.h>
 #include <signal.h>
 #include <errno.h>
 #include <sys/wait.h>
 #include <sys/types.h>
 #include <string.h>
 
 // Global variable for child process PID
static pid_t child_pid;


/* sig handler for SIGALRM (timeout) / timeout handler:
.executes when alarm() expires
.doesn't need special logic
.its existence makes waitpid() return with EINTR */
void	alarm_handler(int sig)
{
	(void)sig;	// suppress unused param warning
}


 
 int sandbox(void (*f)(void), unsigned int timeout, bool verbose)
 {
	 /*
	  * PARAMETERS:
	  * - f: Function to test
	  * - timeout: Time limit in seconds
	  * - verbose: Whether to print diagnostic messages
	  * 
	  * RETURN:
	  * - 1: "good" function (exit code 0, no signals, no timeout)
	  * - 0: "bad" function (exit code != 0, signal, or timeout)
	  * - -1: error in sandbox (fork failed, etc.)
	  */
	 
	 struct sigaction sa;
	 pid_t pid;
	 int status;
	 
	 /*
	  * CONFIGURE SIGALRM HANDLER:
	  * - Set custom handler for timeout
	  * - Clear signal mask
	  * - Don't restart syscalls automatically
	  */
	 sa.sa_handler = alarm_handler;
	 sa.sa_flags = 0;  // No SA_RESTART: we want waitpid to be interrupted
	 sigemptyset(&sa.sa_mask);
	 sigaction(SIGALRM, &sa, NULL);
	 
	 /*
	  * FORK CHILD PROCESS:
	  */
	 pid = fork();
	 if (pid == -1)
		 return -1;  // Fork error
	 
	 if (pid == 0)  // CHILD PROCESS
	 {
		 /*
		  * EXECUTE FUNCTION IN CHILD:
		  * - Call the provided function
		  * - If returns normally, exit with code 0
		  * - If segfaults/aborts, kernel will send signal
		  */
		 f();
		 exit(0);  // Function terminated normally
	 }
	 
	 // PARENT PROCESS
	 child_pid = pid;
	 
	 /*
	  * SET TIMEOUT:
	  * - alarm() sends SIGALRM after timeout seconds
	  * - This will interrupt waitpid() if function takes too long
	  */
	 alarm(timeout);
	 
	 /*
	  * WAIT FOR CHILD PROCESS:
	  * - waitpid() can return for various reasons:
	  *   1. Child terminated normally (exit)
	  *   2. Child was terminated by signal
	  *   3. waitpid was interrupted by SIGALRM (timeout)
	  */
	 if (waitpid(pid, &status, 0) == -1)
	 {
		 if (errno == EINTR)  // Interrupted by SIGALRM
		 {
			 /*
			  * TIMEOUT DETECTED:
			  * - waitpid was interrupted by alarm
			  * - Child is probably still running
			  * - Kill it with SIGKILL and collect its state
			  */
			 kill(pid, SIGKILL);
			 waitpid(pid, NULL, 0);  // Collect zombie process
			 
			 if (verbose)
				 printf("Bad function: timed out after %d seconds\n", timeout);
			 return 0;
		 }
		 return -1;  // Other type of error
	 }
	 
	 /*
	  * ANALYZE HOW THE PROCESS TERMINATED:
	  */
	 
	 if (WIFEXITED(status))
	 {
		 /*
		  * NORMAL TERMINATION (exit):
		  * - Process called exit() or returned from main
		  * - Check the exit code
		  */
		 if (WEXITSTATUS(status) == 0)
		 {
			 if (verbose)
				 printf("Nice function!\n");
			 return 1;  // Good function
		 }
		 else
		 {
			 if (verbose)
				 printf("Bad function: exited with code %d\n", WEXITSTATUS(status));
			 return 0;  // Bad function
		 }
	 }
	 
	 if (WIFSIGNALED(status))
	 {
		 /*
		  * TERMINATION BY SIGNAL:
		  * - Process was terminated by signal (segfault, abort, etc.)
		  * - Get signal number for diagnostics
		  */
		 int sig = WTERMSIG(status);
		 if (verbose)
			 printf("Bad function: %s\n", strsignal(sig));
		 return 0;  // Bad function
	 }
	 
	 return -1;  // Unrecognized state
 }
 
 /*
  * EXAMPLE FUNCTIONS TO TEST:
  * 
  * void nice_function(void)
  * {
  *     // This function doesn't do anything bad
  *     return;
  * }
  * 
  * void bad_function_exit_code(void)
  * {
  *     // This function terminates with error code
  *     exit(1);
  * }
  * 
  * void bad_function_segfault(void)
  * {
  *     // This function causes segmentation fault
  *     int *ptr = NULL;
  *     *ptr = 42;
  * }
  * 
  * void bad_function_timeout(void)
  * {
  *     // This function never terminates
  *     while (1) {}
  * }
  * 
  * void bad_function_abort(void)
  * {
  *     // This function calls abort()
  *     abort();
  * }
  */
 
 /*
  * USAGE EXAMPLE:
  * 
  * int main()
  * {
  *     int result;
  * 
  *     printf("Testing nice function:\n");
  *     result = sandbox(nice_function, 5, true);
  *     printf("Result: %d\n\n", result);  // Expected: 1
  * 
  *     printf("Testing bad function (segfault):\n");
  *     result = sandbox(bad_function_segfault, 5, true);
  *     printf("Result: %d\n\n", result);  // Expected: 0
  * 
  *     printf("Testing bad function (timeout):\n");
  *     result = sandbox(bad_function_timeout, 2, true);
  *     printf("Result: %d\n\n", result);  // Expected: 0
  * 
  *     return 0;
  * }
  */
 
 /*
  * PROCESS STATE ANALYSIS:
  * 
  * 1. WIFEXITED(status):
  *    - true if process terminated with exit() or return
  *    - WEXITSTATUS(status) gets the exit code
  * 
  * 2. WIFSIGNALED(status):
  *    - true if process was terminated by signal
  *    - WTERMSIG(status) gets the signal number
  *    - Includes: SIGSEGV, SIGABRT, SIGKILL, etc.
  * 
  * 3. WIFSTOPPED(status):
  *    - true if process was stopped (SIGSTOP)
  *    - Not relevant for this exercise
  * 
  * 4. WIFCONTINUED(status):
  *    - true if process was continued (SIGCONT)
  *    - Not relevant for this exercise
  */
 
 /*
  * COMMON SIGNALS THAT CAN BE RECEIVED:
  * 
  * - SIGSEGV (11): Segmentation fault
  * - SIGABRT (6): Abort signal (abort() call)
  * - SIGFPE (8): Floating point exception (division by zero)
  * - SIGILL (4): Illegal instruction
  * - SIGBUS (7): Bus error
  * - SIGKILL (9): Kill signal (sent by sandbox on timeout)
  */
 
 /*
  * ZOMBIE PROCESS MANAGEMENT:
  * 
  * It's CRUCIAL to do waitpid() after kill() in timeout case:
  * - kill(pid, SIGKILL) terminates the process
  * - But process remains as "zombie" until parent does wait
  * - waitpid(pid, NULL, 0) cleans up the zombie
  * - Without this, zombie processes accumulate in system
  */
 
 /*
  * KEY POINTS FOR THE EXAM:
  * 
  * 1. SIGNAL CONFIGURATION:
  *    - Use sigaction() instead of signal() (more portable)
  *    - Don't use SA_RESTART: we want waitpid to be interrupted
  *    - Handler can be empty, only needs to exist
  * 
  * 2. TIMEOUT DETECTION:
  *    - alarm() + waitpid() interrupted by SIGALRM
  *    - Check errno == EINTR
  *    - Kill child process and collect zombie
  * 
  * 3. STATUS ANALYSIS:
  *    - Use macros WIFEXITED, WIFSIGNALED, etc.
  *    - Don't assume specific format of status
  *    - Handle all possible cases
  * 
  * 4. LEAK PREVENTION:
  *    - Always do waitpid() after kill()
  *    - Don't leave zombie processes
  *    - Verify all code paths clean up processes
  * 
  * 5. ROBUSTNESS:
  *    - Handle errors from fork(), waitpid(), etc.
  *    - Very bad functions can do unexpected things
  *    - Sandbox must be more robust than functions it tests
  */