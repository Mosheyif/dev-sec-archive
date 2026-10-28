

# TCP Ping-Pong Networking Suite (Multi-Threaded)

## 1. Description
This project implements a modular, connection-oriented networking suite using TCP Stream sockets. It features a robust multi-threaded server architecture capable of handling multiple concurrent clients. The codebase strictly adheres to the ANSI C89/90 standard, POSIX compliance, and professional C formatting conventions, ensuring robust connection lifecycle management (3-way handshake) and graceful tear-downs.

---

## 2. Program Flow & Integration (Pseudo-code)

The architecture is divided into a shared utility library and individual node implementations. The server utilizes `pthreads` to avoid single-client blocking.

```text
// SHARED UTILITIES (tcp_utils.h / tcp_utils.c)
Function create_tcp_socket():
    Create and return an AF_INET, SOCK_STREAM socket.
    
Function init_sockaddr():
    Zero out memory using memset.
    Configure family, IP (inet_addr), and port (htons).

Function get_user_input():
    Read from STDIN_FILENO using raw read() to prevent buffering deadlocks.
    Strip newline character.

// PING NODE (tcp_ping.c - Active Client)
Function run_tcp_client(Target IP, Target Port):
    sock = create_tcp_socket().
    init_sockaddr(Target IP, Target Port).
    connect() to the Target (initiates 3-way handshake).
    Loop infinitely:
        Call select() monitoring STDIN and sock.
        If sock has data: 
            recv() data. If return value is 0 (FIN), exit loop.
            Print received message.
        If STDIN has data: 
            get_user_input() and send() to Target.

// PONG NODE (tcp_pong.c - Multi-Threaded Server)
Function run_tcp_server(Listen Port):
    Initialize Mutex and global Client Array.
    serv_sock = create_tcp_socket().
    bind() serv_sock to 0.0.0.0:Listen Port.
    listen() on serv_sock.
    Spawn AcceptorThread(serv_sock).
    Loop infinitely (Main Thread):
        get_user_input() from keyboard.
        Lock Mutex, broadcast message to all active clients, Unlock Mutex.

Function AcceptorThread(serv_sock):
    Loop infinitely:
        clnt_sock = accept() incoming connection.
        Lock Mutex, find empty slot in Client Array, Unlock Mutex.
        Spawn ClientHandlerThread(clnt_sock) and Detach it.

Function ClientHandlerThread(clnt_sock):
    Loop infinitely:
        recv() data. If return value is 0 (FIN), break.
        Print received message.
    Lock Mutex, remove clnt_sock from array, Unlock Mutex.
    close(clnt_sock).
    Exit thread.

```

---

## 3. Original Prompt

> Act as a senior C systems programmer. Implement the following task: "Implement TCP ping-pong - Create two programs that send each other messages through a TCP connection."
> Provide strict, ANSI C89/90 standard, POSIX-compliant C code suitable for a Linux environment.
> To enforce modularity and code reuse (DRY principle), you MUST separate the shared networking logic from the application logic.
> Provide the following file structure:
> 1. `tcp_utils.h` - Header file containing shared socket operations, constants, and function prototypes.
> 2. `tcp_utils.c` - Implementation of shared network logic.
> 3. `tcp_ping.c` - The ping node (client), utilizing the shared library.
> 4. `tcp_pong.c` - The pong node (server), utilizing the shared library.
> 
> 
> Strictly enforce the following architectural constraints:
> 1. ANSI C89/90 Standard: Code MUST be compliant with `-ansi -pedantic-errors -Wall -Wextra`. All variables must be declared exclusively at the beginning of a block. Use only `/* */` for comments.
> 2. Header Documentation: Every `#include` directive MUST have an aligned, indented inline comment specifying the exact functions, macros, or types it imports.
> 3. Minimal Commenting: Exclude all other comments from the code unless they are absolutely critical to explain complex system-level logic.
> 4. Network API: Use standard POSIX sockets (`<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`).
> 5. Protocol: Implement Stream sockets (`SOCK_STREAM`) for TCP communication.
> 6. Memory Safety: Explicitly zero out all structures (e.g., using `memset` on `struct sockaddr_in`) before use.
> 7. Error Handling: Every system call (`socket`, `bind`, `listen`, `accept`, `connect`, `send`, `recv`) must be checked for failure. Use `perror` and exit gracefully upon critical failures.
> 8. Resource Management: Ensure all socket file descriptors are properly closed upon termination.
> 
> 
> Provide a `Makefile` or exact compilation instructions using `gcc`.
> Provide an .md file in a markdown format that contains this prompt itself and explains about the code structure and workflow.

---

## 4. Development Workflow & Iterations

The development of this project progressed through several critical engineering phases:

1. **Protocol Shift (Connection-Oriented):** The architecture transitioned from connectionless UDP to TCP. This required implementing `listen()` and `accept()` on the server-side, and `connect()` on the client-side to facilitate the TCP 3-way handshake.
2. **I/O Multiplexing (Asynchronous Duplex):** To allow simultaneous sending and receiving on the client side, `select()` was utilized.
3. **Resolving Buffer Desynchronization (`fgets` vs `select`):** `fgets()` caused deadlocks by buffering user input in the User Space and desynchronizing with the Kernel's `select()` state. This was fixed by utilizing raw POSIX `read()` from `STDIN_FILENO`.
4. **Graceful Disconnection Handling:** TCP explicitly notifies peers of connection closures via a `FIN` packet. The logic was updated to monitor the `recv()` return value. If it returns `0`, the program safely breaks the loop and closes the descriptors.
5. **Multi-threading Architecture:** The server was transitioned from a single-client blocking model to a multi-threaded architecture using `<pthread.h>`. The main thread handles keyboard broadcasting, an Acceptor thread manages incoming connections, and dedicated ClientHandler threads process incoming packet streams simultaneously.

---

## 5. Final Comprehensive Prompt

To generate the final, polished version of this software suite in one go, use the following prompt:

```text
Act as a senior C systems programmer. Implement a multi-threaded TCP networking suite consisting of two programs: an active client (Ping) and a passive server (Pong).

File Structure Requirements:
1. `tcp_utils.h` - Shared prototypes and constants.
2. `tcp_utils.c` - Shared network logic (socket creation, address initialization, unbuffered user input).
3. `tcp_ping.c` - Active client node utilizing I/O multiplexing via `select()`.
4. `tcp_pong.c` - Passive server node utilizing `<pthread.h>` for concurrent client handling.
5. `Makefile` - Compiles both executables using `-pthread`.

Strict Architectural & Formatting Constraints:
1. Standard: Strict ANSI C89/90 standard, POSIX-compliant (`-ansi -pedantic-errors -Wall -Wextra`). All variables MUST be declared at the top of their respective blocks. 
2. Asynchronous I/O (Client): Use `select()` to monitor both the active connection socket and `STDIN_FILENO`. Do NOT use `fgets` for user input. Use raw `read()` instead.
3. Multi-threading (Server): Implement a global array of client sockets protected by `pthread_mutex_t`. The Main thread handles reading from STDIN and broadcasting via `send()` to all active clients. Spawn a detached AcceptorThread to continuously call `accept()`. For each successful connection, spawn a detached ClientHandlerThread that loops on `recv()`.
4. Protocol & Safety: Use `AF_INET` and `SOCK_STREAM`. Handle graceful TCP disconnection (break the loop if `recv()` returns 0 and remove the socket from the array safely). 
5. Dynamic CLI Arguments: No hardcoded ports or IPs. Accept them via `argv`.
6. Code Styling:
   - Use TABS for indentation, never spaces.
   - Include a file banner at the top of every `.c` file with Description, Author (Moshe Yifrah), Reviewer (Valeriia).
   - Separate code sections using asterisks.
   - Every `#include` directive must have a tab-aligned comment showing the specific functions imported.

Produce all the requested `.c`, `.h`, and `Makefile` code blocks according to these constraints, alongside a comprehensive README.md.

```

```
