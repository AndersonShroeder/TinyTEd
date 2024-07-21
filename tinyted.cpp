// TinyTed: A tiny text editor
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sstream>
#include <array>
#include <algorithm>

#define K_CTRL(k) ((k) & 0x1f)
#define VERSION "0.0.1"

enum keys {
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL,
	HOME,
	END,
	PAGE_UP,
	PAGE_DOWN,
};

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
	static void genCoverPage(std::string &s);
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
	static int sRow, sCol, cx = 0, cy = 0;
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

	static void updateCursor() {
		std::stringstream ss;
		ss << "\x1b[" << globalConfig::cy + 1 << ";" << globalConfig::cx + 1 << "H"; //must convert cursor pos from 0-indexed;
		buf << ss.str();
	}

	static void hideCursor() {
		buf << "\x1b[?25l"; // esacpe-argument-cursour-reset(off).
	}

	static void showCursor() {
		buf << "\x1b[?25h"; // esacpe-argument-cursour-set(on).
	}

	static void reset() {
		wipeScreen();
		resetCursor();
		flushBuf();
	}

	static void flushBuf() {
		std::string s = buf.rdbuf()->str();
		write(STDOUT_FILENO, s.c_str(), s.size()); // Send buffer to stdout
		buf.clear();
	}

	static void drawRows() {
		for (int r = 0; r < globalConfig::sRow; r++) {
			std::string s = "~\x1b[K"; // Add tilde and clear everything right of line with K0.
			s += r < globalConfig::sRow - 1 ? "\r\n" : "";
			buf << s;
		}
	}

	static void draw() {	
		// Hide cursor to prevent flicker if possible --> if not supported by system logic is ignored.
		hideCursor();
		resetCursor();
		drawRows();
		updateCursor();
		showCursor();
		flushBuf();
	}

	static std::string centerText(const std::string &text) {
		int padding = (globalConfig::sCol - text.length()) / 2;
		return std::string(padding > 0 ? padding : 0, ' ') + text;
	}
	

	static void genCoverPage(std::string &s) {
		s += centerText("   ___                       ___           ___           ___           ___           ___     \r\n");
		s += centerText("  /\\  \\          ___        /\\__\\         |\\__\\         /\\  \\         /\\  \\         /\\  \\    \r\n");
		s += centerText("  \\:\\  \\        /\\  \\      /::|  |        |:|  |        \\:\\  \\       /::\\  \\       /::\\  \\   \r\n");
		s += centerText("   \\:\\  \\       \\:\\  \\    /:|:|  |        |:|  |         \\:\\  \\     /:/\\:\\  \\     /:/\\:\\  \\  \r\n");
		s += centerText("   /::\\  \\      /::\\__\\  /:/|:|  |__      |:|__|__       /::\\  \\   /::\\~\\:\\  \\   /:/  \\:\\__\\ \r\n");
		s += centerText("  /:/\\:\\__\\  __/:/\\/__/ /:/ |:| /\\__\\     /::::\\__\\     /:/\\:\\__\\ /:/\\:\\ \\:\\__\\ /:/__/ \\:|__|\r\n");
		s += centerText(" /:/  \\/__/ /\\/:/  /    \\/__|:|/:/  /    /:/~~/~       /:/  \\/__/ \\:\\~\\:\\ \\/__/ \\:\\  \\ /:/  /\r\n");
		s += centerText("/:/  /      \\::/__/         |:/:/  /    /:/  /        /:/  /       \\:\\ \\:\\__\\    \\:\\  /:/  / \r\n");
		s += centerText("\\/__/        \\:\\__\\         |::/  /     \\/__/         \\/__/         \\:\\ \\/__/     \\:\\/:/  /  \r\n");
		s += centerText("              \\/__/         /:/  /                                   \\:\\__\\        \\::/__/   \r\n");
		s += centerText("                            \\/__/                                     \\/__/         ~~       \r\n");
		s += "\n\n";
		s += centerText("A Minimalist Command Line Text Editor\r\n");
		std::string tmp = "Version ";
		tmp += VERSION;
		tmp += "\r\n";
		s += centerText(tmp);
		s += centerText("(Press any key to continue)\r\n");
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
	int r;

	const int readKey() {
		char c;
		while (r = read(STDIN_FILENO, &c, sizeof(c)) != 1) {
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
							case '1': return HOME; // Multiple different ways home/end can be sent --> handle all
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
			}
			else if (buf[0] == 'O') {
				switch(buf[1]) {
					case 'H': return HOME;
					case 'F': return END;
				}
			}
			return '\x1b';
		}
		return c;
	}

	void moveCursor(int c) {
		switch(c) {
			case ARROW_UP:
				globalConfig::cy > 0 ? globalConfig::cy-- : 0;
				break;
			case ARROW_LEFT:
				globalConfig::cx > 0 ? globalConfig::cx-- : 0;
				break;
			case ARROW_DOWN:
				globalConfig::cy < globalConfig::sRow ? globalConfig::cy++ : 0;
				break;
			case ARROW_RIGHT:
				globalConfig::cx < globalConfig::sCol ? globalConfig::cx++ : 0;

				break;
		}
	}

public:
	void procKey(bool breakAny = false) {
		int c = this->readKey();
		if (breakAny) return;
		switch (c) {
			// Quit keystroke
			case K_CTRL('q'): {
				TerminalGUI::reset();
				exit(0);
				break;
			}

			// top/bottom of file
			case PAGE_UP:
			case PAGE_DOWN:
				globalConfig::cy = c == PAGE_UP ? 0 : globalConfig::sRow - 1;
				break;

			// start/end of line
			case HOME:
			case END:
				globalConfig::cx = c == HOME ? 0 : globalConfig::sCol - 1;
				break;

			// delete
			case DEL:
				break;

			// Curosr moving
			case ARROW_UP:
			case ARROW_LEFT:
			case ARROW_DOWN:
			case ARROW_RIGHT:
				moveCursor(c);

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

	// Cover Page
	std::string s;
	TerminalGUI::genCoverPage(s);
	std::cout << s << std::endl;
	p.procKey(true);
	TerminalGUI::reset();

	while(1) {
		TerminalGUI::draw();
		p.procKey();
	}
	return 0;
}
