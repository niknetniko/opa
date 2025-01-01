/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QStackedWidget>

class QTabWidget;
class QStackedLayout;

namespace KDDockWidgets::QtWidgets {
    class MainWindow;
}

class PlaceholderWidget : public QStackedWidget {
    Q_OBJECT

public:
    explicit PlaceholderWidget(QWidget* parent = nullptr);

    /**
     * Set the placeholder widget to be shown if the other widget is empty.
     *
     * The PlaceholderWidget will take ownership of the placeholder.
     */
    void setPlaceholder(QWidget* placeholder);

    [[nodiscard]] QWidget* getPlaceholder() const;

protected:
    void setMainWidget(QWidget* mainWidget);

    [[nodiscard]] QWidget* getMainWidget() const;
};

class KDDockPlaceholderWidget : public PlaceholderWidget {
    Q_OBJECT

public:
    void setDockContainer(KDDockWidgets::QtWidgets::MainWindow* mainWidget);
    [[nodiscard]] KDDockWidgets::QtWidgets::MainWindow* getDockContainer() const;

protected Q_SLOTS:
    void groupCountChanged(int count);
};
