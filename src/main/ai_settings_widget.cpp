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
#include <QTabWidget>
#include <QVBoxLayout>

AiSettingsWidget::AiSettingsWidget(QWidget* parent) :
    QWidget(parent) {
    auto* layout = new QVBoxLayout(this);

    auto* tabs = new QTabWidget(this);
    layout->addWidget(tabs);

    // --- Provider tab ---
    auto* providerTab = new QWidget(tabs);
    auto* providerLayout = new QVBoxLayout(providerTab);
    tabs->addTab(providerTab, i18n("Provider"));

    providerCombo = new QComboBox(providerTab);
    providerCombo->setObjectName(QStringLiteral("kcfg_aiProvider"));
    providerCombo->addItem(i18n("None"));
    providerCombo->addItem(i18n("Claude API"));
    providerCombo->addItem(i18n("Local (llama.cpp)"));
    providerCombo->addItem(i18n("OpenAI Compatible"));

    auto* topForm = new QFormLayout;
    topForm->addRow(i18n("AI Provider:"), providerCombo);
    providerLayout->addLayout(topForm);

    claudeGroup = new QGroupBox(i18n("Claude API Settings"), providerTab);
    auto* claudeForm = new QFormLayout(claudeGroup);

    modelEdit = new QLineEdit(claudeGroup);
    modelEdit->setObjectName(QStringLiteral("kcfg_claudeModel"));
    claudeForm->addRow(i18n("Model:"), modelEdit);

    apiKeyEdit = new QLineEdit(claudeGroup);
    apiKeyEdit->setEchoMode(QLineEdit::Password);
    apiKeyEdit->setPlaceholderText(i18n("Enter API key…"));
    claudeForm->addRow(i18n("API Key:"), apiKeyEdit);

    providerLayout->addWidget(claudeGroup);

    localLabel = new QLabel(i18n("Local AI — coming soon"), providerTab);
    localLabel->setEnabled(false);
    providerLayout->addWidget(localLabel);

    openAiGroup = new QGroupBox(i18n("OpenAI-Compatible Settings"), providerTab);
    auto* oaForm = new QFormLayout(openAiGroup);

    openAiEndpointEdit = new QLineEdit(openAiGroup);
    openAiEndpointEdit->setObjectName(QStringLiteral("kcfg_openAiCompatibleEndpoint"));
    openAiEndpointEdit->setPlaceholderText(u"https://api.openai.com/v1"_s);
    oaForm->addRow(i18n("Endpoint:"), openAiEndpointEdit);

    openAiModelEdit = new QLineEdit(openAiGroup);
    openAiModelEdit->setObjectName(QStringLiteral("kcfg_openAiCompatibleModel"));
    oaForm->addRow(i18n("Model:"), openAiModelEdit);

    openAiKeyEdit = new QLineEdit(openAiGroup);
    openAiKeyEdit->setEchoMode(QLineEdit::Password);
    openAiKeyEdit->setPlaceholderText(i18n("API key (leave empty for local servers)"));
    oaForm->addRow(i18n("API Key:"), openAiKeyEdit);

    providerLayout->addWidget(openAiGroup);
    providerLayout->addStretch();

    // --- Source Extraction tab ---
    auto* promptsTab = new QWidget(tabs);
    auto* promptsLayout = new QVBoxLayout(promptsTab);
    tabs->addTab(promptsTab, i18n("Source Extraction"));

    auto* promptsForm = new QFormLayout;
    sourceExtractionEdit = new QPlainTextEdit(promptsTab);
    sourceExtractionEdit->setObjectName(QStringLiteral("kcfg_sourceExtractionPrompt"));
    promptsForm->addRow(i18n("Source Extraction:"), sourceExtractionEdit);

    exampleJsonEdit = new QPlainTextEdit(promptsTab);
    exampleJsonEdit->setReadOnly(true);
    exampleJsonEdit->setPlainText(uR"({
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "type": "object",
  "properties": {
    "title":       { "type": "string" },
    "type":        { "type": "string", "enum": ["book","article","census","vital_record","church_record","newspaper","photograph","other"] },
    "author":      { "type": "string" },
    "publication": { "type": "string" },
    "confidence":  { "type": "string", "enum": ["low","medium","high"] },
    "note":        { "type": "string" },
    "message":     { "type": "string", "description": "Optional note about ambiguous or missing fields" }
  },
  "required": ["title","type","confidence"]
})"_s);
    promptsForm->addRow(i18n("Expected JSON Schema:"), exampleJsonEdit);

    promptsLayout->addLayout(promptsForm);

    connect(providerCombo, &QComboBox::currentIndexChanged, this, &AiSettingsWidget::onProviderChanged);

    // Show/hide groups for the current setting.
    onProviderChanged(providerCombo->currentIndex());

    // Asynchronously pre-populate the API key field from the keychain.
    loadApiKey();
}

void AiSettingsWidget::onProviderChanged(int index) {
    // Enum order: 0 = None, 1 = Claude, 2 = Local, 3 = OpenAiCompatible
    claudeGroup->setVisible(index == 1);
    localLabel->setVisible(index == 2);
    openAiGroup->setVisible(index == 3);
}

void AiSettingsWidget::loadApiKey() {
    auto* claudeJob = new QKeychain::ReadPasswordJob(KeychainKeys::Service, this);
    claudeJob->setKey(KeychainKeys::ClaudeApiKey);
    claudeJob->setAutoDelete(true);
    connect(claudeJob, &QKeychain::ReadPasswordJob::finished, this, [this, claudeJob] {
        if (claudeJob->error() == QKeychain::NoError) {
            apiKeyEdit->setText(claudeJob->textData());
        }
    });
    claudeJob->start();

    auto* openAiJob = new QKeychain::ReadPasswordJob(KeychainKeys::Service, this);
    openAiJob->setKey(KeychainKeys::OpenAiCompatibleApiKey);
    openAiJob->setAutoDelete(true);
    connect(openAiJob, &QKeychain::ReadPasswordJob::finished, this, [this, openAiJob] {
        if (openAiJob->error() == QKeychain::NoError) {
            openAiKeyEdit->setText(openAiJob->textData());
        }
    });
    openAiJob->start();
}

void AiSettingsWidget::saveApiKey() {
    auto* claudeJob = new QKeychain::WritePasswordJob(KeychainKeys::Service, this);
    claudeJob->setKey(KeychainKeys::ClaudeApiKey);
    claudeJob->setTextData(apiKeyEdit->text());
    claudeJob->setAutoDelete(true);
    claudeJob->start();

    auto* openAiJob = new QKeychain::WritePasswordJob(KeychainKeys::Service, this);
    openAiJob->setKey(KeychainKeys::OpenAiCompatibleApiKey);
    openAiJob->setTextData(openAiKeyEdit->text());
    openAiJob->setAutoDelete(true);
    openAiJob->start();
}
