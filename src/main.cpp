#include <termgui.hh>
#include <editor.hh>
#include <config.hh>
#include <fileio.hh>
#include <iostream>

int main(int argc, char *argv[]) {
    Config config;
    config.term.enterRaw();
    config.term.getWindowSize();

    Editor editor(config);
    TerminalGUI terminalGUI(config);
    // terminalGUI.splashScreen(editor);
    if (argc >= 2) {
        FileIO::openFile(config.fileData, argv[1]);
    }
    // else {
    //     std::cout << "NEED A FILE LOL\r\n";
    //     config.term.exitRaw();
    //     exit(0);
    // }

    config.status.setStatusMsg("HELP: Ctrl-Q = quit");
    
    while (true) {
        terminalGUI.draw();
        if (editor.processKey() < 0) {
            break;
        }
    }

    terminalGUI.reset();
    config.term.exitRaw();


    return 0;
}