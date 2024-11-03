#pragma once

#include <KXmlGuiWindow>

#include "opaSettings.h"
#include "database/schema.h"

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

    ~MainWindow() override;

private Q_SLOTS:
    void fileNew();

    void settingsConfigure();

    void openOrSelectPerson(IntegerPrimaryKey personId);

    void closeTab(int tabIndex);

    void openNameOriginManager();

    void openEventRolesManager();

    void openEventTypesManager();

    bool queryClose() override;

private:
    // this is the name of the root widget inside our Ui file
    // you can rename it in designer and then change it here
    Ui::Settings *m_settings;
    QAction *m_switchAction;

    int findTabFor(IntegerPrimaryKey personId);
};
