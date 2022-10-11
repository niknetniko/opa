//
// Created by niko on 2/09/2022.
//

#ifndef OPA_PERSON_DETAIL_VIEW_H
#define OPA_PERSON_DETAIL_VIEW_H

#include <QtWidgets/QWidget>
#include <QSqlRecord>
#include <QLabel>

/**
 * Detail view for a single person.
 *
 * This will display all details of a single person.
 *
 * Note: this will not automatically load the data at the moment.
 */
class PersonDetailView: public QWidget {
    Q_OBJECT;

public:
    const int id;
    PersonDetailView(int id, QWidget* parent);
    void populate();

Q_SIGNALS:
    void personNameChanged(int personId, const QString& newName);

private:
    QSqlRecord record;
    QLabel *displayName;
};

#endif //OPA_PERSON_DETAIL_VIEW_H
