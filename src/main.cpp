#include <QApplication>

#include "window.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    chart_qt::Window win;
    win.resize(500, 500);
    win.show();

    return app.exec();
}
