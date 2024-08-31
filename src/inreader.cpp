#include <inreader.hh>
#include <unistd.h>
#include <array>
#include <errmgr.hh>
#include <config.hh>
#include <fcntl.h>

int InputReader::readKey() {
    char c;
    int r;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Set up the timeout for select
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000; // Timeout in microseconds

    // Wait for input on STDIN using select
    int ready = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);
    if (ready == -1) {
        ErrorMgr::err("select");
    } else if (ready == 0) {
        return -1; // Timeout, no input
    }

    // Read the input character
    r = read(STDIN_FILENO, &c, sizeof(c));
    if (r == -1 && errno != EAGAIN) {
        ErrorMgr::err("read");
    }

    // Process escape sequences if any
    if (c == '\x1b') { 
        std::array<char, 3> buf;

        // Read the next characters to determine the sequence
        if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0)
            return c;
        if (read(STDIN_FILENO, &buf[0], 1) != 1)
            return c;

        if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0)
            return c;
        if (read(STDIN_FILENO, &buf[1], 1) != 1)
            return c;

        if (buf[0] == '[') {
            if (buf[1] >= '0' && buf[1] <= '9') {
                if (read(STDIN_FILENO, &buf[2], 1) != 1)
                    return c;
                if (buf[2] == '~') {
                    switch (buf[1]) {
                        case '1': return HOME;
                        case '3': return DEL;
                        case '4': return END;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME;
                        case '8': return END;
                        default: return '\x1b';
                    }
                }
            } else {
                switch (buf[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME;
                    case 'F': return END;
                    default: return '\x1b';
                }
            }
        } else if (buf[0] == 'O') {
            switch (buf[1]) {
                case 'H': return HOME;
                case 'F': return END;
                default: return '\x1b';
            }
        }
        return '\x1b'; 
    }

    return c;
}