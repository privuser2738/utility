#include "application.h"
#include "mainwindow.h"
#include "settings.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Enable high DPI support
    SetProcessDPIAware();
#endif

    Application app(argc, argv);

    return app.exec();
}
