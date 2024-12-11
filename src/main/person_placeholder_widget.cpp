/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "person_placeholder_widget.h"

#include "editors/new_person_editor_dialog.h"
#include "main_window.h"

#include <KLocalizedString>
#include <QLabel>
#include <QVBoxLayout>

PersonPlaceholderWidget::PersonPlaceholderWidget(QWidget* parent) : QWidget(parent) {

    auto* label1 = new QLabel(i18n("Nobody selected"));
    auto* label2 = new QLabel(i18n("Select a person in the list to the right"));
    auto* label3 = new QLabel(i18n("<a href=\"%1\">Or add a new person</a>").arg(OPEN_PERSON_ANCHOR));
    label3->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    label3->setTextFormat(Qt::RichText);
    label3->setOpenExternalLinks(false);
    connect(label3, &QLabel::linkActivated, this, &PersonPlaceholderWidget::onLinkActivated);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(label1);
    layout->addWidget(label2);
    layout->addWidget(label3);
    layout->setAlignment(Qt::AlignCenter);
}

void PersonPlaceholderWidget::onLinkActivated(const QString& link) {
    if (link == OPEN_PERSON_ANCHOR) {
        auto* dialog = new NewPersonEditorDialog(this);
        dialog->show();
    }
}
