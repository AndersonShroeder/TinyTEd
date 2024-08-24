#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>

const int PORT = 8080;
const int BUFFER_SIZE = 1024;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    std::string message;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // std::cout << "Connected to the server. Enter a message: ";
    // std::getline(std::cin, message);

    // // Send the message to the server
    // send(sock, message.c_str(), message.size(), 0);
    // std::cout << "Message sent" << std::endl;

    // Receive the server's response
    size_t valread;
    std::stringstream data;

    while (true) {

        // Read size
        uint32_t size;
        valread = read(sock, &size, sizeof(size));

        if (valread <= 0) {
            break;
        }
        // read data
        char buf[size + 1];
        memset(buf, '\0', size + 1);

        valread = read(sock, buf, size);

        std::string line(buf);
        data << line << '\n';
    }

    std::cout << data.str();
    // Close the socket
    close(sock);

    return 0;
}
