#include "data/data_manager.h"
#include "database/database.h"
#include "logging.h"
#include "main/main_window.h"

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KIconTheme>
#include <KLocalizedString>
#include <KStyleManager>
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLoggingCategory>
#include <QString>
#include <QTableView>
#include <QtLogging>

int main(int argc, char** argv) {
    qSetMessagePattern(QStringLiteral("%{if-category}[%{category}] %{endif}%{file}(%{line}): %{message}"));
    QApplication application(argc, argv);

    // Ensure proper icons and styles on non-plasma sessions.
    KIconTheme::initTheme();
    KStyleManager::initStyle();

    KLocalizedString::setApplicationDomain("opa");
    KCrash::initialize();

    KAboutData aboutData(
        QStringLiteral("opa"),
        i18n("opa"),
        QStringLiteral("0.1"),
        i18n("A Simple Application written with KDE Frameworks"),
        KAboutLicense::GPL,
        i18n("Copyright 2022, Niko Strijbol <strijbol.niko@gmail.com>")
    );

    aboutData.addAuthor(i18n("Niko Strijbol"), i18n("Author"), QStringLiteral("strijbol.niko@gmail.com"));
    aboutData.setOrganizationDomain("example.org");
    aboutData.setDesktopFileName(QStringLiteral("org.example.opa"));

    KAboutData::setApplicationData(aboutData);
    application.setWindowIcon(QIcon::fromTheme(QStringLiteral("opa")));

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(application);
    aboutData.processCommandLine(&parser);

    const KDBusService appDBusService(KDBusService::Multiple | KDBusService::NoExitOnFailure);

    // Set up the SQLite database file.
    open_database(QStringLiteral("./test.db"));

    // Initialize the model manager.
    DataManager::initialize(&application);

    auto* window = new MainWindow;
    window->show();

    return application.exec();
}
