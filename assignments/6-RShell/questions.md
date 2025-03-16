1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The the remote client determines when a command's output is fully received from the server using an End-of-File (EOF) character marker. In the client,
- The client sends a command to the server
- The client enters a receiving loop to collect response data
- For each chunk of data received, it checks if the last byte is the special EOF character:
```is_eof = (rsp_buff[recv_bytes-1] == RDSH_EOF_CHAR) ? 1 : 0;```
- If the EOF character is detected, the client knows the server has finished sending the command output and breaks out of the receive loop.

On the server side (exec_client_requests function), after executing each command, it sends this EOF character:
```send_message_eof(cli_socket);```

To ensure partial reads or ensure complete message transmission, I use byte-based printing instead of assuming null-terminated strings:
```printf("%.*s", (int)recv_bytes - 1, rsp_buff);```
Other techniques are to allocate fixed-size buffers (RDSH_COMM_BUFF_SZ) and managing their usgase, and proper
error checking (check for negative return values from recv() to detect communication errors)

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

Ways for detecting command boundaries:
- Explicit Delimiters (e.g., \n, \0). Each command is terminated by a special character (e.g., newline \n, null byte \0). The receiver reads data until it encounters the delimiter.
- Each message starts with a fixed-size header specifying the length of the command. The receiver reads the header first, then reads the specified number of bytes.
- Length-Prefixed protocol (variable-length encoding), that uses a length field (e.g., in binary format) before the actual command.

Challenges if this is not handled correctly:
- Message Truncation: Receiving only part of a message and treating it as complete.
- Message Concatenation: Processing multiple messages as one message, leading to incorrect processing and injection leaks.
- Buffer Overflows: If the receiver gets a maximum message size without properly enforcing it
- Blocking and Deadlocks: Not properly handling partial reads can lead to the client waiting infinitely for more data

3. Describe the general differences between stateful and stateless protocols.

Stateful Protocols:
- Maintain state between requests (maintaining context/state of a connection or session)
- Requires more resources since the server stores session data needing more memory and overhead.
- More complex but has more robust capabilities and reliability.
- Examples: TCP, FTP. 

Stateless Protocols:
- No memory of past interactions,  request is treated independently.
- Lightweight and scalable, no session data is stored, so servers can handle more requests efficiently.
- Will be faster, will have less overhead, but will not be fully reliable
- Examples: IP, HTTP, UDP

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

Despite being unreliable (not guaranteeing delivery, order, ETC), UDP is used because it is fast, lightweight, and efficient due to low overhead. It can be ideal for applications where speed is more important, like streaming, VoIP, and online gaming, where minor data loss is preferred over delays.


5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The operating system provides the socket API as an interface to enable applications to use network communications. 
A socket is an endpoint for sending or receiving data over a network, the socket API allows applications to create, configure, and manage network connections.

