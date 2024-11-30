
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QDialog>

class KRichTextWidget;

/**
 * An editor for notes.
 */
class NoteEditorDialog : public QDialog {
    Q_OBJECT

public:
    NoteEditorDialog(const QString& text, QWidget* parent);

    static QString editText(const QString& initialText, const QString& windowTitle, QWidget* parent);

private:
    KRichTextWidget* textEditor;
};
