/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(OPA_SQL);

/**
 * Open the database, or error.
 */
void open_database(const QString& file, bool seed = true);

void close_database();
