#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include "MainWindow.h"
#include "Style.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KAboutData about("obsbot-kde", i18n("OBSBOT KDE"), "1.0.0",
        i18n("Contrôleur OBSBOT Tiny 2 Lite pour KDE Plasma"),
        KAboutLicense::GPL_V3, i18n("© 2025 obsbot-linux contributors"));
    about.setHomepage("https://github.com/obsbot-linux/obsbot-kde");
    about.setOrganizationDomain("github.com");
    KAboutData::setApplicationData(about);
    QApplication::setWindowIcon(QIcon::fromTheme("obsbot-kde"));

    MainWindow w;
    w.show();
    return app.exec();
}
