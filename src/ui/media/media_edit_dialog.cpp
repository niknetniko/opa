/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_edit_dialog.h"

#include "domain/media/media_repository.h"
#include "domain/media/media_service.h"

#include <KIO/OpenFileManagerWindowJob>
#include <KLocalizedString>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPdfDocument>
#include <QPdfView>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

MediaEditDialog::MediaEditDialog(const MediaEntity& entity, QWidget* parent) : QDialog(parent), mediaId(entity.id) {

    if (MediaService::isActive()) {
        absolutePath = MediaService::instance().resolveAbsolutePath(entity.path);
    }

    setWindowTitle(i18n("Edit Media"));
    setMinimumWidth(500);

    auto* formLayout = new QFormLayout;

    titleEdit = new QLineEdit(this);
    titleEdit->setPlaceholderText(i18n("Optional title"));
    if (entity.title.has_value()) {
        titleEdit->setText(*entity.title);
    }
    formLayout->addRow(i18n("Title:"), titleEdit);

    noteEdit = new QPlainTextEdit(this);
    if (entity.note.has_value()) {
        noteEdit->setPlainText(*entity.note);
    }
    formLayout->addRow(i18n("Note:"), noteEdit);

    auto* pathLabel = new QLabel(entity.path, this);
    pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pathLabel->setWordWrap(true);

    auto* openButton = new QToolButton(this);
    openButton->setIcon(QIcon::fromTheme(u"document-open"_s));
    openButton->setToolTip(i18n("Open file"));
    openButton->setEnabled(!absolutePath.isEmpty());
    connect(openButton, &QToolButton::clicked, this, &MediaEditDialog::openFile);

    auto* folderButton = new QToolButton(this);
    folderButton->setIcon(QIcon::fromTheme(u"folder-open"_s));
    folderButton->setToolTip(i18n("Show in folder"));
    folderButton->setEnabled(!absolutePath.isEmpty());
    connect(folderButton, &QToolButton::clicked, this, &MediaEditDialog::showInFolder);

    auto* pathRow = new QHBoxLayout;
    pathRow->addWidget(pathLabel, 1);
    pathRow->addWidget(openButton);
    pathRow->addWidget(folderButton);
    formLayout->addRow(i18n("File:"), pathRow);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);

    // Preview section — images and PDFs are rendered inline.
    if (!absolutePath.isEmpty()) {
        if (entity.mimeType.startsWith(u"image/"_s)) {
            QPixmap pixmap(absolutePath);
            if (!pixmap.isNull()) {
                auto* previewLabel = new QLabel(this);
                previewLabel->setAlignment(Qt::AlignCenter);
                constexpr int maxPreviewHeight = 300;
                previewLabel->setPixmap(
                    pixmap.scaledToHeight(std::min(pixmap.height(), maxPreviewHeight), Qt::SmoothTransformation)
                );
                auto* scroll = new QScrollArea(this);
                scroll->setWidget(previewLabel);
                scroll->setWidgetResizable(true);
                scroll->setMinimumHeight(std::min(pixmap.height(), maxPreviewHeight) + 4);
                layout->addWidget(scroll);
            }
        } else if (entity.mimeType == u"application/pdf"_s) {
            auto* doc = new QPdfDocument(this);
            if (doc->load(absolutePath) == QPdfDocument::Error::None) {
                auto* pdfView = new QPdfView(this);
                pdfView->setDocument(doc);
                pdfView->setPageMode(QPdfView::PageMode::SinglePage);
                pdfView->setZoomMode(QPdfView::ZoomMode::FitInView);
                pdfView->setFixedHeight(300);
                layout->addWidget(pdfView);
            }
        }
    }

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &MediaEditDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &MediaEditDialog::reject);
    layout->addWidget(buttons);
}

void MediaEditDialog::accept() {
    const auto titleText = titleEdit->text().trimmed();
    const std::optional<QString> title = titleText.isEmpty() ? std::nullopt : std::make_optional(titleText);

    const auto noteText = noteEdit->toPlainText().trimmed();
    const std::optional<QString> note = noteText.isEmpty() ? std::nullopt : std::make_optional(noteText);

    MediaRepository repo;
    if (!repo.update(mediaId, title, note)) {
        qWarning() << "Could not update media" << mediaId;
        return;
    }

    QDialog::accept();
}

void MediaEditDialog::openFile() const {
    QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath));
}

void MediaEditDialog::showInFolder() const {
    KIO::highlightInFileManager({QUrl::fromLocalFile(absolutePath)});
}
