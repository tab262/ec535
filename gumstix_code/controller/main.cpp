#include <QApplication>

#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    window.setStyleSheet("background-color: #3498db");
    window.showFullScreen();
    return app.exec();
}
