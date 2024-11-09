#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <QString>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(OPA_SQL);

/**
 * Open the database, or error.
 */
void open_database(const QString& file, bool seed = true);
