/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "placeholder_widget.h"

#include <kddockwidgets/MainWindow.h>

using namespace KDDockWidgets;

static constexpr int PLACEHOLDER_POSITION = 0;
static constexpr int MAIN_POSITION = 1;

PlaceholderWidget::PlaceholderWidget(QWidget* parent) : QStackedWidget(parent) {
    // Populate with empty widgets by default.
    addWidget(new QWidget);
    addWidget(new QWidget);
}

void PlaceholderWidget::setPlaceholder(QWidget* placeholder) {
    Q_ASSERT(count() == 2);
    const int index = currentIndex();
    auto* oldWidget = widget(PLACEHOLDER_POSITION);
    insertWidget(PLACEHOLDER_POSITION, placeholder);

    removeWidget(oldWidget);
    oldWidget->deleteLater();

    setCurrentIndex(index);
    Q_ASSERT(count() == 2);
}

QWidget* PlaceholderWidget::getPlaceholder() const {
    return widget(PLACEHOLDER_POSITION);
}

void PlaceholderWidget::setMainWidget(QWidget* mainWidget) {
    Q_ASSERT(count() == 2);
    const int index = currentIndex();
    auto* oldWidget = widget(MAIN_POSITION);
    insertWidget(MAIN_POSITION, mainWidget);

    removeWidget(oldWidget);
    oldWidget->deleteLater();

    setCurrentIndex(index);
    Q_ASSERT(count() == 2);
}

QWidget* PlaceholderWidget::getMainWidget() const {
    return widget(MAIN_POSITION);
}

void KDDockPlaceholderWidget::setDockContainer(QtWidgets::MainWindow* mainWidget) {
    setMainWidget(mainWidget);
    connect(mainWidget, &QtWidgets::MainWindow::groupCountChanged, this, &KDDockPlaceholderWidget::groupCountChanged);
}

QtWidgets::MainWindow* KDDockPlaceholderWidget::getDockContainer() const {
    auto* container = qobject_cast<QtWidgets::MainWindow*>(getMainWidget());
    Q_ASSERT(container != nullptr);
    return container;
}

void KDDockPlaceholderWidget::groupCountChanged(int count) {
    if (count == 0) {
        setCurrentIndex(PLACEHOLDER_POSITION);
    } else {
        setCurrentIndex(MAIN_POSITION);
    }
}
