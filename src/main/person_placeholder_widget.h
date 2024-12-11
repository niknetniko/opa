/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QWidget>

class PersonPlaceholderWidget : public QWidget {
    Q_OBJECT

public:
    static const inline auto OPEN_PERSON_ANCHOR = QStringLiteral("opa:new_person");

    explicit PersonPlaceholderWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void onLinkActivated(const QString& link);
};
