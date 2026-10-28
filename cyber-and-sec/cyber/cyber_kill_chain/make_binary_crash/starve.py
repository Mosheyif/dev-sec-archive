# *********************************************************
#
# Description: Forks and runs the target binary with VAS limited
#              to 1 MB, triggering an OOM crash (ENOMEM)
# Author: Moshe Yifrah
#
# *********************************************************

import os
import resource

pid = os.fork()

if pid == 0:
    # Child process: restrict Virtual Address Space to 1MB (1024 * 1024 bytes)
    resource.setrlimit(resource.RLIMIT_AS, (1048576, 1048576))
    
    # Execute the binary in this starved environment
    os.execve("./make_this_crash", ["./make_this_crash"], os.environ)
else:
    # Parent process (your shell): wait safely for the child to crash
    os.waitpid(pid, 0)
    print("[*] Child process crashed safely. Shell remains intact.")
