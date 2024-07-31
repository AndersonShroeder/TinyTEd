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
#include <vector>
#include <list>
#include <memory>
#include <fstream>
#include <regex>
#include <time.h>

#define K_CTRL(k) ((k) & 0x1f)
#define VERSION "0.0.1"
#define TABSTOP 4

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

struct rowData {
	std::string s;
	std::string renderS;
};

class Editor {
	int r;
	int readKey();
	void moveCursor(int c);

public:
	void procKey(bool breakAny = false);	
};

namespace FileIO {
	static void openFile(char *path);
}

// Responsible for managing visual aspects of terminal
namespace TerminalGUI {
	static void wipeScreen();
	static void resetCursor();
	static void hideCursor();
    static int rowCxToRX(std::shared_ptr<rowData> row);
	static void showCursor();
	static void scroll();
	static void reset();
	static void flushBuf();
	static void splashScreen(Editor &e);
    static void setStatusMsg(std::string msg);
	static void drawRows();
    static void drawStatusBar();
    static void drawMessageBar();
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
	static size_t sRow, sCol, rOffset, cOffset, cx, cy, rx;
	static struct termios tty;
	static std::vector<std::shared_ptr<rowData>> fileData;
    static std::string filename;
    static std::string statusMsg;
    static time_t statusTime = 0;
};


int Editor::readKey() {
	char c;
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

void Editor::moveCursor(int c) {

	std::shared_ptr<rowData> data = globalConfig::cy >= globalConfig::fileData.size() ? NULL : globalConfig::fileData.at(globalConfig::cy);
	
	switch(c) {
		case ARROW_UP:
			globalConfig::cy > 0 ? globalConfig::cy-- : 0;
			break;
		case ARROW_LEFT:
			// Move cursor left -- if at beginning of line go to previous line
			if (globalConfig::cx != 0) {
				globalConfig::cx--;
			} else if (globalConfig::cy > 0) {
				globalConfig::cy--;
				globalConfig::cx = data->s.size();
			}
			break;
		case ARROW_DOWN:
			globalConfig::cy < globalConfig::fileData.size() ? globalConfig::cy++ : 0;
			break;
		case ARROW_RIGHT:
			if (data && globalConfig::cx < data->s.size()) {
				globalConfig::cx++;
			} else if (data && globalConfig::cx == data->s.size()) {
				globalConfig::cy++;
				globalConfig::cx = 0;
			}
			break;
	}

	// cy could be updated after the switch statement so reset
	data = globalConfig::cy >= globalConfig::fileData.size() ? NULL : globalConfig::fileData.at(globalConfig::cy);
	size_t newlen = data ? data->s.size() : 0;
	if (globalConfig::cx > newlen) {
		globalConfig::cx = newlen;
	}
}

void Editor::procKey(bool breakAny) {
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
            {
                if (c == PAGE_UP) {
                    globalConfig::cy = globalConfig::rOffset;
                } else if (c == PAGE_DOWN) {
                    globalConfig::cy = globalConfig::rOffset + globalConfig::sRow - 1;
                    if (globalConfig::cy > globalConfig::fileData.size()) {
                        globalConfig::cy = globalConfig::fileData.size();
                    }
                }
                
                int times = globalConfig::sRow;
                while (times--)
                moveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
			break;

		// start/end of line
		case HOME:
            globalConfig::cx = 0;
            break;
		case END:
            if (globalConfig::cy < globalConfig::fileData.size()) {
                globalConfig::cx = globalConfig::fileData.at(globalConfig::cy)->s.size();
            }
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

namespace FileIO{
	static void openFile(char *path) {
		std::ifstream file(path);
		if (!file) ErrorMgr::err("failed to open file");
        globalConfig::filename = path;

		std::string spaceStr;
		for (size_t i = 0; i < TABSTOP; i++) spaceStr += " ";

		std::string s1;
		while (std::getline(file, s1)) {
			std::string s2 = std::regex_replace(s1, std::regex("\t"), spaceStr);
			globalConfig::fileData.emplace_back(std::make_shared<rowData>(s1, s2));
		}

		file.close();
	}
}

// Responsible for managing visual aspects of terminal
namespace TerminalGUI {
	std::stringstream buf;

	static void wipeScreen() {
		buf << "\x1b[2J"; // Clear screen
	}

	static void resetCursor() {
		buf << "\x1b[H"; // Reset cursor to 1, 1
	}

    static int rowCxToRX(std::shared_ptr<rowData> row) {
        int rx = 0;
        for (size_t j = 0; j < globalConfig::cx; j++) {
            if (row->s[j] == '\t')
                rx += (TABSTOP - 1) - (rx % TABSTOP);
            rx++;
        }

        return rx;
    }

	static void updateCursor() {
		std::stringstream ss;
		ss << "\x1b[" << (globalConfig::cy - globalConfig::rOffset) + 1 << ";" << globalConfig::rx + 1 << "H"; //must convert cursor pos from 0-indexed;
		buf << ss.str();
	}

	static void hideCursor() {
		buf << "\x1b[?25l"; // esacpe-argument-cursour-reset(off).
	}

	static void showCursor() {
		buf << "\x1b[?25h"; // esacpe-argument-cursour-set(on).
	}

	static void scroll() {
		// Vertical Scroll
        globalConfig::rx = 0;
        if (globalConfig::cy < globalConfig::fileData.size()) {
            globalConfig::rx = rowCxToRX(globalConfig::fileData.at(globalConfig::cy));
        }


		if (globalConfig::cy < globalConfig::rOffset) globalConfig::rOffset = globalConfig::cy;
		if (globalConfig::cy >= globalConfig::rOffset + globalConfig::sRow) globalConfig::rOffset = globalConfig::cy - globalConfig::sRow + 1;

		// Horizontal Scroll
		if (globalConfig::rx < globalConfig::cOffset) globalConfig::cOffset = globalConfig::rx;
		if (globalConfig::rx >= globalConfig::cOffset + globalConfig::sCol) globalConfig::cOffset = globalConfig::rx - globalConfig::sCol + 1;
	}

	static void reset() {
		wipeScreen();
		resetCursor();
		flushBuf();
	}

	static void flushBuf() {
		std::string s = buf.rdbuf()->str();
		write(STDOUT_FILENO, s.c_str(), s.size()); // Send buffer to stdout
		buf.str("");
	}

	static void drawRows() {
		for (size_t r = 0; r < globalConfig::sRow; r++) {
			if (r + globalConfig::rOffset >= globalConfig::fileData.size()) {
				std::string s = "~\x1b[K"; // Add tilde and clear everything right of line with K0.
				s += r < globalConfig::sRow - 1 ? "\r\n" : "";
				buf << s;
			} else {
				std::string data = globalConfig::fileData.at(r+globalConfig::rOffset)->renderS;
				// std::cout << data << '\r\n';

				int len = data.size() - globalConfig::cOffset;
				if (len < 0) len = 0;
				if ((size_t)len > globalConfig::sCol) len = globalConfig::sCol;

				buf << "\x1b[K";
				buf << data.substr(globalConfig::cOffset, len);
                buf << "\r\n";
			}
		}
	}

    static void drawMessageBar() {
        buf << "\x1b[K";
        size_t msglen = globalConfig::statusMsg.size();
        if (msglen > globalConfig::sCol) msglen = globalConfig::sCol;
        if (msglen && time(NULL) - globalConfig::statusTime < 5) {
            buf << globalConfig::statusMsg.substr(0, msglen);
        }
    }

    static void drawStatusBar() {
        std::string lstring = " " + globalConfig::filename + " - " + std::to_string(globalConfig::fileData.size()) + " lines";
        std::string rstring = "Ln " + std::to_string(globalConfig::cy + 1) + ", Col " + std::to_string(globalConfig::cx + 1) + " ";
        std::string spaces(globalConfig::sCol - lstring.size() - rstring.size(), ' ');
        buf << "\x1b[7m" << lstring << spaces << rstring << "\x1b[m\r\n"; // Draw status in inverted colors
    }

	static void draw() {	
		// Hide cursor to prevent flicker if possible --> if not supported by system logic is ignored.
		scroll();
		hideCursor();
		resetCursor();
		drawRows();
        drawStatusBar();
        drawMessageBar();
		updateCursor();
		showCursor();
		flushBuf();
	}

	static void splashScreen(Editor &e) {
		// Generate splash screen
		reset();
		std::string s;
		genCoverPage(s);
		std::cout << s << std::endl;
		e.procKey();
	}

	static std::string centerText(const std::string &text) {
		int padding = (globalConfig::sCol - text.length()) / 2;
		return std::string(padding > 0 ? padding : 0, ' ') + text;
	}
	

	static void genCoverPage(std::string &s) {
		std::vector<std::string> v;
		v.push_back("\033[1;36m");
        v.push_back(centerText("   ___                       ___           ___           ___           ___           ___     \r\n"));
        v.push_back(centerText("  /\\  \\          ___        /\\__\\         |\\__\\         /\\  \\         /\\  \\         /\\  \\    \r\n"));
        v.push_back(centerText("  \\:\\  \\        /\\  \\      /::|  |        |:|  |        \\:\\  \\       /::\\  \\       /::\\  \\   \r\n"));
        v.push_back(centerText("   \\:\\  \\       \\:\\  \\    /:|:|  |        |:|  |         \\:\\  \\     /:/\\:\\  \\     /:/\\:\\  \\  \r\n"));
        v.push_back(centerText("   /::\\  \\      /::\\__\\  /:/|:|  |__      |:|__|__       /::\\  \\   /::\\~\\:\\  \\   /:/  \\:\\__\\ \r\n"));
        v.push_back(centerText("  /:/\\:\\__\\  __/:/\\/__/ /:/ |:| /\\__\\     /::::\\__\\     /:/\\:\\__\\ /:/\\:\\ \\:\\__\\ /:/__/ \\:|__|\r\n"));
        v.push_back(centerText(" /:/  \\/__/ /\\/:/  /    \\/__|:|/:/  /    /:/~~/~       /:/  \\/__/ \\:\\~\\:\\ \\/__/ \\:\\  \\ /:/  /\r\n"));
        v.push_back(centerText("/:/  /      \\::/__/         |:/:/  /    /:/  /        /:/  /       \\:\\ \\:\\__\\    \\:\\  /:/  / \r\n"));
        v.push_back(centerText("\\/__/        \\:\\__\\         |::/  /     \\/__/         \\/__/         \\:\\ \\/__/     \\:\\/:/  /  \r\n"));
        v.push_back(centerText("              \\/__/         /:/  /                                   \\:\\__\\        \\::/__/   \r\n"));
        v.push_back(centerText("                            \\/__/                                     \\/__/         ~~       \r\n"));
        v.push_back("\n\n");
		v.push_back("\e[1;35m");
		v.push_back(centerText("A Minimalist Command Line Text Editor\r\n"));
		std::string tmp = "Version ";
		tmp += VERSION;
		tmp += "\r\n";
		v.push_back(centerText(tmp));
		v.push_back(centerText("(Press any key to continue)\r\n"));
		v.push_back("\e[0m");

		size_t verticalPadding = (globalConfig::sRow - v.size()) / 2;
		for (size_t i = 0; i < verticalPadding + v.size(); i++) {
			s += i < verticalPadding ? "\n" : v.at(i - verticalPadding);
		}
	}

    static void setStatusMsg(std::string msg) {
        globalConfig::statusMsg = msg;
        globalConfig::statusTime = time(NULL);
    }

	static int getWindowSize() {
		struct winsize w;

		// If ioctl doesnt work use escape sequences to find rows/cols
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0) {
			if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
			return getCursorPosition();
		}

        w.ws_row -= 2; // subtract one for status bar
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
		if (sscanf(&buf[2], "%ld;%ld", &globalConfig::sRow, &globalConfig::sCol) != 2) return -1;

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


int main(){
	StateMgr::enterRaw();
	TerminalGUI::initGUI();
    TerminalGUI::setStatusMsg("Ctrl-Q to exit");
	Editor e;

	// Cover Page
	TerminalGUI::splashScreen(e);
	char *test_path = "tinyted.cpp";
	FileIO::openFile(test_path);

	while(1) {
		TerminalGUI::draw();
		e.procKey();
	}
	return 0;
}
