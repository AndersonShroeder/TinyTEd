// TinyTed: A tiny text editor
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sstream>

#define K_CTRL(k) ((k) & 0x1f)
// Responsible for managing visual aspects of terminal
namespace TerminalGUI {
	static void wipeScreen();
	static void resetCursor();
	static void hideCursor();
	static void showCursor();
	static void reset();
	static void flushBuf();
	static void drawRows(std::stringstream buf);
	static void draw();
	static int getWindowSize();
	static int getCursorPosition();
	static void initGUI();
}


// Responsible for handling/implementing errors
namespace ErrorMgr {
	static void err(const char *s);
};


// Responsible for managing meta state of Terminal/program
namespace StateMgr {
	static void exitRaw();
	static void enterRaw();
};

namespace globalConfig {
	static int sRow;
	static int sCol;
	static struct termios tty;
};

// Responsible for managing visual aspects of terminal
namespace TerminalGUI {
	std::stringstream buf;

	static void wipeScreen() {
		buf << "\x1b[2J"; // Clear screen
	}

	static void resetCursor() {
		buf << "\x1b[H"; // Reset cursor to 1, 1
	}

	static void hideCursor() {
		buf << "\x1b[?25l]"; // esacpe-argument-cursour-reset(off).
	}

	static void showCursor() {
		buf << "\x1b[?25h]"; // esacpe-argument-cursour-set(on).
	}

	static void reset() {
		wipeScreen();
		resetCursor();
		flushBuf();
	}

	static void flushBuf() {
		std::cout << buf.rdbuf(); // Send buffer to stdout
		buf.clear();
	}

	static void drawRows() {
		for (int r = 0; r < globalConfig::sRow; r++) {
			std::string s = r < globalConfig::sRow - 1 ? "~\r\n" : "~";
			buf << s;
		}
	}

	static void draw() {	
		// Hide cursor to prevent flicker if possible --> if not supported by system logic is ignored.
		hideCursor();	
		reset();
		drawRows();
		resetCursor();
		showCursor();
		flushBuf();
	}

	static int getWindowSize() {
		struct winsize w;

		char c;
		// If ioctl doesnt work use escape sequences to find rows/cols
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0) {
			if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
			return getCursorPosition();
		}

		globalConfig::sRow = w.ws_row;
		globalConfig::sCol = w.ws_col;
		return 0;
	}

	static int getCursorPosition() {
		char buf[32];
		uint32_t i;

		if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
		for (i = 0; i < sizeof(buf) - 1; i++) {
			if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
			if (buf[i] == 'R') break;
		}

		buf[i] = '\0';

		if (buf[0] != '\x1b' || buf[1] != '[') return -1;
		if (sscanf(&buf[2], "%d;%d", globalConfig::sRow, globalConfig::sCol) != 2) return -1;

		return 0;
	}

	static void initGUI() {
		if (getWindowSize() < 0) ErrorMgr::err("getWindowSize");
	}
}


// Responsible for handling/implementing errors
namespace ErrorMgr {
	
	static void err(const char *s) {
		TerminalGUI::reset();
		perror(s);
		exit(1);
	} 
};


// Responsible for managing meta state of Terminal/program
namespace StateMgr {
	static void exitRaw() {
		int ret = tcsetattr(STDIN_FILENO, TCSANOW, &globalConfig::tty);
		if (ret < 0) {
			ErrorMgr::err("tcsetattr");
		}
	}

	static void enterRaw() {
		int ret = tcgetattr(STDIN_FILENO, &globalConfig::tty);
		if (ret < 0) {
			ErrorMgr::err("tcgetattr");
		}

		atexit(exitRaw);

		struct termios copy = globalConfig::tty; 
		copy.c_iflag &= ~(ICRNL | IXON | INPCK | BRKINT | ISTRIP); // Turn off ctrl-s/q and carriage return conversions.
		copy.c_oflag &= ~(OPOST); // Turn off output processing.
		copy.c_cflag |= (CS8); // 8 bit bytes
		copy.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // Disable echo, cannonical mode, ctrl-v, ctrl-c, ctrl-z

		ret = tcsetattr(STDIN_FILENO, TCSANOW, &copy);
		if (ret < 0) {
			ErrorMgr::err("tcsetattr");
		}
	}
};


class Editor {
	char c;
	int r;

	const char readKey() {
		char c;
		while (r = read(STDIN_FILENO, &c, sizeof(c)) != 1) {
			if (r < 0 && errno != EAGAIN) {
				ErrorMgr::err("read");
			}
		}

		return c;
	}

public:
	void procKey() {
		const char c = this->readKey();
		std::cout << c;
		switch (c) {
			case K_CTRL('q'): {
				TerminalGUI::reset();
				exit(0);
				break;
			}
			default: {
				break;
			}
		}
	}

		
};

int main(){
	StateMgr::enterRaw();
	TerminalGUI::initGUI();
	Editor p;

	while(1) {
		TerminalGUI::draw();
		p.procKey();
	}
	return 0;
}
