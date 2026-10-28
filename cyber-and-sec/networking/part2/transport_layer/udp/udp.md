

# UDP Ping-Pong and Broadcast Networking Suite

## 1. Description
This project implements a modular, connectionless networking suite using UDP Datagram sockets. It features a full-duplex, asynchronous chat system (Ping and Pong nodes) utilizing I/O multiplexing, alongside a network broadcasting tool. The codebase strictly adheres to the ANSI C89/90 standard, POSIX compliance, and professional C formatting conventions.

---

## 2. Program Flow & Integration (Pseudo-code)

The architecture is divided into a shared utility library and individual node implementations.

```text
// SHARED UTILITIES (udp_utils.h / udp_utils.c)
Function create_udp_socket():
    Create and return an AF_INET, SOCK_DGRAM socket.
    
Function init_sockaddr():
    Zero out memory using memset.
    Configure family, IP (inet_addr), and port (htons).

Function get_user_input():
    Read from STDIN_FILENO using raw read() to prevent buffering deadlocks.
    Strip newline character.

// PING NODE (udp_ping.c - Active Client)
Function run_ping_node(Target IP, Target Port):
    sock = create_udp_socket().
    init_sockaddr(Target IP, Target Port).
    Loop infinitely:
        Call select() monitoring STDIN and sock.
        If sock has data: 
            recvfrom() and print.
        If STDIN has data: 
            get_user_input() and sendto() Target.

// PONG NODE (udp_pong.c - Passive Server)
Function run_pong_node(Listen Port):
    sock = create_udp_socket().
    bind() sock to 0.0.0.0:Listen Port.
    Set client_known flag to 0.
    Loop infinitely:
        Call select() monitoring sock (and STDIN only if client_known == 1).
        If sock has data:
            Reset client_len = sizeof(client_addr) to prevent truncation.
            recvfrom(), store client_addr, and set client_known = 1.
        If STDIN has data:
            get_user_input() and sendto() stored client_addr.

```

---

## 3. Original Prompt

> Act as a senior C systems programmer. Implement the following task: "Implement UDP ping-pong - Create two separate programs that send each other messages through a UDP connection."
> Provide strict, ANSI C89/90 standard, POSIX-compliant C code suitable for a Linux environment.
> To enforce modularity and code reuse (DRY principle), you MUST separate the shared networking logic from the application logic.
> Provide the following file structure:
> 1. `udp_utils.h` - Header file containing shared socket operations, constants, and function prototypes.
> 2. `udp_utils.c` - Implementation of shared network logic.
> 3. `udp_ping.c` - The ping node, utilizing the shared library.
> 4. `udp_pong.c` - The pong node, utilizing the shared library.
> 
> 
> Strictly enforce the following architectural constraints:
> 1. ANSI C89/90 Standard: Code MUST be compliant with `-ansi -pedantic-errors -Wall -Wextra`. All variables must be declared exclusively at the beginning of a block. Use only `/* */` for comments.
> 2. Header Documentation: Every `#include` directive MUST have an aligned, indented inline comment specifying the exact functions, macros, or types it imports.
> 3. Minimal Commenting: Exclude all other comments from the code unless they are absolutely critical to explain complex system-level logic.
> 4. Network API: Use standard POSIX sockets (`<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`).
> 5. Protocol: Implement Datagram sockets (`SOCK_DGRAM`) for UDP communication.
> 6. Memory Safety: Explicitly zero out all structures (e.g., using `memset` on `struct sockaddr_in`) before use to prevent memory corruption and undefined behavior.
> 7. Error Handling: Every system call (`socket`, `bind`, `sendto`, `recvfrom`) must be checked for failure. Use `perror` and exit gracefully upon critical failures.
> 8. Resource Management: Ensure all socket file descriptors are properly closed upon termination.
> 
> 
> Provide a `Makefile` or exact compilation instructions using `gcc`.
> Provide an .md file in a markdown format that contains this prompt itself and explains about the code structure and workflow.

---

## 4. Development Workflow & Iterations

The development of this project progressed through several critical engineering phases:

1. **Initial Implementation (Synchronous Blocking):** The first iteration successfully separated network primitives into `udp_utils`. However, it utilized blocking `recvfrom()` calls and hardcoded ports, forcing a rigid, alternating ping-pong sequence.
2. **I/O Multiplexing (Asynchronous Duplex):** To allow simultaneous sending and receiving, `select()` was introduced to monitor both the network socket and `STDIN_FILENO` concurrently. CLI arguments were added to dynamically assign IP and Port configurations.
3. **Resolving Buffer Desynchronization (`fgets` vs `select`):** A critical deadlock occurred because `fgets()` (User Space buffered I/O) drained the kernel buffer, causing subsequent `select()` calls to hang. This was resolved by replacing `fgets` with the raw POSIX `read()` system call.
4. **Fixing `socklen_t` Truncation:** The `recvfrom()` function modifies the address length parameter. A bug where this variable was not re-initialized before each loop iteration was fixed to prevent network address truncation.
5. **Strict Styling and Extension:** The codebase was refactored to use tab-aligned formatting, explicit banner documentation (Author: Moshe Yifrah), static helper functions, and distinct section headers. Finally, a `udp_broadcast.c` utility was added to the `Makefile`.

---

## 5. Final Comprehensive Prompt

To generate the final, polished version of this software suite in one go, use the following prompt:

```text
Act as a senior C systems programmer. Implement a UDP networking suite consisting of three programs: an active client (Ping), a passive server (Pong), and a broadcasting node.

File Structure Requirements:
1. `udp_utils.h` - Shared prototypes and constants.
2. `udp_utils.c` - Shared network logic (socket creation, address initialization, unbuffered user input).
3. `udp_ping.c` - Active node utilizing I/O multiplexing.
4. `udp_pong.c` - Passive node utilizing I/O multiplexing.
5. `udp_broadcast.c` - Node utilizing `SO_BROADCAST`.
6. `Makefile` - Compiles all three executables.

Strict Architectural & Formatting Constraints:
1. Standard: Strict ANSI C89/90 standard, POSIX-compliant (`-ansi -pedantic-errors -Wall -Wextra`). All variables MUST be declared at the top of their respective blocks. 
2. Asynchronous I/O: Use `select()` to monitor both the socket and `STDIN_FILENO`. Do NOT use `fgets` for user input, as it causes buffer desynchronization with `select`. Use raw `read()` instead.
3. Memory Safety: Explicitly use `memset` on `struct sockaddr_in`. In `udp_pong.c`, you MUST re-initialize `socklen_t client_len = sizeof(client_addr);` immediately before EVERY `recvfrom` call to prevent truncation.
4. Protocol: Use `AF_INET` and `SOCK_DGRAM`. Ensure proper error handling (`perror`, `exit`) for `socket`, `bind`, `sendto`, `recvfrom`, `select`, and `setsockopt`. 
5. Dynamic CLI Arguments: No hardcoded ports or IPs (except for broadcast/listen all addresses). Accept them via `argv`.
6. Code Styling:
   - Use TABS for indentation, never spaces.
   - Include a file banner at the top of every `.c` file with Description, Author (Moshe Yifrah), Reviewer (leave blank), and compilation instructions.
   - Separate code sections using asterisks (e.g., `/* Includes *********************************************/`).
   - Every `#include` directive must have a tab-aligned comment showing the specific functions imported.
   - Separate logic into static helper functions (e.g., `HandleIncomingData`, `HandleKeyboardInput`) that perform single, atomic tasks.
   - Omit variable names from function prototypes in header files (e.g., `void die_with_error(const char*);`).

Produce all the requested `.c`, `.h`, and `Makefile` code blocks according to these constraints.

```

```

