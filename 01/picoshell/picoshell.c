/* PICOSHELL.C - COMPLETE ENGLISH TRANSLATION

## EXERCISE: PICOSHELL

**DESCRIPTION:**  
Implement a mini shell that executes command pipelines.  
Connect the output of one command with the input of the next.

**KEY CONCEPTS:**
1. **PIPELINE**: Chain of commands connected by pipes
2. **MULTIPLE PROCESSES**: One fork per command  
3. **REDIRECTION**: Connect stdout of one to stdin of next
4. **SYNCHRONIZATION**: Wait for all processes to terminate
5. **DESCRIPTOR MANAGEMENT**: Open/close at correct moments

**ALGORITHM:**
1. For each command (except the last), create pipe
2. Fork process for current command
3. In child: configure stdin/stdout and execute command
4. In parent: manage descriptors and continue with next command
5. Wait for all processes to finish */


#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int picoshell(char **cmds[])
{
    /*
     * PARAMETERS:
     * - cmds: Array of arrays of strings (commands)
     * - cmds[0] = {"ls", "-l", NULL}
     * - cmds[1] = {"grep", "txt", NULL}  
     * - cmds[2] = NULL (terminator)
     * 
     * RETURN:
     * - 0: all commands executed successfully
     * - 1: error in some command or syscall
     */
    
    pid_t pid;
    int pipefd[2];      // Current pipe
    int prev_fd = -1;   // Previous pipe descriptor
    int status;
    int exit_code = 0;
    int i = 0;

	// main loop: process each cmd in pipeline
    while (cmds[i])
    {
        /*
         * CREATE PIPE (except for last command):
         * - Only create pipe if there's a next command
         * - This pipe will connect current command with next
         */
        if (cmds[i + 1] && pipe(pipefd) == -1)
            return 1;
        
        /*
         * FORK PROCESS FOR CURRENT COMMAND:
         */
        pid = fork();
        if (pid == -1)
        {
            // Fork error, close created descriptors
            if (cmds[i + 1])
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }
            return 1;
        }
        
        if (pid == 0)  // CHILD PROCESS
        {
            /*
             * CHILD STDIN CONFIGURATION:
             * - If prev_fd exists, command should read from previous pipe
             * - Redirect stdin to read end of previous pipe
             */
            if (prev_fd != -1)
            {
                if (dup2(prev_fd, STDIN_FILENO) == -1)
                    exit(1);
                close(prev_fd);
            }
            
            /*
             * CHILD STDOUT CONFIGURATION:
             * - If next command exists, command should write to current pipe
             * - Redirect stdout to write end of current pipe
             */
            if (cmds[i + 1])
            {
                close(pipefd[0]);  // Close read end (don't need it)
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                    exit(1);
                close(pipefd[1]);
            }
            
            /*
             * EXECUTE COMMAND:
             * - cmds[i][0] is the command name
             * - cmds[i] is the complete argument array
             */
            execvp(cmds[i][0], cmds[i]);
            exit(1);  // Only executes if execvp fails
        }
        
        // PARENT PROCESS
        /*
         * DESCRIPTOR MANAGEMENT IN PARENT:
         * - Close prev_fd if exists (no longer needed)
         * - For current pipe:
         *   - Close write end (child uses it)
         *   - Save read end as prev_fd for next command
         */
        
        if (prev_fd != -1)
            close(prev_fd);
        
        if (cmds[i + 1])
        {
            close(pipefd[1]);     // Close write end
            prev_fd = pipefd[0];  // Save read end for next iteration
        }
        
        i++;
    }
    
    /*
     * WAIT FOR ALL CHILD PROCESSES:
     * - Use wait() to collect all processes
     * - Check exit codes
     * - If any process fails, return error
     */
    while (wait(&status) != -1)
    {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            exit_code = 1;
    }
    
    return exit_code;
}

/*
## PIPELINE DIAGRAM FOR "ls | grep txt | wc -l":

**COMMAND 1: ls**
```
CHILD 1:  stdin=terminal  stdout=pipe1[1]  stderr=terminal
PARENT:   close(pipe1[1]) prev_fd=pipe1[0]
```

**COMMAND 2: grep txt**
```  
CHILD 2:  stdin=pipe1[0]  stdout=pipe2[1]  stderr=terminal
PARENT:   close(pipe1[0]) close(pipe2[1]) prev_fd=pipe2[0]
```

**COMMAND 3: wc -l**
```
CHILD 3:  stdin=pipe2[0]  stdout=terminal  stderr=terminal
PARENT:   close(pipe2[0]) wait_all()
```

**DATA FLOW:**
```
ls → pipe1 → grep → pipe2 → wc → terminal
```

## DETAILED FILE DESCRIPTOR MANAGEMENT:

**Iteration 1 (ls):**
- Create pipe1
- Fork child1
- Child1: dup2(pipe1[1], STDOUT), exec ls
- Parent: close(pipe1[1]), prev_fd = pipe1[0]

**Iteration 2 (grep):**
- Create pipe2  
- Fork child2
- Child2: dup2(prev_fd, STDIN), dup2(pipe2[1], STDOUT), exec grep
- Parent: close(prev_fd), close(pipe2[1]), prev_fd = pipe2[0]

**Iteration 3 (wc):**
- Don't create pipe (last command)
- Fork child3
- Child3: dup2(prev_fd, STDIN), exec wc
- Parent: close(prev_fd), wait_all()

## COMMON ERRORS AND HOW TO AVOID THEM:

### 1. DEADLOCK FROM OPEN DESCRIPTORS:
- If we don't close write end in parent, grep never receives EOF
- Result: pipeline hangs indefinitely

### 2. BROKEN PIPE:
- If we close descriptors in wrong order
- Process may receive SIGPIPE and terminate

### 3. ZOMBIE PROCESSES:
- If we don't wait() for all children
- Processes remain as zombies in system

### 4. DESCRIPTOR LEAKS:
- Not closing unused descriptors
- Can exhaust system descriptor table

## KEY EXAM POINTS:

### 1. ORDER OF OPERATIONS:
- Create pipe BEFORE fork
- Configure redirections in child
- Close appropriate descriptors in parent and child

### 2. PREV_FD MANAGEMENT:
- Represents "output" of previous command
- Becomes "input" of current command
- Must be closed after use

### 3. LAST COMMAND SPECIAL CASE:
- Doesn't need output pipe
- Its stdout goes to terminal
- Check cmds[i + 1] before creating pipe

### 4. ERROR HANDLING:
- Check return values of pipe(), fork(), dup2()
- Close descriptors on error
- Return 1 if any command fails

### 5. SYNCHRONIZATION:
- wait() in loop until no more children
- Check exit codes of all processes
- Single failed process fails entire pipeline
*/


// EXAMPLE USAGE WITH MAIN:


#include <stdio.h>
#include <string.h>

static int count_cmds(int argc, char **argv)
{
    int count = 1;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "|") == 0)
            count++;
    }
    return count;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return (fprintf(stderr, "Usage: %s cmd1 [args] | cmd2 [args] ...\n", argv[0]), 1);

    int cmd_count = count_cmds(argc, argv);
    char ***cmds = calloc(cmd_count + 1, sizeof(char **));
    if (!cmds)
        return (perror("calloc"), 1);

    // Parse arguments and build command array
    int i = 1, j = 0;
    while (i < argc)
    {
        int len = 0;
        while (i + len < argc && strcmp(argv[i + len], "|") != 0)
            len++;
        
        cmds[j] = calloc(len + 1, sizeof(char *));
        if (!cmds[j])
            return (perror("calloc"), 1);
        
        for (int k = 0; k < len; k++)
            cmds[j][k] = argv[i + k];
        cmds[j][len] = NULL;
        
        i += len + 1;  // Skip the "|"
        j++;
    }
    cmds[cmd_count] = NULL;

    int ret = picoshell(cmds);

    // Clean memory
    for (int i = 0; cmds[i]; i++)
        free(cmds[i]);
    free(cmds);

    return ret;
}
