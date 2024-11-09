#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <QString>

/**
 * Open the database, or error.
 */
void open_database(const QString& file, bool seed = true);
