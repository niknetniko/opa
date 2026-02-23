# Architecture Guide & Coding Standards

This document outlines the architectural patterns and coding standards for the application. All AI agents and developers must adhere to these guidelines when refactoring existing code or adding new features.

## 1. Core Philosophy: The Repository Pattern

We are migrating away from the "Global `QSqlTableModel` + Proxy Chain" pattern.
**Do not use `QSqlTableModel` for domain logic or complex views.**

### The New Standard
The application is layered as follows:
1.  **Database (SQL)**: Normalized SQLite schema.
2.  **Repository Layer**: Executes SQL, handles `JOIN`s, and returns strongly-typed C++ structs (Entities).
3.  **Service/Model Layer**: Generic `ObjectListModel<T>` wrappers that expose Entities to QML/Widgets.
4.  **View Layer**: QML or standard Qt Views.

---

## 2. Entity Definitions (DTOs)

Entities are simple C++ structs representing data. They must:
* Be **Plain Old Data (POD)** or simple structs with helper methods.
* Include a static `fromSql(const QSqlQuery&)` factory method to centralize mapping.
* **Not** inherit from `QObject` (to allow storage in `QList` or `std::vector`).

**Example:**
```c++
// family_entities.h
struct Person {
    int id = -1;
    QString givenName;
    QString surname;

    // Standard factory method
    static Person fromSql(const QSqlQuery& query) {
        Person p;
        p.id = query.value("id").toInt();
        p.givenName = query.value("given_names").toString();
        p.surname = query.value("surname").toString();
        return p;
    }
};
```

## 3. Repositories

Repositories are responsible for **all** SQL execution. 
* **Input**: Primitive types (IDs, strings) or Criteria structs.
* **Output**: `QList<Entity>`, `std::optional<Entity>`, or `void`.
* **No Qt Models**: Repositories must NEVER return `QAbstractItemModel` or `QSqlTableModel`.

### Base Class
All repositories should inherit from `BaseRepository` (or equivalent) to access helper methods for executing queries and broadcasting changes.

### Query Construction
* Use raw SQL for complex queries (CTEs, recursive joins).
* Use the `QueryHelper` utility for dynamic filtering/sorting (WHERE, ORDER BY, LIMIT).

**Example:**
```c++
// person_repository.cpp
QList<Person> PersonRepository::findPeople(const PersonCriteria& criteria) {
    QString sql = "SELECT * FROM people p JOIN names n ON p.id = n.person_id";
    QVariantMap bindings;
    
    // Helper injects WHERE/ORDER BY clauses automatically
    QueryHelper::applyCriteria(sql, bindings, criteria);
    
    return fetchAll<Person>(sql, bindings);
}
```

## 4. View Models (The `ObjectListModel`)

Do not write custom `QAbstractListModel` subclasses for every entity. Use the generic `ObjectListModel<T>`.

* **Setup**: Instantiate `ObjectListModel<MyEntity>` in the Controller/Widget.
* **Mapping**: Use lambdas to map Entity fields to Qt Roles.

**Example usage in a Widget/Controller:**
```c++
void FamilyController::loadPeople() {
    auto people = m_repository.findPeople({});
    
    auto* model = new ObjectListModel<Person>(this);
    model->setItems(people);
    
    // Define roles for the View
    model->addRole(Qt::DisplayRole, "name", [](const Person& p){ return p.surname; });
    
    m_view->setModel(model);
}
```

---

## 5. Sorting and Filtering

* **Database Side**: Preferred for large lists. Use `Criteria` structs passed to the Repository.
* **View Side**: Preferred for small lists (< 1000 items). Use `QSortFilterProxyModel` wrapping the `ObjectListModel`.

**Do not implement manual sorting logic inside the Custom Model.**

---

## 6. Change Notifications (The Event Bus)

We use a centralized **Domain Event Broker** to signal data changes, rather than relying on SQL table signals.

1.  **Write**: When a Repository modifies data (INSERT/UPDATE), it **must** call the broker.
    ```c++
    // Inside Repository
    if (query.exec()) {
        DomainEventBroker::instance().notifyPersonChanged(personId);
    }
    ```

2.  **Read**: UI Components or Models listen to the broker to refresh specific data.
    ```c++
    // Inside a ViewModel or Widget
    connect(&DomainEventBroker::instance(), &DomainEventBroker::personChanged, 
            this, [this](int id) {
        if (id == m_currentPersonId) refresh();
    });
    ```

---

## 7. Migration Checklist for AI Agents

When asked to refactor a feature (e.g., "Move Sources to the new architecture"):

1.  [ ] **Define Entity**: Create `struct Source` in `source_entities.h`.
2.  [ ] **Create Repository**: Create `SourceRepository` with methods like `getSourcesForPerson(int id)`.
3.  [ ] **Remove Old Logic**: Delete `DataManager::sourcesModel()` and related Proxy classes.
4.  [ ] **Update UI**: Replace the `QTreeView` model setup with `ObjectListModel<Source>` populated by the Repository.
5.  [ ] **Add Notifications**: Ensure saves trigger `notifySourceChanged(id)`.

## 8. Appendix: Core Utility Classes

Add these files to your `utils/` folder to enable the architecture described above.

### A. `utils/query_helper.h`
*Handles dynamic SQL generation for sorting and filtering.*

```c++
#pragma once
#include <QString>
#include <QVariantMap>
#include <QList>

struct QueryCriteria {
    QString filterText;       // General search text (e.g., "John")
    QString sortColumn;       // Database column (e.g., "surname")
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    int limit = -1;
    int offset = 0;
    QVariantMap filters;      // Specific filters: { "type_id": 2 }
};

class QueryHelper {
public:
    static void applyCriteria(QString& sql, QVariantMap& bindings, const QueryCriteria& criteria, 
                              const QString& defaultSearchCol = "name") {
        QStringList whereClauses;

        // 1. Generic Search
        if (!criteria.filterText.isEmpty()) {
            whereClauses << QString("%1 LIKE :search_text").arg(defaultSearchCol);
            bindings[":search_text"] = "%" + criteria.filterText + "%";
        }

        // 2. Exact Filters
        for (auto it = criteria.filters.begin(); it != criteria.filters.end(); ++it) {
            QString paramName = ":" + it.key().replace(".", "_");
            whereClauses << QString("%1 = %2").arg(it.key(), paramName);
            bindings[paramName] = it.value();
        }

        // 3. Append WHERE
        if (!whereClauses.isEmpty()) {
            if (sql.contains(" WHERE ", Qt::CaseInsensitive)) sql += " AND " + whereClauses.join(" AND ");
            else sql += " WHERE " + whereClauses.join(" AND ");
        }

        // 4. Append ORDER BY
        if (!criteria.sortColumn.isEmpty()) {
            sql += QString(" ORDER BY %1 %2").arg(criteria.sortColumn, 
                           criteria.sortOrder == Qt::AscendingOrder ? "ASC" : "DESC");
        }

        // 5. Append LIMIT
        if (criteria.limit > 0) {
            sql += " LIMIT :limit OFFSET :offset";
            bindings[":limit"] = criteria.limit;
            bindings[":offset"] = criteria.offset;
        }
    }
};
```

### B. `utils/object_table_model.h`
*The generic model that replaces custom QAbstractListModels.*

```c++
#pragma once
#include <QAbstractListModel>
#include <functional>

template <typename T>
class ObjectListModel : public QAbstractListModel {
public:
    using Extractor = std::function<QVariant(const T&)>;
    
    explicit ObjectListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}

    void addRole(int role, const QByteArray& name, Extractor extractor) {
        m_roleNames[role] = name;
        m_extractors[role] = extractor;
    }

    void setItems(const QList<T>& items) {
        beginResetModel();
        m_items = items;
        endResetModel();
    }

    int rowCount(const QModelIndex&) const override { return m_items.size(); }

    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() >= m_items.size()) return {};
        const T& item = m_items[index.row()];
        if (m_extractors.contains(role)) return m_extractors[role](item);
        return {};
    }

    QHash<int, QByteArray> roleNames() const override { return m_roleNames; }

private:
    QList<T> m_items;
    QHash<int, QByteArray> m_roleNames;
    QHash<int, Extractor> m_extractors;
};
```

### C. `core/data_event_broker.h`
*The signal bus for data changes.*

```c++
#pragma once
#include <QObject>

class DomainEventBroker : public QObject {
    Q_OBJECT
public:
    static DomainEventBroker& instance() {
        static DomainEventBroker _instance;
        return _instance;
    }

    void notifyPersonChanged(int id) { Q_EMIT personChanged(id); }
    void notifyFamilyChanged(int id) { Q_EMIT familyChanged(id); }
    void notifySourceChanged(int id) { Q_EMIT sourceChanged(id); }

Q_SIGNALS:
    void personChanged(int personId);
    void familyChanged(int personId);
    void sourceChanged(int sourceId);

private:
    DomainEventBroker() = default;
};
```
