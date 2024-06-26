## Simple File Server and Client using Unix Domain Sockets

This repository contains a simple file server and client implementation using Unix Domain Sockets.

**Features:**

* **File Transfer:** The client can request files from the server by sending the filename. The server then sends the file's content back to the client.
* **Error Handling:** If the server cannot find or open a requested file, it sends an error message to the client.
* **"ls" Command:** The client can request a list of files in the server's directory by sending the "ls" command.
* **Quit Command:** Both the client and server support the "quit" command for terminating the connection.

**How to Run:**

1. Compile the server and client using a C compiler (e.g., `gcc server.c -o server` and `gcc client.c -o client`).
2. Run the server: `./server`
3. Run the client: `./client`

**Usage:**

* The client will prompt you to enter a message. Type the filename you want to request or "ls" to list files or "quit" to exit.
* The server will process the request and respond accordingly.

**Note:**

* This is a simple example and does not include advanced features like security, authentication, or file size limits.
* The server stores files in the `./files/` directory. Ensure this directory exists and has appropriate permissions.

**Code Structure:**

* `server.c`: Contains the server implementation.
* `client.c`: Contains the client implementation.

This is a basic demonstration of using Unix Domain Sockets for communication between a server and a client. You can expand upon this example by adding more features and functionality.
