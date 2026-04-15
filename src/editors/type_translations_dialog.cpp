/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "type_translations_dialog.h"

#include "ui_type_translations_dialog.h"

#include "database/database.h"

#include <KLocalizedString>
#include <QLocale>
#include <QStandardItemModel>

using namespace Qt::StringLiterals;

static constexpr int LOCALE_CODE_ROLE = Qt::UserRole;
static constexpr int ROW_ID_ROLE = Qt::UserRole + 1;

TypeTranslationsDialog::TypeTranslationsDialog(
    const QString& typeName,
    std::function<QList<TranslationEntry>()> loadFn,
    std::function<std::optional<IntegerPrimaryKey>(const QString& locale, const QString& name)> insertFn,
    std::function<bool(IntegerPrimaryKey id)> removeFn,
    QWidget* parent
) :
    QDialog(parent),
    ui(new Ui::TypeTranslationsDialog),
    tableModel(new QStandardItemModel(0, 2, this)),
    loadFn(std::move(loadFn)),
    insertFn(std::move(insertFn)),
    removeFn(std::move(removeFn)) {
    ui->setupUi(this);
    setWindowTitle(i18n("Translations for %1", typeName));

    // Insert KLanguageButton before the name edit in the toolbar row
    languageButton = new KLanguageButton(this);
    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();
    ui->toolbarLayout->insertWidget(0, languageButton);

    // Style the add/remove buttons as icon-only toolbar-style buttons
    ui->addButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    ui->addButton->setToolTip(i18n("Add translation"));
    ui->addButton->setFlat(true);
    ui->removeButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    ui->removeButton->setToolTip(i18n("Remove selected translation"));
    ui->removeButton->setFlat(true);

    tableModel->setHorizontalHeaderLabels({i18n("Language"), i18n("Name")});
    ui->translationsTable->setModel(tableModel);
    ui->translationsTable->horizontalHeader()->setStretchLastSection(true);
    ui->translationsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->translationsTable->verticalHeader()->hide();
    ui->translationsTable->setShowGrid(false);

    connect(ui->addButton, &QPushButton::clicked, this, &TypeTranslationsDialog::addTranslation);
    connect(ui->removeButton, &QPushButton::clicked, this, &TypeTranslationsDialog::removeTranslation);
    connect(
        ui->translationsTable->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        [this](const QItemSelection& selected, const QItemSelection&) {
            ui->removeButton->setEnabled(!selected.isEmpty());
        }
    );
    connect(tableModel, &QStandardItemModel::itemChanged, this, &TypeTranslationsDialog::onItemChanged);

    reload();
}

TypeTranslationsDialog::~TypeTranslationsDialog() {
    delete ui;
}

void TypeTranslationsDialog::reload() {
    QSignalBlocker blocker(tableModel);
    tableModel->removeRows(0, tableModel->rowCount());
    for (const auto& entry : loadFn()) {
        auto* localeItem = new QStandardItem(formatLocale(entry.locale));
        localeItem->setData(entry.locale, LOCALE_CODE_ROLE);
        localeItem->setData(entry.id, ROW_ID_ROLE);
        localeItem->setFlags(localeItem->flags() & ~Qt::ItemIsEditable);
        auto* nameItem = new QStandardItem(entry.name);
        tableModel->appendRow({localeItem, nameItem});
    }
}

void TypeTranslationsDialog::addTranslation() {
    const auto locale = languageButton->current();
    const auto name = ui->nameEdit->text().trimmed();
    if (locale.isEmpty() || name.isEmpty()) {
        return;
    }
    // Find any existing translation for this locale to replace atomically.
    std::optional<IntegerPrimaryKey> existingId;
    for (int row = 0; row < tableModel->rowCount(); ++row) {
        if (tableModel->item(row, 0)->data(LOCALE_CODE_ROLE).toString() == locale) {
            existingId = tableModel->item(row, 0)->data(ROW_ID_ROLE).toLongLong();
            break;
        }
    }
    const auto ok = executeInTransaction([&]() -> std::optional<bool> {
        if (existingId && !removeFn(*existingId)) return std::nullopt;
        if (!insertFn(locale, name)) return std::nullopt;
        return true;
    });
    if (ok) {
        ui->nameEdit->clear();
        reload();
    }
}

void TypeTranslationsDialog::removeTranslation() {
    const auto selected = ui->translationsTable->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    const auto row = selected.constFirst().row();
    const auto id = tableModel->item(row, 0)->data(ROW_ID_ROLE).toLongLong();
    if (removeFn(id)) {
        reload();
    }
}

void TypeTranslationsDialog::onItemChanged(QStandardItem* item) {
    if (item->column() != 1) {
        return;
    }
    const auto* localeItem = tableModel->item(item->row(), 0);
    const auto id = localeItem->data(ROW_ID_ROLE).toLongLong();
    const auto locale = localeItem->data(LOCALE_CODE_ROLE).toString();
    const auto newName = item->text().trimmed();
    if (newName.isEmpty()) {
        return;
    }
    // Update by removing the old row and inserting a new one with the same locale.
    const auto ok = executeInTransaction([&]() -> std::optional<bool> {
        if (!removeFn(id)) return std::nullopt;
        if (!insertFn(locale, newName)) return std::nullopt;
        return true;
    });
    if (ok) {
        reload();
    }
}

QString TypeTranslationsDialog::formatLocale(const QString& localeCode) {
    const QLocale locale(localeCode);
    const auto nativeName = locale.nativeLanguageName();
    if (nativeName.isEmpty()) {
        return localeCode;
    }
    return u"%1 (%2)"_s.arg(nativeName, localeCode);
}
