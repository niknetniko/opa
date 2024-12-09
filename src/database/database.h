/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QLoggingCategory>
// ReSharper disable once CppUnusedIncludeDirective
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(OPA_SQL);

/**
 * Open the database, or error.
 */
void openDatabase(const QString& file, bool seed = true, bool initialise = true);

void closeDatabase();
