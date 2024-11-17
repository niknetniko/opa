
/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <KXmlGuiWindow>
#include <QDialog>

class KRichTextWidget;

/**
 * A "dialog" which allows editing some text.
 */
class NoteEditorWindow : public QDialog {
    Q_OBJECT

public:
    NoteEditorWindow(const QString& text, QWidget* parent);

    static QString editText(const QString& initialText, const QString& windowTitle, QWidget* parent);

private:
    KRichTextWidget* textEditor;
};
