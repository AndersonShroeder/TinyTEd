#include <commands.hh>
#include <inhandler.hh>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <fileio.hh>

void Commands::Search::callback(Config &cfg, std::string_view s, int c)
{
    // Track the index of the previous match and the direction of movement
    static int matchPrev = -1;
    static int matchDir = 1;

    static int matchLineIdx = -1;
    static std::array<textState, UCHAR_MAX> matchLine;
    if (matchLineIdx != -1) {
        auto row = cfg.fileData.at(matchLineIdx);
        std::copy(matchLine.begin(), matchLine.end(), row->textStates.begin());
        matchLineIdx = -1;
    }

    // Handle input commands
    if (c == '\r' || c == '\x1b')
    {                   // Enter or Escape key
        matchPrev = -1; // Reset match state
        matchDir = 1;   // Default direction
        return;
    }
    else if (c == ARROW_RIGHT || c == ARROW_DOWN)
    {
        matchDir = 1; // Move forward
    }
    else if (c == ARROW_LEFT || c == ARROW_UP)
    {
        matchDir = -1; // Move backward
    }
    else
    {
        matchPrev = -1; // Reset on other key presses
        matchDir = 1;
    }

    // Start search from the current match or the beginning
    if (matchPrev == -1)
        matchDir = 1;
    int current = matchPrev;

    // Search through the file data
    for (size_t i = 0; i < cfg.fileData.size(); i++)
    {
        current += matchDir;
        if (current < 0)
            current = cfg.fileData.size() - 1;
        else if (current >= (int)cfg.fileData.size())
            current = 0;

        std::shared_ptr<Row> row = cfg.fileData.at(current);

        size_t match = row->sRender.find(s);
        if (match != std::string::npos)
        { // Found a match
            matchPrev = current;
            cfg.cursor.cy = current;
            cfg.cursor.cx = match + s.size();
            cfg.cursor.rOffset = cfg.term.sRow; // Adjust row offset

            matchLineIdx = current;
            std::copy(row->textStates.begin(), row->textStates.end(), matchLine.begin());
            std::fill(row->textStates.begin() + match, row->textStates.begin() + match + s.size(), TS_SEARCH);

            break;
        }
    }
}

void Commands::Search::run(TerminalGUI &gui, Config &cfg)
{
    // Save previous cursor state
    size_t prevCX = cfg.cursor.cx;
    size_t prevCY = cfg.cursor.cy;
    size_t prevCOffset = cfg.cursor.cOffset;
    size_t prevROffset = cfg.cursor.rOffset;

    // Prompt user for search input
    std::string_view s = InputHandler::promptUser(gui, cfg, "Search: ", Commands::Search::callback);

    // Reset cursor position if no search term was provided
    if (s.empty())
    {
        cfg.cursor.cx = prevCX;
        cfg.cursor.cy = prevCY;
        cfg.cursor.cOffset = prevCOffset;
        cfg.cursor.rOffset = prevROffset;
    }
}

void Commands::LaunchServer::run(TerminalGUI &gui, Config &cfg)
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
   
    std::string connectionPort = InputHandler::promptUser(gui, cfg, "INADDR_ANY <Port>: ", std::nullopt);
    if (connectionPort.empty()) {
      connectionPort = "8080";
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(std::stoi(connectionPort));
    
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
    
    cfg.status.setStatusMsg("TinyTed server launched on port: " + connectionPort);
    gui.draw();
    
    // Accept an incoming connection
    if ((cfg.conn.sockfd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    cfg.conn.connected = true;

    cfg.status.setStatusMsg("Connection Established, transfering file data");
    // gui.draw();

    // Send path
    size_t length =  cfg.fileData.path.string().size();
    send(cfg.conn.sockfd, &length, sizeof(length), 0);
    send(cfg.conn.sockfd, cfg.fileData.path.string().c_str(), length, 0);

    // Send buffer data
    std::string line;
    for (auto line: cfg.fileData.fileData)
    {
        uint32_t size = line->size();
        // send size
        send(cfg.conn.sockfd, &size, sizeof(size), 0);

        // read ack

        // send data
        send(cfg.conn.sockfd, line->sRaw.c_str(), line->size(), 0);

        // read ack
    }

    // fin
    int fin = -1;
    send(cfg.conn.sockfd, &fin, 0, 0);

    close(server_fd);
}

void Commands::ConnectServer::run(TerminalGUI &gui, Config &cfg)
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    std::string message;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
    }

    // Get Connection input
    std::string connectionIP = InputHandler::promptUser(gui, cfg, "<IP>: ", std::nullopt);
    if (connectionIP.empty()) {
      connectionIP = "127.0.0.1";
    }

    std::string connectionPort = InputHandler::promptUser(gui, cfg, connectionIP + " <Port>: ", std::nullopt);
    if (connectionPort.empty()) {
      connectionPort = "8080";
    }

    int port = std::stoi(connectionPort);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, connectionIP.c_str(), &serv_addr.sin_addr) <= 0) {
        cfg.status.setStatusMsg("Invalid address/ Address not supported");
        return;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cfg.status.setStatusMsg("Connected Failed - No Server");
        return;
    }

    // std::cout << "Connected to the server. Enter a message: ";
    cfg.status.setStatusMsg("Connected to TinyTed server");


    size_t length;

    // Recv file path for name/extension
    recv(sock, &length, sizeof(length), 0);
    std::vector<char> buffer(length);
    recv(sock, buffer.data(), length, 0);
    cfg.fileData.path = std::string(buffer.begin(), buffer.end());

    parseFileExtension(cfg);

    size_t valread;
    std::vector<std::shared_ptr<Row>> fileData;

    while (true) {

        // Read size
        uint32_t size;
        valread = read(sock, &size, sizeof(size));

        if (valread <= 0 || (int)size < 0) {
            break;
        }
        // read data
        char buf[size + 1];
        memset(buf, '\0', size + 1);

        valread = read(sock, buf, size);

        std::string line(buf);

        auto r = std::make_shared<Row>(Row{line});
        r->updateRender();

        fileData.emplace_back(r);
    }

    cfg.conn.connected = true;

    // Close the socket
    // close(sock);
    // cfg.fileData.fileData.clear
    cfg.fileData.fileData = fileData;
    cfg.status.setStatusMsg("File data transfer success, now editing: " + cfg.fileData.path.string());

}
