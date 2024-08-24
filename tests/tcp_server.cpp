#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>


const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server is listening on port " << PORT << std::endl;
    
    // Accept an incoming connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Connection accepted" << std::endl;

    // Send File Buffer data
    std::ifstream ifs("/home/aschroeder/tinyted/TinyTEd/src/config.cpp");
    if (!ifs)
    {
        perror("Failed to open file"); // Report the file path in error
        return -1;                            // Return an error code
    }

    std::string line;
    while (std::getline(ifs, line))
    {
        uint32_t size = line.size();
        // send size
        send(new_socket, &size, sizeof(size), 0);

        // read ack

        // send data
        send(new_socket, line.c_str(), line.size(), 0);

        // read ack
    }

    ifs.close();

    // // Receive and echo messages back to the client
    // while (true) {

    //     ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
    //     if (valread <= 0) {
    //         std::cout << "Client disconnected" << std::endl;
    //         break;
    //     }
        
    //     std::cout << "Received: " << buffer << std::endl;
    //     memset(buffer, 0, BUFFER_SIZE);
    // }
    
    close(new_socket);
    close(server_fd);
    return 0;
}
