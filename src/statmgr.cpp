#include <statmgr.hh>
#include <errmgr.hh>
#include <config.hh>

void StateMgr::enterRaw(Config& config) {
    if (tcgetattr(STDIN_FILENO, &config.tty) == -1) {
        ErrorMgr::err("tcgetattr");
    }
    
    struct termios raw = config.tty;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        ErrorMgr::err("tcsetattr");
    }
}

void StateMgr::exitRaw(Config& config) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.tty) == -1) {
        ErrorMgr::err("tcsetattr");
    }
}
