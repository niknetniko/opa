/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "database/schema.h"
#include "domain/media/media_entities.h"

#include <QDialog>
#include <optional>

class QLabel;
class QLineEdit;
class QPlainTextEdit;

class MediaEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit MediaEditDialog(const MediaEntity& entity, QWidget* parent = nullptr);

public Q_SLOTS:
    void accept() override;

private:
    IntegerPrimaryKey mediaId;
    QString absolutePath;
    QLineEdit* titleEdit;
    QPlainTextEdit* noteEdit;

    void openFile() const;
    void showInFolder() const;
};
