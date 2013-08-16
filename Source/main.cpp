#include <iostream>

using namespace std;

#include <QApplication>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;

    window.resize(250, 150);
    window.setWindowTitle("Screenshotter");
    window.show();

    return app.exec();
}
