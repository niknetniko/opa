/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_placeholder_widget.h"

#include <KLocalizedString>
#include <QLabel>
#include <QVBoxLayout>

PersonPlaceholderWidget::PersonPlaceholderWidget(QWidget* parent) : QWidget(parent) {

    auto* label1 = new QLabel(i18n("Nobody selected"));
    auto* label2 = new QLabel(i18n("Select a person in the list to the right"));
    auto* label3 = new QLabel(i18n("Or add a new person (TODO)"));

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label1);
    layout->addWidget(label2);
    layout->addWidget(label3);
    layout->setAlignment(Qt::AlignCenter);
}
