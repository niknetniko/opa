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
 * Open a person in the main window from anywhere.
 *
 * TODO: is this the best way of doing this?
 *
 * @param personId The ID to open.
 * @param activate If the window with the person should be activated.
 */
void openOrSelectPerson(IntegerPrimaryKey personId, bool activate = false);

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

Q_SIGNALS:
    /**
     * Called when a new person has been opened in the main view, or an existing person has been closed.
     * Note that selecting an existing open person does not trigger this.
     */
    void onPersonOpenedOrClosed();

public Q_SLOTS:
    void newFile();
    void openFile();
    void openUrl(const QUrl& url);
    void closeFile();
    void openOrSelectPerson(IntegerPrimaryKey personId);

private Q_SLOTS:
    void settingsConfigure();

    void closeTab(int tabIndex);

    void openNameOriginManager() const;

    void openEventRolesManager() const;

    void openEventTypesManager() const;

    bool queryClose() override;

    void syncRemoveAction() const;

    void onOpenNewPersonEditor();

    void onDeleteCurrentPerson();

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
    QAction* addNewPersonAction_ = nullptr;
    QAction* removePersonAction_ = nullptr;

    [[nodiscard]] int findTabFor(IntegerPrimaryKey personId) const;

    void loadFile(const QString& filename, bool isNew = false);

    void clearUi();

    void syncActions();

    [[nodiscard]] QTabWidget* getTabWidget() const;
};
