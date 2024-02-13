//
// Created by niko on 8/02/24.
//

#include <QQuickWidget>
#include <QSqlQuery>
#include <QDataWidgetMapper>
#include <QMessageBox>
#include <QSqlError>
#include <QCompleter>
#include "nameEditor.h"
#include "ui_nameEditor.h"
#include "names.h"


NamesEditor::NamesEditor(NamesTableModel *model, int selectedRow, QWidget *parent) : QDialog(parent) {
    this->model = model;

    auto *form = new Ui::NameEditorForm();
    form->setupUi(this);

    // Connect the buttons.
    connect(form->dialogButtons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(form->dialogButtons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    this->mapper = new QDataWidgetMapper(this);
    mapper->setModel(this->model);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->addMapping(form->titles, NamesTableModel::TITLES);
    mapper->addMapping(form->givenNames, NamesTableModel::GIVEN_NAMES);
    mapper->addMapping(form->prefix, NamesTableModel::PREFIX);
    mapper->addMapping(form->surname, NamesTableModel::SURNAME);
    mapper->addMapping(form->originCombo, NamesTableModel::ORIGIN);
    mapper->setCurrentIndex(selectedRow);

    // Set up autocomplete on the last name.
    // We want to sort this, so we need to create a new model.
    // Additionally, we want to use all last names, not just the once from the current person.
    auto* autocompleteModel = new NamesTableModel(this);
    autocompleteModel->setSort(NamesTableModel::SURNAME, Qt::SortOrder::AscendingOrder);
    autocompleteModel->select();
    auto* completer = new QCompleter(autocompleteModel, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setCompletionColumn(NamesTableModel::SURNAME);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    form->surname->setCompleter(completer);
}

void NamesEditor::accept() {
    // Attempt to submit the mapper changes.
    if (this->mapper->submit() && this->model->submitAll()) {
        // We are done.
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "Problem during saving", "The changes could not be saved for some reason");
        qDebug() << "Error was: " << this->model->lastError();
        qDebug() << "Query was: " << this->model->query().lastQuery();
        return;
    }
}

void NamesEditor::reject() {
    this->model->revertAll();
    QDialog::reject();
}
