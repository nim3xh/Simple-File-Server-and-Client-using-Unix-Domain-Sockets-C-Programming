#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/local_socket"
#define LISTEN_BACKLOG 5
#define BUFFER_SIZE 1024
#define FILE_DIR "./files/"  // Directory where the server stores files

void send_file(int client_socket, const char *filename) {
    char filepath[BUFFER_SIZE];
    snprintf(filepath, sizeof(filepath), "%s%s", FILE_DIR, filename);

    printf("Attempting to open file: %s\n", filepath);

    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        // Send an error message to the client
        char error_message[BUFFER_SIZE];
        snprintf(error_message, sizeof(error_message), "Error: %s", strerror(errno));
        send(client_socket, error_message, strlen(error_message), 0);
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    // Read the file and send its content to the client
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(client_socket, buffer, bytesRead, 0) == -1) {
            perror("Error sending file");
            break;
        }
    }

    fclose(file);

    // Terminate the server's end of the connection
    shutdown(client_socket, SHUT_RDWR);
}




void execute_ls(int client_socket) {
    FILE *ls_output = popen("ls", "r");
    if (ls_output == NULL) {
        perror("Error executing ls command");
        return;
    }

    char buffer[BUFFER_SIZE];

    while (fgets(buffer, sizeof(buffer), ls_output) != NULL) {
        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
            perror("Error sending ls command output");
            break;
        }
    }

    pclose(ls_output);

    // Terminate the server's end of the connection
    shutdown(client_socket, SHUT_RDWR);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_un server_address, client_address;
    socklen_t addr_len = sizeof(client_address);
    char buffer[BUFFER_SIZE];

    //Create a socket
    server_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_LOCAL;
    strncpy(server_address.sun_path, SOCKET_PATH, sizeof(server_address.sun_path) - 1);

    //Bind the socket to the address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(sa_family_t) + strlen(server_address.sun_path)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    //Listen for incoming connections
    if (listen(server_socket, LISTEN_BACKLOG) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on local socket %s...\n", SOCKET_PATH);

    while (1) {
        //Accept connections in an event loop
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue; // Continue to the next iteration
        }

        printf("Connection accepted from client\n");

        //Read data from the socket and handle requests
        ssize_t bytes_received;
        while ((bytes_received = read(client_socket, buffer, sizeof(buffer))) > 0) {
            // Null-terminate the received data
            buffer[bytes_received] = '\0';

            // Print the received data
            printf("Received data from client: %s\n", buffer);

            // Check if the message is "quit"
            if (strncmp(buffer, "quit", 4) == 0) {
                printf("Received quit message. Exiting...\n");
                close(client_socket);
                close(server_socket);
                unlink(SOCKET_PATH);
                exit(EXIT_SUCCESS);
            } else if (strncmp(buffer, "ls", 2) == 0) {
                printf("Received ls command. Executing...\n");
                execute_ls(client_socket);
            } else {
                // Handle file requests
                send_file(client_socket, buffer);
            }

            // Clear the buffer for the next iteration
            memset(buffer, 0, sizeof(buffer));
        }


        if (bytes_received == -1) {
            perror("Read error");
        }

        //Close the client socket
        close(client_socket);
    }

    // The server will not reach this point in this example

    return 0;
}
