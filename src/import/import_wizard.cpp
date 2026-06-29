/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "import_wizard.h"

#include "utils/resource_exception.h"

#include <KLocalizedString>
#include <QLabel>
#include <QStringLiteral>
#include <QVBoxLayout>
#include <QtConcurrent>

using namespace Qt::StringLiterals;


ImportWizard::ImportWizard(QWidget* parent) : QWizard(parent) {
    setPage(Page_Intro, new IntroPage);
    setPage(Page_GrampsFileSelect, new GrampsXmlSelectPage);
    setPage(Page_GrampsCheck, new GrampsCheckPage);

    setStartId(Page_Intro);

    setWindowTitle(i18n("Import data"));
}


IntroPage::IntroPage(QWidget* parent) : QWizardPage(parent) {
    setTitle(i18n("Choose data type"));

    auto *topLabel = new QLabel(i18n("This wizard will help you import data into Gramps."));
    auto *secondLabel = new QLabel(i18n("Select the type of data to import:"));

    grampsXmlRadioButton = new QRadioButton(i18n("Gramps XML file"));
    grampsXmlRadioButton->setChecked(true);

    auto *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(secondLabel);
    layout->addWidget(grampsXmlRadioButton);
    setLayout(layout);
}

int IntroPage::nextId() const {
    if (grampsXmlRadioButton->isChecked()) {
        return ImportWizard::Page_GrampsFileSelect;
    }

    return QWizardPage::nextId();
}

GrampsXmlSelectPage::GrampsXmlSelectPage(QWidget* parent) : QWizardPage(parent) {
    setTitle(i18n("Choose data source"));

    auto *topLabel = new QLabel(i18n("Select which Gramps XML file should be imported."));

    urlRequester = new KUrlRequester(this);
    urlRequester->setMode(KFile::File | KFile::ExistingOnly);
    urlRequester->setNameFilter(i18n("Gramps XML") + u" (*.gramps *.xml)"_s);
    urlRequester->setPlaceholderText(i18n("Select a Gramps XML file…"));

    auto *bottomLabel = new QLabel(i18n(
        "Opa supports Gramps XML up to Gramps version 6.0.x (XML schema 1.7.2). It does not support gpkg files at the "
        "moment."
    ));
    bottomLabel->setWordWrap(true);

    registerField(u"grampsXmlFile*"_s, urlRequester, "url", SIGNAL(urlSelected(QUrl)));

    auto *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    layout->addWidget(urlRequester);
    layout->addWidget(bottomLabel);
    setLayout(layout);
}

int GrampsXmlSelectPage::nextId() const {
    return ImportWizard::Page_GrampsCheck;
}

GrampsCheckPage::GrampsCheckPage(QWidget* parent) : QWizardPage(parent) {
    setTitle(i18n("Checking data"));

    busyIndicator = new KBusyIndicatorWidget;
    busyIndicator->hide();
    busyText = new QLabel(i18n("Validating Gramps XML file..."));
    busyText->hide();

    resultLabel = new QLabel;
    resultLabel->setWordWrap(true);
    resultLabel->hide();

    auto *layout = new QVBoxLayout;
    layout->addWidget(busyIndicator);
    layout->addWidget(busyText);
    layout->addWidget(resultLabel);
    setLayout(layout);
}

void GrampsCheckPage::initializePage() {
    auto file = field(u"grampsXmlFile"_s).toUrl().toLocalFile();

    if (analyzedFile == file && watcher_ == nullptr) {
        return; // Already analyzed
    }

    analyzedFile = file;
    analysis_ = {};
    setBusy(true);

    Q_EMIT completeChanged();

    watcher_ = new QFutureWatcher<GrampsXmlAnalysis>(this);
    connect(watcher_, &QFutureWatcher<GrampsXmlAnalysis>::finished, this, &GrampsCheckPage::onFinished);
    watcher_->setFuture(QtConcurrent::run(validateGrampsXml, file));
}

void GrampsCheckPage::cleanupPage() {
    QWizardPage::cleanupPage();

    if (watcher_) {
        watcher_->cancel();
        watcher_->disconnect(this);
        watcher_->deleteLater();
        watcher_ = nullptr;
    }

    setBusy(false);
    analyzedFile.clear();
}

bool GrampsCheckPage::isComplete() const {
    return analysis_.valid;
}

int GrampsCheckPage::nextId() const {
    return QWizardPage::nextId();
}

void GrampsCheckPage::setBusy(bool busy) const {
    busyIndicator->setVisible(busy);
    busyText->setVisible(busy);
    resultLabel->setVisible(!busy);

    if (!busy) {
        if (analysis_.valid) {
            resultLabel->setText(i18n(
                "This file is valid and contains:\n"
                "%1 people, %2 families, %3 events, "
                "%4 sources and %5 places.",
                analysis_.people,
                analysis_.families,
                analysis_.events,
                analysis_.sources,
                analysis_.places
            ));
        } else {
            resultLabel->setText(i18n("This file cannot be imported:\n%1", analysis_.error));
        }
    }
}

void GrampsCheckPage::onFinished() {
    try {
        analysis_ = watcher_->result();
    } catch (const ResourceNotFoundException&) {
        analysis_.valid = false;
    }
    watcher_->deleteLater();
    watcher_ = nullptr;
    setBusy(false);

    Q_EMIT completeChanged();
}
