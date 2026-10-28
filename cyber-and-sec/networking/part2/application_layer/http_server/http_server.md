

# HTTP Server Implementation Guide

## 1. Description
This project implements a multi-threaded HTTP web server in C, utilizing POSIX sockets. The server operates on the Application Layer (Layer 7) over a TCP transport foundation. It asynchronously accepts incoming connections, parses HTTP `GET` requests, returns a static response for the root directory (`/`), and dynamically serves physical HTML files requested from the local file system.

---

## 2. Program Flow (Pseudo-code)

The server utilizes a concurrent architecture where a master thread accepts connections and dispatches worker threads to handle the stateless HTTP request-response cycle.

```text
Function main():
    sock = create_tcp_socket()
    bind(sock, 0.0.0.0:8000)
    listen(sock, BACKLOG)
    Spawn AcceptorThread(sock)
    Wait for AcceptorThread to finish (join)

Function AcceptorThread(sock):
    Loop infinitely:
        clnt_sock = accept(sock)
        Spawn detached ClientHandlerThread(clnt_sock)

Function ClientHandlerThread(clnt_sock):
    Receive data (HTTP Request) from clnt_sock
    Parse HTTP Method and Path
    If Method == "GET":
        If Path == "/":
            Send 200 OK with "Hello World" body
        Else:
            Attempt to open file at Path
            If file exists:
                Send 200 OK with file contents
            Else:
                Send 404 Not Found
    Else:
        Send 405 Method Not Allowed
    
    Close clnt_sock (Stateless execution)
    Exit Thread

```

---

## 3. Step-by-Step Testing Guide

To properly test the server, we simulate a real-world domain name resolution using the local loopback interface, and subsequently fetch both static routes and physical files.

### Step 1: Local DNS Configuration

Map a custom hostname to your local machine to simulate a real web domain. Add the following entry to your `/etc/hosts` file:

```text
127.0.0.1       mycoolserver.local

```

### Step 2: Starting the Server

Compile the program and launch the server. It will begin listening for incoming HTTP connections on port `8000`.

**Server Terminal:**

```bash
user@host:~/http_server$ ./http_server 
HTTP Server running on port 8000...

```

### Step 3: Web Browser & File Serving Tests

While the server is running, you can test it using a web browser or command-line tools like `wget`. First, we generate a physical HTML file on the disk, and then we fetch it.

**Client Terminal:**

```bash
user@host:~/http_server$ echo '<h1>This is a real file!</h1>' > test.html
user@host:~/http_server$ wget -qO- [http://mycoolserver.local:8000/test.html](http://mycoolserver.local:8000/test.html)
<h1>This is a real file!</h1>

```

**Server Terminal Output:**
As the client executes requests via a browser and `wget`, the server logs the incoming parsed methods and paths:

```bash
[HTTP Request]: GET /
[HTTP Request]: GET /favicon.ico
[HTTP Request]: GET /
[HTTP Request]: GET /HTTP Request]: GET /test.html
[HTTP Request]: GET /test.html

*(Note: Browsers often automatically request a `favicon.ico` alongside the root directory request).*

```

