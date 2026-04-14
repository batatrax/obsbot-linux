#include <QApplication>
#include <QIcon>
#include <QLockFile>
#include <QDir>
#include <QMessageBox>
#include "MainWindow.h"
#include "Style.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("OBSBOT Linux");
    app.setApplicationVersion("1.1.0");
    app.setOrganizationName("obsbot-linux");
    app.setOrganizationDomain("github.com/obsbot-linux");
    app.setWindowIcon(QIcon::fromTheme("obsbot-linux"));
    app.setStyleSheet(ObsbotStyle::globalStyleSheet());

    // Empêcher les instances multiples
    QLockFile lockFile(QDir::tempPath() + "/obsbot-linux.lock");
    if (!lockFile.tryLock(100)) {
        QMessageBox::warning(nullptr, "OBSBOT Linux",
            "Another instance is already running.");
        return 1;
    }

    MainWindow w;
    w.show();
    return app.exec();
}
