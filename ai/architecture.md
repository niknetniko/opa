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

# Architecture Guide: Data Editing (CQRS Pattern)

This document outlines how data modification (Create, Update, Delete) is handled in the new Repository architecture.

We strictly follow the **CQRS (Command Query Responsibility Segregation)** pattern. Our models (`ObjectTableModel`, `ObjectListModel`) are **Read-Only** by default. We do not use `QSqlTableModel`'s automatic `OnRowChange` editing. All database writes must flow through a Repository class.

## 1. Core Principles for AI Agents
When tasked with adding editing functionality to a view:
1. **Never write SQL directly in the UI or View Controller.**
2. **Never modify the database directly from a Model** (unless using a configured inline-setter).
3. **Always use Entities (Structs)** to pass data between the UI and the Repository.
4. **Always emit a change notification** from the Repository upon successful save using `DataEventBroker`.

---

## 2. Approach A: Form-Based Editing (Recommended)

Use this approach when the user clicks an "Edit" button and modifies data inside a form or dialog containing `QLineEdit`, `QComboBox`, etc.

### Step 1: The UI logic (Controller)
Gather the updated values from the UI, mutate the C++ Entity struct, and pass it to the Repository.

```c++
void PersonProfileWidget::saveChanges() {
    // 1. Copy the current state into an Entity struct
    PersonDisplayEntity updatedPerson = m_currentPerson;
    
    // 2. Apply UI changes to the struct
    updatedPerson.givenNames = ui->givenNamesLineEdit->text();
    updatedPerson.surname = ui->surnameLineEdit->text();
    
    // 3. Delegate writing to the Repository
    PersonRepository repo;
    if (repo.updatePerson(updatedPerson)) {
        // Handle UI success (e.g., close dialog)
    } else {
        // Handle UI error
    }
}
```

### Step 2: The Repository logic
The Repository maps the struct to an `UPDATE` or `INSERT` query. On success, it **must** notify the application.

```c++
bool PersonRepository::updatePerson(const PersonDisplayEntity& person) {
    QSqlQuery query;
    query.prepare("UPDATE names SET given_names = :g, surname = :s WHERE person_id = :id");
    query.bindValue(":g", person.givenNames);
    query.bindValue(":s", person.surname);
    query.bindValue(":id", person.id);
    
    if (!query.exec()) return false;
    
    // CRITICAL: Notify the event broker using std::optional
    DataEventBroker::instance().notifyChanged<Schema::Names>(person.id);
    
    return true;
}
```
*(Note: Because our generic read models use `connectToTable`, emitting this signal will automatically cause all visible tables and lists displaying this person to refresh).*

---

## 3. Approach B: Inline Table Editing

Use this approach *only* when the user needs to double-click a cell in a `QTableView` to type a new value directly.

### Step 1: Extend `ObjectTableModel`
If not already implemented, ensure `ObjectTableModel` supports `Setter` lambdas.

```c++
// Required modifications to ObjectTableModel<T>:
using Setter = std::function<bool(T&, const QVariant&)>;

struct ColumnDef {
    QString header;
    Extractor extractor;
    Setter setter; // Allow null for read-only columns
};

// Override flags to enable Qt::ItemIsEditable
Qt::ItemFlags flags(const QModelIndex& index) const override {
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    if (index.isValid() && columns[index.column()].setter != nullptr) {
        return defaultFlags | Qt::ItemIsEditable;
    }
    return defaultFlags;
}

// Override setData to trigger the setter and save
bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override {
    if (index.isValid() && role == Qt::EditRole) {
        auto setter = columns[index.column()].setter;
        if (setter) {
            T& item = items[index.row()];
            if (setter(item, value)) {
                Q_EMIT dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
                return true;
            }
        }
    }
    return false;
}
```

### Step 2: Define Setters in the View Model
When setting up columns in a specific model (e.g., `PersonNamesModel`), pass a lambda that updates the struct and calls the Repository.

```c++
// Inside PersonNamesModel constructor:
this->setColumn(GIVEN_NAMES, i18n("Given names"), &NameWithOriginEntity::givenNames, 
    [](NameWithOriginEntity& rowData, const QVariant& newValue) -> bool {
        
        // 1. Validate data if necessary
        QString newName = newValue.toString();
        if (newName.isEmpty()) return false;
        
        // 2. Update the local memory struct
        rowData.givenNames = newName;
        
        // 3. Persist to DB via Repository
        NameRepository repo;
        return repo.updateName(rowData); 
        // Note: repo.updateName() will trigger DataEventBroker
    }
);
```

---

## 4. Agent Checklist for Implementing an Edit Feature
When instructed to "make entity X editable":

1. [ ] Check if the User requested Form-Based or Inline editing.
2. [ ] **Entity**: Ensure the Entity struct holds all necessary IDs to perform an SQL `UPDATE`.
3. [ ] **Repository**: Add an `updateEntity(const Entity& e)` method.
4. [ ] **Repository SQL**: Write the `UPDATE` query using parameterized bindings.
5. [ ] **Event Broker**: Ensure the Repository calls `DataEventBroker::instance().notifyChanged<Schema::Table>(entity.id)` upon successful `exec()`.
6. [ ] **UI/Model Hookup**: Link the UI 'Save' action (Approach A) or the Model's `Setter` (Approach B) to the new Repository method.
