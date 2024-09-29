#ifndef PERSONDETAILVIEW_H
#define PERSONDETAILVIEW_H

#include <QFrame>
#include <QSqlRecord>
#include <QAbstractProxyModel>
#include <QStyledItemDelegate>
#include "database/schema.h"
#include "utils/formatted_identifier_delegate.h"

namespace Ui {
    class PersonDetailView;
}

class PersonDetailView : public QFrame {
Q_OBJECT

public:
    explicit PersonDetailView(IntegerPrimaryKey id, QWidget *parent);

    bool hasId(IntegerPrimaryKey id);

    QString getDisplayName();

    ~PersonDetailView() override;

public Q_SLOTS:

    /**
     * Populate the UI with the data from the field.
     */
    void populate();

Q_SIGNALS:

    /**
     * Emitted when the UI has been re-populated.
     */
    void dataChanged(IntegerPrimaryKey id);

private:
    QAbstractProxyModel *model;
    Ui::PersonDetailView *ui;
};

#endif // PERSONDETAILVIEW_H
