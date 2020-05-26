
#include "main_window.h"
#include "ad_interface.h"

#include <QApplication>

int main(int argc, char **argv) {
    // Load fake AD data if given "fake" argument
    // This also swaps all ad interface functions to their fake versions (including login)
    if (argc >= 1 && QString(argv[1]) == "fake") {
        FAKE_AD = true;
    }

    if (!ad_interface_login()) {
        return 1;
    }

    QApplication app(argc, argv);
    MainWindow main_window;
    main_window.show();

    const int retval = app.exec();

    return retval;
}
