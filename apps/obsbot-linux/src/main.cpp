#include <QApplication>
#include <QIcon>
#include "MainWindow.h"
#include "Style.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("OBSBOT Linux");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("obsbot-linux");
    app.setOrganizationDomain("github.com/obsbot-linux");
    app.setWindowIcon(QIcon::fromTheme("obsbot-linux"));
    app.setStyleSheet(ObsbotStyle::globalStyleSheet());
    MainWindow w;
    w.show();
    return app.exec();
}
