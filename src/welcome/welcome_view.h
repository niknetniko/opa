/*
    SPDX-FileCopyrightText: 2022 Jiří Wolker <woljiri@gmail.com>
    SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "ui_welcome_view.h"

#include <QObject>
#include <QScrollArea>


class MainWindow;
class Placeholder;
class RecentItemsModel;

class WelcomeView : public QScrollArea, Ui::WelcomeView {
    Q_OBJECT

public:
    explicit WelcomeView(MainWindow* mainWindow, QWidget* parent = nullptr);

protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    bool eventFilter(QObject* watched, QEvent* event) override;

private Q_SLOTS:
    void onRecentItemsContextMenuRequested(const QPoint& pos);
    bool shouldClose() {
        return true;
    }

private:
    void updateButtons() const;
    void updateFonts() const;
    bool updateLayout();

    RecentItemsModel* m_recentItemsModel = nullptr;
    Placeholder* m_placeholderRecentItems = nullptr;
    Placeholder* m_placeholderSavedSessions = nullptr;
    MainWindow* mainWindow_ = nullptr;
};