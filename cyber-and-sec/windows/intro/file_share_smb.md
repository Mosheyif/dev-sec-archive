
## Exercise 5: Investigate File Sharing with Wireshark

**Objective:** Capture and analyze Server Message Block (SMB) network traffic, identify the authentication handshake, extract network components such as the NTLMv2 Hash, and analyze transferred file payloads.

### Step 1: Session Reset and Environment Preparation
To force a full authentication handshake and prevent Windows from reusing cached credentials, existing active SMB sessions must be terminated.

1. Open a **Command Prompt**.
2. Execute the following command to clear active network connections:
   ```cmd
   net use * /delete /y

    ```

### Step 2: Initialize Wireshark Capture

1. Launch **Wireshark**.
2. Select the designated network interface facilitating the Host-to-VM communication.
3. Begin the packet capture.

### Step 3: Trigger SMB Authentication and File Transfer

1. Return to the **Command Prompt**.
2. Authenticate to the remote share using the designated lab account credentials:
```cmd
net use \\<Host_IP>\<Shared_Directory> /user:<Username> <Password>

```


3. Initiate a file transfer to generate SMB read/write traffic:
```cmd
copy \\<Host_IP>\\<Shared_Directory>\<file_name> C:\Users\Public\Desktop\

```



### Step 4: Packet Analysis and Hash Extraction

1. Stop the Wireshark capture.
2. Apply the following display filter to isolate relevant authentication and transfer traffic:
```text
smb2 || ntlmssp

```


3. **Hash Extraction:** Locate the `Session Setup Request, NTLMSSP_AUTH` packet. Expand the NTLM details in the packet tree to locate the `NTLMv2 Response` field, which contains the cryptographic challenge-response hash.
4. **Payload Analysis:** Locate packets such as `SMB2 Read Response` or `SMB2 Write Request`. Inspect the packet bytes (Hex View) to observe the transferred file's content in clear text.

---

### Appendix: The SMB Authentication and Transfer Protocol Flow

The captured traffic illustrates the standard SMB network flow, divided into six distinct phases:

**Phase 1: Protocol Negotiation**

* **Negotiate Request & Response:** The client and server exchange their supported SMB dialects (e.g., SMB 2.1, SMB 3.1.1) and security capabilities. The server selects the highest mutually supported version to establish the connection baseline.

**Phase 2: Session Setup and Authentication (NTLMSSP)**

* **NTLMSSP_NEGOTIATE:** The client sends a Session Setup Request indicating it wants to authenticate using the NTLM Security Support Provider.
* **NTLMSSP_CHALLENGE:** The server responds with a Session Setup Response containing an 8-byte random string known as the Server Challenge.
* **NTLMSSP_AUTH:** The client calculates the NTLMv2 Response (the cryptographic proof) using the user's password hash, the Server Challenge, and a locally generated Client Blob. It sends this response, along with the username and domain, back to the server. If valid, the server assigns a unique Session ID.

**Phase 3: Share Access (Tree Connect)**

* **Tree Connect Request & Response:** The authenticated client requests access to a specific network share path. The server verifies permissions and, if authorized, grants access and assigns a Tree ID.

**Phase 4: File Initialization (Create)**

* **Create Request & Response:** The client sends a request to open or create a specific file within the mapped Tree ID. The server allocates a file handle and returns a unique hexadecimal File ID (GUID) used as a reference for subsequent operations.

**Phase 5: Data Transfer (Read/Write)**

* **Write/Read Operations:** The client requests specific byte offsets using the File ID. The server transmits the file's binary payload back to the client. In standard SMBv2 environments without encryption enforced, this payload is transmitted and can be read in clear text.

**Phase 6: Session Teardown**

* **Close Request & Response:** Once the transfer is complete, the client requests to close the active file handle. The server confirms, releasing the file lock. The client can then explicitly terminate the share connection and destroy the Session ID.

