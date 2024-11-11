/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "opaSettings.h"

#include <KXmlGuiWindow>
#include <QAction>


class KRecentFilesAction;
namespace Ui {
    class Settings;
}

class opaView;

/**
 * The main window of the program.
 *
 * This window is responsible for putting together the various views,
 * menus, toolbars and status bars.
 *
 * This class serves as the main window for opa.  It handles the
 * menus, toolbars and status bars.
 */
class MainWindow : public KXmlGuiWindow {
    Q_OBJECT

public:
    MainWindow();

    ~MainWindow() override = default;

    [[nodiscard]] KRecentFilesAction* recentFilesAction() const;

    void showWelcomeScreen();

protected:
    void saveProperties(KConfigGroup& config) override;
    void readProperties(const KConfigGroup& config) override;

public Q_SLOTS:
    void newFile();
    void openFile();
    void openUrl(const QUrl& url);
    void closeFile();

private Q_SLOTS:
    void settingsConfigure();

    void openOrSelectPerson(IntegerPrimaryKey personId);

    void closeTab(int tabIndex) const;

    void openNameOriginManager() const;

    void openEventRolesManager() const;

    void openEventTypesManager() const;

    bool queryClose() override;

private:
    QString currentFile;

    // this is the name of the root widget inside our Ui file
    // you can rename it in designer and then change it here
    Ui::Settings* m_settings = nullptr;
    QAction* m_switchAction = nullptr;

    QAction* manageNameOrigins_ = nullptr;
    QAction* manageEventRoles_ = nullptr;
    QAction* manageEventTypes_ = nullptr;
    KRecentFilesAction* recentFilesAction_ = nullptr;
    QAction* openNewAction_ = nullptr;
    QAction* openAction_ = nullptr;
    QAction* quitAction_ = nullptr;
    QAction* closeAction_ = nullptr;

    [[nodiscard]] int findTabFor(IntegerPrimaryKey personId) const;

    void loadFile(const QString& filename, bool isNew = false);

    void clearUi();

    void syncActions();

    [[nodiscard]] QTabWidget* getTabWidget() const;
};
