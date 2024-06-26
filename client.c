#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/local_socket"
#define BUFFER_SIZE 1024

void receive_data(int server_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    // Continuously read from the socket and print to standard output
    while ((bytesRead = read(server_socket, buffer, sizeof(buffer))) > 0) {
        // Print the received data for debugging
        printf("Received data from server (%zd bytes): '", bytesRead);
        fwrite(buffer, 1, bytesRead, stdout);
        printf("'\n");

        // Check if the received data contains an error message
        if (strncmp(buffer, "Error", 5) == 0) {
            fprintf(stderr, "Server error: %s\n", buffer);
            break;
        }

        // Check if the received data is the "quit" command
        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Received quit command. Exiting...\n");
            break;
        }
    }

    // Check if the read failed or if it reached the end of the file
    if (bytesRead == -1) {
        perror("Error reading from socket");
    } else {
        printf("Connection closed by the server. Exiting...\n");
    }
}

int main() {
    int client_socket;
    struct sockaddr_un server_address;

    //Create a socket
    client_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_LOCAL;
    strncpy(server_address.sun_path, SOCKET_PATH, sizeof(server_address.sun_path) - 1);

    // Establish a connection with the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(sa_family_t) + strlen(server_address.sun_path)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");

    while (1) {
        //Input a message from the user
        printf("Enter a message (type 'quit' to exit): ");
        char buffer[BUFFER_SIZE];  // Initialize the buffer
        memset(buffer, 0, sizeof(buffer));  // Zero out the buffer
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove the newline character from fgets

        //Send the message to the server
        if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        //Check if the message is "quit"
        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Sent quit message. Exiting...\n");
            break;  // Exit the loop
        }

        //Receive data from the server and print to standard output
        receive_data(client_socket);
    }

    //Close the socket
    close(client_socket);

    return 0;
}
