// TinyTed: A tiny text editor
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

#define K_CTRL(k) ((k) & 0x1f)

int rows = 24;

namespace GlobalData {
	static struct termios tty;
};


// Responsible for managing visual aspects of terminal
namespace TerminalGUI {
	static void wipeScreen() {
		write(STDOUT_FILENO, "\x1b[2J", 4); // Clear screen
	}

	static void resetCursor() {
		write(STDOUT_FILENO, "\x1b[H", 4); // Reset cursor to 1, 1
	}

	static void reset() {
		wipeScreen();
		resetCursor();
	}

	static void drawRows() {
		for (int r = 0; r < rows; r++) {
			write(STDOUT_FILENO, "~\r\n", 3);
		}
	}

	static void draw() {
		reset();
		drawRows();
		resetCursor();
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
		int ret = tcsetattr(STDIN_FILENO, TCSANOW, &GlobalData::tty);
		if (ret < 0) {
			ErrorMgr::err("tcsetattr");
		}
	}

	static void enterRaw() {
		int ret = tcgetattr(STDIN_FILENO, &GlobalData::tty);
		if (ret < 0) {
			ErrorMgr::err("tcgetattr");
		}

		atexit(exitRaw);

		struct termios copy = GlobalData::tty; 
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
	Editor p;

	while(1) {
		TerminalGUI::draw();
		p.procKey();
	}
	return 0;
}
