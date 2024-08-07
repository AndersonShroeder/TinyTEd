#include <inreader.hh>
#include <unistd.h>
#include <array>
#include <errmgr.hh>
#include <keys.hh>

int InputReader::readKey() {
    char c;
    int r;

    while ((r = read(STDIN_FILENO, &c, sizeof(c))) != 1) {
        if (r < 0 && errno != EAGAIN) {
            ErrorMgr::err("read");
        }
    }

    if (c == '\x1b') {
        std::array<char, 3> buf;
        if (read(STDIN_FILENO, &buf[0], 1) != 1) return c;
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