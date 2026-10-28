# *********************************************************
#
# Description: Patches a binary at a fixed offset to inject ud2
#              (SIGILL) or restore the original endbr64 prologue
# Author: Moshe Yifrah
#
# *********************************************************

with open('./make_this_crash', 'r+b') as f:
    # Seek to the exact file offset of main
    f.seek(0x1196)
    
    op = int(input("Enter 1 to destroy and 0 to restore\n"))
    
    if op == 1:
        # Overwrite the prologue (endbr64 / push rbp) with the ud2 opcode
        f.write(b'\x0f\x0b')
        print("[+] Binary permanently patched with SIGILL.")
    
    elif op == 0:
        f.write(b'\xf3\x0f\x1e\xfa')
        print("[+] Code segment repaired. Binary execution restored.")
    
    else:
        print("Input error")
    	

