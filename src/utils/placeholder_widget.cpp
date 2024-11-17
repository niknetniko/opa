/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "placeholder_widget.h"

#include <QTabWidget>

static constexpr int PLACEHOLDER_POSITION = 0;
static constexpr int MAIN_POSITION = 1;

PlaceholderWidget::PlaceholderWidget(QWidget* parent) : QStackedWidget(parent) {
    // Populate with empty widgets by default.
    addWidget(new QWidget);
    addWidget(new QWidget);
}

void PlaceholderWidget::setPlaceholder(QWidget* placeholder) {
    assert(count() == 2);
    const int index = currentIndex();
    auto* oldWidget = widget(PLACEHOLDER_POSITION);
    insertWidget(PLACEHOLDER_POSITION, placeholder);

    removeWidget(oldWidget);
    oldWidget->deleteLater();

    setCurrentIndex(index);
    assert(count() == 2);
}

QWidget* PlaceholderWidget::getPlaceholder() const {
    return widget(PLACEHOLDER_POSITION);
}

void PlaceholderWidget::setMainWidget(QWidget* mainWidget) {
    assert(count() == 2);
    const int index = currentIndex();
    auto* oldWidget = widget(MAIN_POSITION);
    insertWidget(MAIN_POSITION, mainWidget);

    removeWidget(oldWidget);
    oldWidget->deleteLater();

    setCurrentIndex(index);
    assert(count() == 2);
}

QWidget* PlaceholderWidget::getMainWidget() const {
    return widget(MAIN_POSITION);
}

void TabWidgetPlaceholderWidget::setTabWidget(QTabWidget* mainWidget) {
    setMainWidget(mainWidget);
    connect(mainWidget, &QTabWidget::currentChanged, this, &TabWidgetPlaceholderWidget::currentTabChanged);
}

QTabWidget* TabWidgetPlaceholderWidget::getTabWidget() const {
    return qobject_cast<QTabWidget*>(getMainWidget());
}

void TabWidgetPlaceholderWidget::currentTabChanged(int index) {
    if (index == -1) {
        setCurrentIndex(PLACEHOLDER_POSITION);
    } else {
        setCurrentIndex(MAIN_POSITION);
    }
}
