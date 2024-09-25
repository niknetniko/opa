//
// Created by niko on 8/02/24.
//

#include <QDialog>
#include <QQuickWidget>
#include <QSqlQuery>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QSqlError>
#include <QCompleter>
#include <QString>
#include <QDialogButtonBox>
#include <QItemEditorFactory>
#include <QSqlRelationalDelegate>
#include "nameEditor.h"
#include "ui_nameEditor.h"
#include "names.h"
#include "data/names.h"


NamesEditor::NamesEditor(QAbstractItemModel *model, QWidget *parent) : QDialog(parent) {
    this->model = model;

    auto *form = new Ui::NameEditorForm();
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept); // NOLINT(*-unused-return-value)
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject); // NOLINT(*-unused-return-value)

    // Set up the name origin combobox.
    auto* originCompleterModel = new NameOriginTableModel(this);
    originCompleterModel->setSort(NameOriginTableModel::ORIGIN, Qt::SortOrder::AscendingOrder);
    originCompleterModel->select();
    form->origin->setModel(originCompleterModel);
    form->origin->setModelColumn(NameOriginTableModel::ORIGIN);
    form->origin->setItemDelegate(new QSqlRelationalDelegate(form->origin));

    // Set up autocomplete on the last name.
    // We want to sort this, so we need to create a new model.
    // Additionally, we want to use all last names, not just the once from the current person.
//    auto* surnameAutocompleteModel = new NamesTableModel(this);
//    surnameAutocompleteModel->setSort(NamesTableModel::SURNAME, Qt::SortOrder::AscendingOrder);
//    surnameAutocompleteModel->select();
//    auto* surnameCompleter = new QCompleter(surnameAutocompleteModel, this);
//    surnameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//    surnameCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
//    surnameCompleter->setCompletionColumn(NamesTableModel::SURNAME);
//    surnameCompleter->setCompletionMode(QCompleter::PopupCompletion);
//    form->surname->setCompleter(surnameCompleter);

    // Set up autocomplete on the given names.
    // This is very basic: it will not tokenize the names.
//    auto* givenNamesAutocompleteModel = new NamesTableModel(this);
//    givenNamesAutocompleteModel->setSort(NamesTableModel::GIVEN_NAMES, Qt::SortOrder::AscendingOrder);
//    givenNamesAutocompleteModel->select();
//    auto* givenNameCompleter = new QCompleter(givenNamesAutocompleteModel, this);
//    givenNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);
//    givenNameCompleter->setModelSorting(QCompleter::CaseSensitivelySortedModel);
//    givenNameCompleter->setCompletionColumn(NamesTableModel::GIVEN_NAMES);
//    givenNameCompleter->setCompletionMode(QCompleter::PopupCompletion);
//    form->givenNames->setCompleter(givenNameCompleter);

    // Map the data from the database to the form.
    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(this->model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->addMapping(form->titles, NamesTableModel::TITLES);
    mapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    mapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    mapper->addMapping(form->surname, NamesTableModel::SURNAME);
    mapper->addMapping(form->origin, NamesTableModel::ORIGIN_ID);
    mapper->addMapping(form->primaryName, NamesTableModel::MAIN);

    // Ensure we show the correct row.
    mapper->setCurrentIndex(0);
}

void NamesEditor::accept() {
    // Attempt to submit the mapper changes.
    // TODO: fix this somehow?
    if (this->mapper->submit()) {
        // We are done.
        QDialog::accept();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("Problem during saving"), QString::fromUtf8("The changes could not be saved for some reason"));
        // TODO: how should errors be propagated?
        //        qDebug() << "Error was: " << this->model->lastError();
        //        qDebug() << "Query was: " << this->model->query().lastQuery();
        return;
    }
}

void NamesEditor::reject() {
    this->model->revert();
    QDialog::reject();
}
