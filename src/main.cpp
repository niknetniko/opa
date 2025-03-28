/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "data/data_manager.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "main/main_window.h"
#include <kddockwidgets/Config.h>
#include <kddockwidgets/MainWindow.h>

#include <KAboutData>
#include <KCrash>
#include <KIconTheme>
#include <KLocalizedString>
#include <KStyleManager>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QIcon>
#include <QLoggingCategory>
#include <QString>
#include <QtLogging>

int main(int argc, char** argv) {
    qSetMessagePattern(QStringLiteral("%{if-category}[%{category}] %{endif}%{file}(%{line}): %{message}"));
    const QApplication application(argc, argv);

    // Ensure proper icons and styles on non-plasma sessions.
    KIconTheme::initTheme();
    KStyleManager::initStyle();

    KLocalizedString::setApplicationDomain("opa");
    KCrash::initialize();

    initFrontend(KDDockWidgets::FrontendType::QtWidgets);

    KAboutData aboutData(
        QStringLiteral("opa"),
        i18n("opa"),
        QStringLiteral("0.1"),
        i18n("A Qt/KDE genealogy program"),
        KAboutLicense::GPL_V3,
        i18n("Copyright Niko Strijbol <niko@strijbol.be>")
    );

    aboutData.addAuthor(i18n("Niko Strijbol"), i18n("Author"), QStringLiteral("niko@strijbol.be"));
    aboutData.setOrganizationDomain("example.org");
    aboutData.setDesktopFileName(QStringLiteral("org.example.opa"));

    KAboutData::setApplicationData(aboutData);
    application.setWindowIcon(QIcon::fromTheme(QStringLiteral("opa")));

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(application);
    aboutData.processCommandLine(&parser);

    if (application.isSessionRestored()) {
        kRestoreMainWindows<MainWindow>();
    } else {
        auto* window = new MainWindow;
        window->show();

        // Load a previous file if needed.
        const KSharedConfigPtr config = KSharedConfig::openConfig();
        const KConfigGroup generalGroup(config, QStringLiteral("General"));
        const bool shouldShowWelcomeScreen = generalGroup.readEntry("showWelcome", true);
        const QString existingFile = generalGroup.readPathEntry("currentFile", QString());
        const QFileInfo info(existingFile);
        if (!shouldShowWelcomeScreen && info.exists() && info.isFile()) {
            window->openUrl(QUrl::fromLocalFile(existingFile));
        }
    }

    return application.exec();
}
