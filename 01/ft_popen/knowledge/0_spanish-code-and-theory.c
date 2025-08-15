/*
 * EXERCISE: FT_POPEN
 * 
 * DESCRIPTION:
 * Implement a simplified version of popen() that executes commands
 * and returns a file descriptor connected to their input or output.
 * 
 /*

 
 /*
  * DIFFERENCES WITH STANDARD POPEN():
  * 
  * 1. SIMPLIFIED INTERFACE:
  *    - popen() uses command string, ft_popen() uses argv[]
  *    - popen() returns FILE*, ft_popen() returns int (fd)
  * 
  * 2. NO SHELL:
  *    - popen() executes command through /bin/sh
  *    - ft_popen() executes directly with execvp()
  *    - More secure (no injection), but less flexible
  * 
  * 3. NO AUTOMATIC CLEANUP:
  *    - popen() pairs with pclose()
  *    - ft_popen() requires manual close() of fd
  *    - Doesn't automatically handle zombie processes
  */
 

