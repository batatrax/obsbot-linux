#include <QApplication>
#include <QIcon>
#include <QLockFile>
#include <QDir>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <csignal>
#include "MainWindow.h"
#include "Style.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Terminer proprement sur SIGTERM/SIGINT → destructeurs appelés → ffmpeg tué
    signal(SIGTERM, [](int) { qApp->quit(); });
    signal(SIGINT,  [](int) { qApp->quit(); });

    // Charger la traduction selon la langue du système (ex: fr_FR → obsbot-linux_fr.qm)
    QTranslator translator;
    const QString lang = QLocale::system().name().left(2); // "fr", "de", "es"...
    if (translator.load(":/translations/obsbot-linux_" + lang))
        app.installTranslator(&translator);
    app.setApplicationName("OBSBOT Linux");
    app.setApplicationVersion("1.1.0");
    app.setOrganizationName("obsbot-linux");
    app.setOrganizationDomain("github.com/obsbot-linux");
    app.setWindowIcon(QIcon::fromTheme("obsbot-linux"));
    app.setStyleSheet(ObsbotStyle::globalStyleSheet());

    // Empêcher les instances multiples
    QLockFile lockFile(QDir::tempPath() + "/obsbot-linux.lock");
    if (!lockFile.tryLock(100)) {
        QMessageBox::warning(nullptr, QObject::tr("OBSBOT Linux"),
            QObject::tr("Another instance is already running."));
        return 1;
    }

    MainWindow w;
    w.show();
    return app.exec();
}
