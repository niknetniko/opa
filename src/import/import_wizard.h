/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "ai/ai_service.h"
#include "gramps_xml.h"

#include <KBusyIndicatorWidget>
#include <KUrlRequester>
#include <QFutureWatcher>
#include <QLabel>
#include <QRadioButton>
#include <QWizardPage>

class ImportWizard : public QWizard {
    Q_OBJECT
public:
    enum { Page_Intro, Page_GrampsFileSelect, Page_GrampsCheck, Page_GrampsImport, Page_GrampsConclude };

    explicit ImportWizard(QWidget* parent = nullptr);
};

class IntroPage : public QWizardPage {
    Q_OBJECT
public:
    explicit IntroPage(QWidget* parent = nullptr);

    int nextId() const override;

private:
    QRadioButton* grampsXmlRadioButton;
};


class GrampsXmlSelectPage : public QWizardPage {
    Q_OBJECT
public:
    explicit GrampsXmlSelectPage(QWidget* parent = nullptr);

    int nextId() const override;

private:
    KUrlRequester* urlRequester;
};

class GrampsCheckPage : public QWizardPage {
    Q_OBJECT
public:
    explicit GrampsCheckPage(QWidget* parent = nullptr);

    void initializePage() override;
    void cleanupPage() override;
    bool isComplete() const override;

    int nextId() const override;

private:
    KBusyIndicatorWidget* busyIndicator;
    QLabel* busyText;
    QLabel* resultLabel;

    QString analyzedFile;
    QFutureWatcher<GrampsXmlAnalysis>* watcher_ = nullptr;
    GrampsXmlAnalysis analysis_;

    void setBusy(bool busy) const;

private Q_SLOTS:
    void onFinished();
};
