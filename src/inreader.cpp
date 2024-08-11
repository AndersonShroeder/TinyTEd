#include <inreader.hh>
#include <unistd.h>
#include <array>
#include <errmgr.hh>
#include <config.hh>

int InputReader::readKey() {
    char c;
    int r;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    while ((r = read(STDIN_FILENO, &c, sizeof(c))) != 1) {
        if (r < 0 && errno != EAGAIN) {
            ErrorMgr::err("read");
        }
        
    }
    
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 5000; // Timeout in microseconds

    if (c == '\x1b') {
        std::array<char, 3> buf;

        if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0) return c;
        if (read(STDIN_FILENO, &buf[0], 1) != 1) return c;
        
        if (select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) <= 0) return c;
        if (read(STDIN_FILENO, &buf[1], 1) != 1) return c;

        
        if (buf[0] == '[') {
            if (buf[1] >= '0' && buf[1] <= '9') {
                if (read(STDIN_FILENO, &buf[2], 1) != 1) return c;
                if (buf[2] == '~') {
                    switch (buf[1]) {
                        case '1': return HOME;
                        case '3': return DEL;
                        case '4': return END;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME;
                        case '8': return END;
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
                }
            }
        } else if (buf[0] == 'O') {
            switch(buf[1]) {
                case 'H': return HOME;
                case 'F': return END;
            }
        }
        return '\x1b';
    }
    return c;
}