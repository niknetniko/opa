/*
    SPDX-FileCopyrightText: 2022 Niko Strijbol <strijbol.niko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// application header
#include "main/main_window.h"
#include "opadebug.h"
#include "database/database.h"
#include "data/data_manager.h"

// KF headers
#include <KCrash>
#include <KDBusService>
#include <KAboutData>
#include <KLocalizedString>

// Qt headers
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLoggingCategory>
#include <QString>
#include <QtLogging>
#include <QTableView>
#include <QSqlQueryModel>


int main(int argc, char **argv)
{
    qSetMessagePattern(QString::fromUtf8("%{file}(%{line}): %{message}"));
    QApplication application(argc, argv);

    KLocalizedString::setApplicationDomain("opa");
    KCrash::initialize();

    KAboutData aboutData( QStringLiteral("opa"),
                          i18n("opa"),
                          QStringLiteral("0.1"),
                          i18n("A Simple Application written with KDE Frameworks"),
                          KAboutLicense::GPL,
                          i18n("Copyright 2022, Niko Strijbol <strijbol.niko@gmail.com>"));

    aboutData.addAuthor(i18n("Niko Strijbol"),i18n("Author"), QStringLiteral("strijbol.niko@gmail.com"));
    aboutData.setOrganizationDomain("example.org");
    aboutData.setDesktopFileName(QStringLiteral("org.example.opa"));

    KAboutData::setApplicationData(aboutData);
    application.setWindowIcon(QIcon::fromTheme(QStringLiteral("opa")));

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.process(application);
    aboutData.processCommandLine(&parser);

    KDBusService appDBusService(KDBusService::Multiple | KDBusService::NoExitOnFailure);

    DataManager::initialize(&application);

    // Do start-up
    open_database(QStringLiteral("./test.db"));

    MainWindow window;
    window.show();

    return application.exec();
}
