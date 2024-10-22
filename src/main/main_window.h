/*
    SPDX-FileCopyrightText: 2022 Niko Strijbol <strijbol.niko@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef OPAWINDOW_H
#define OPAWINDOW_H

#include <KXmlGuiWindow>

#include "opaSettings.h"
#include "database/schema.h"

namespace Ui { class Settings; }
class opaView;

/**
 * The main window of the program.
 *
 * This window is responsible for putting together the various views,
 * menus, toolbars and status bars.
 *
 * This class serves as the main window for opa.  It handles the
 * menus, toolbars and status bars.
 *
 * @short Main window class
 * @author Niko Strijbol <strijbol.niko@gmail.com>
 * @version 0.1
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    MainWindow();

    /**
     * Default Destructor
     */
    ~MainWindow() override;

private Q_SLOTS:
    /**
     * Create a new window
     */
    void fileNew();

    /**
     * Open the settings dialog
     */
    void settingsConfigure();

    /**
     * Open or select a person.
     */
    void openOrSelectPerson(IntegerPrimaryKey personId);

    void closeTab(int tabIndex);

    void openNameOriginManager();
    void openEventRolesManager();

private:
    // this is the name of the root widget inside our Ui file
    // you can rename it in designer and then change it here
    Ui::Settings* m_settings;
    QAction *m_switchAction;

    int findTabFor(IntegerPrimaryKey personId);
};

#endif // OPAWINDOW_H
