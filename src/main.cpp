#include <termgui.hh>
#include <config.hh>
#include <fileio.hh>
#include <iostream>
#include <inhandler.hh>
#include <vector>

void processInput(Config config, int argc, char *argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    for (std::string arg: args) {
        // CLI option
        if (arg.at(0) == '-') {
            // Allow chained arguments
            std::vector<char> chain(arg.begin() + 1, arg.end());
            for (char c: chain) {
                switch (c) {
                default:
                    std::cout << "Unknown argument: " << c << std::endl;
                    exit(0);
                }
            }
        }

        // Assume file
        else {
            FileIO::openFile(config.fileData, arg);
        }
    }
}

int main(int argc, char *argv[]) {
    Config config;
    TerminalGUI terminalGUI(config);
    processInput(config, argc, argv);
    config.term.enterRaw();
    config.term.getWindowSize();
    // terminalGUI.splashScreen(editor);

    config.status.setStatusMsg("HELP: Ctrl-Q = quit");
    while (true) {
        terminalGUI.draw();
        if (InputHandler::processKey(config.cursor, config.fileData, config.term) < 0) {
            break;
        }
    }

    terminalGUI.reset();
    config.term.exitRaw();


    return 0;
}