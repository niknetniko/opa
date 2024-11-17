/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "note_editor_window.h"

#include <KActionCollection>
#include <KRichTextWidget>
#include <QDialogButtonBox>
#include <QToolBar>
#include <QVBoxLayout>

NoteEditorWindow::NoteEditorWindow(const QString& text, QWidget* parent) : QDialog(parent) {
    setWindowModality(Qt::WindowModal);

    auto* toolbar = new QToolBar(this);
    textEditor = new KRichTextWidget(this);
    textEditor->setTextOrHtml(text);
    textEditor->setRichTextSupport(KRichTextWidget::FullSupport);
    textEditor->setCheckSpellingEnabled(true);
    auto actions = textEditor->createActions();

    auto* allActions = new KActionCollection(this);
    allActions->addActions(actions);
    toolbar->addAction(allActions->action(QStringLiteral("format_text_bold")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_italic")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_underline")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_strikeout")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_foreground_color")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_background_color")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_subscript")));
    toolbar->addAction(allActions->action(QStringLiteral("format_text_superscript")));
    toolbar->addSeparator();
    toolbar->addAction(allActions->action(QStringLiteral("format_font_family")));
    toolbar->addAction(allActions->action(QStringLiteral("format_font_size")));
    toolbar->addAction(allActions->action(QStringLiteral("format_heading_level")));
    toolbar->addSeparator();
    toolbar->addAction(allActions->action(QStringLiteral("format_align_left")));
    toolbar->addAction(allActions->action(QStringLiteral("format_align_center")));
    toolbar->addAction(allActions->action(QStringLiteral("format_align_right")));
    toolbar->addAction(allActions->action(QStringLiteral("format_align_justify")));
    toolbar->addSeparator();
    toolbar->addAction(allActions->action(QStringLiteral("format_list_style")));
    toolbar->addAction(allActions->action(QStringLiteral("format_list_indent_more")));
    toolbar->addAction(allActions->action(QStringLiteral("format_list_indent_less")));
    toolbar->addAction(allActions->action(QStringLiteral("insert_horizontal_rule")));
    toolbar->addSeparator();
    toolbar->addAction(allActions->action(QStringLiteral("manage_link")));
    toolbar->addAction(allActions->action(QStringLiteral("format_painter")));
    toolbar->addSeparator();
    toolbar->addAction(allActions->action(QStringLiteral("direction_ltr")));
    toolbar->addAction(allActions->action(QStringLiteral("direction_rtl")));
    toolbar->addSeparator();
    toolbar->addAction(allActions->action(QStringLiteral("action_to_plain_text")));

    auto* buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel | QDialogButtonBox::StandardButton::Ok);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* nestedLayout = new QVBoxLayout;
    nestedLayout->addWidget(textEditor);
    nestedLayout->addWidget(buttonBox);

    auto* layout = new QVBoxLayout;
    layout->setMenuBar(toolbar);

    layout->addLayout(nestedLayout);
    setLayout(layout);
    auto defaultContentMargins = layout->contentsMargins();
    // Needed to make the menu bar look good.
    layout->setContentsMargins(0, 0, 0, 0);
    nestedLayout->setContentsMargins(defaultContentMargins);
}
QString NoteEditorWindow::editText(const QString& initialText, const QString& windowTitle, QWidget* parent) {
    NoteEditorWindow noteEditor{initialText, parent};
    noteEditor.setWindowTitle(windowTitle);

    if (noteEditor.exec() == Accepted) {
        return noteEditor.textEditor->textOrHtml();
    } else {
        return QStringLiteral();
    }
}
