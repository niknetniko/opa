/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "ai_settings_widget.h"

#include "../core/keychain_keys.h"
#include "opaSettings.h"
#include <qt6keychain/keychain.h>

#include <KLocalizedString>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

AiSettingsWidget::AiSettingsWidget(QWidget* parent) :
    QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    providerCombo = new QComboBox(this);
    providerCombo->setObjectName(QStringLiteral("kcfg_aiProvider"));
    providerCombo->addItem(i18n("None"));
    providerCombo->addItem(i18n("Claude API"));
    providerCombo->addItem(i18n("Local (llama.cpp)"));

    auto* topForm = new QFormLayout;
    topForm->addRow(i18n("AI Provider:"), providerCombo);
    layout->addLayout(topForm);

    claudeGroup = new QGroupBox(i18n("Claude API Settings"), this);
    auto* claudeForm = new QFormLayout(claudeGroup);

    modelEdit = new QLineEdit(claudeGroup);
    modelEdit->setObjectName(QStringLiteral("kcfg_claudeModel"));
    claudeForm->addRow(i18n("Model:"), modelEdit);

    apiKeyEdit = new QLineEdit(claudeGroup);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setPlaceholderText(i18n("Enter API key…"));
    claudeForm->addRow(i18n("API Key:"), apiKeyEdit);

    layout->addWidget(claudeGroup);

    localLabel = new QLabel(i18n("Local AI — coming soon"), this);
    localLabel->setEnabled(false);
    layout->addWidget(localLabel);

    auto* promptsGroup = new QGroupBox(i18n("System Prompts"), this);
    auto* promptsForm = new QFormLayout(promptsGroup);

    sourceExtractionEdit = new QPlainTextEdit(promptsGroup);
    sourceExtractionEdit->setObjectName(QStringLiteral("kcfg_sourceExtractionPrompt"));
    promptsForm->addRow(i18n("Source Extraction:"), sourceExtractionEdit);

    layout->addWidget(promptsGroup);
    layout->addStretch();

    connect(providerCombo, &QComboBox::currentIndexChanged, this, &AiSettingsWidget::onProviderChanged);

    // Show/hide groups for the current setting.
    onProviderChanged(providerCombo->currentIndex());

    // Asynchronously pre-populate the API key field from the keychain.
    loadApiKey();
}

void AiSettingsWidget::onProviderChanged(int index) {
    // Enum order: 0 = None, 1 = Claude, 2 = Local
    claudeGroup->setVisible(index == 1);
    localLabel->setVisible(index == 2);
}

void AiSettingsWidget::loadApiKey() {
    auto* job = new QKeychain::ReadPasswordJob(KeychainKeys::Service, this);
    job->setKey(KeychainKeys::ClaudeApiKey);
    job->setAutoDelete(true);

    connect(job, &QKeychain::ReadPasswordJob::finished, this, [this, job] {
        if (job->error() == QKeychain::NoError) {
            apiKeyEdit->setText(job->textData());
        }
    });

    job->start();
}

void AiSettingsWidget::saveApiKey() {
    auto* job = new QKeychain::WritePasswordJob(KeychainKeys::Service, this);
    job->setKey(KeychainKeys::ClaudeApiKey);
    job->setTextData(apiKeyEdit->text());
    job->setAutoDelete(true);
    job->start();
}
