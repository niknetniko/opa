/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QWidget>

class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;

class AiSettingsWidget : public QWidget {
    Q_OBJECT

public:
    explicit AiSettingsWidget(QWidget* parent = nullptr);

public Q_SLOTS:
    void saveApiKey();

private Q_SLOTS:
    void onProviderChanged(int index);

private:
    QComboBox* providerCombo = nullptr;
    QGroupBox* claudeGroup = nullptr;
    QLineEdit* modelEdit = nullptr;
    QLineEdit* apiKeyEdit = nullptr;
    QLabel* localLabel = nullptr;
    QPlainTextEdit* sourceExtractionEdit = nullptr;

    void loadApiKey();
};
