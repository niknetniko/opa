/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QString>
#include <optional>

/**
 * Manages the media root directory and file import operations.
 *
 * The media root defaults to a 'media/' subdirectory next to the open database
 * file. It can be overridden by the user via the mediaDirectory setting.
 *
 * All paths stored in the database are relative to the media root. Use
 * resolveAbsolutePath() to turn them into absolute paths at display time.
 *
 * Call initialize() when a database is opened and reset() when it is closed.
 * Access the current session via instance().
 */
class MediaService {
public:
    /**
     * Initialize the service for a newly opened database.
     *
     * @param databasePath Absolute path to the .opa database file.
     */
    static void initialize(const QString& databasePath);

    /**
     * Tear down the current session (call when the database is closed).
     */
    static void reset();

    /**
     * Returns true if a database session is currently active.
     */
    [[nodiscard]] static bool isActive();

    /**
     * Returns the active session instance. Aborts if not initialized.
     */
    [[nodiscard]] static MediaService& instance();

    /**
     * Absolute path of the media root directory for this session.
     */
    [[nodiscard]] QString mediaRoot() const;

    /**
     * Resolve a relative (database-stored) path to an absolute path.
     */
    [[nodiscard]] QString resolveAbsolutePath(const QString& relativePath) const;

    /**
     * Copy a file into the media root and return its stored relative path.
     *
     * @param absoluteSourcePath The file to import.
     * @param subfolder          Optional subfolder within the media root.
     * @return Relative path on success, nullopt if the copy failed.
     */
    [[nodiscard]] std::optional<QString> importFile(const QString& absoluteSourcePath, const QString& subfolder = {});

private:
    explicit MediaService(const QString& mediaRoot);

    QString m_mediaRoot;

    static MediaService* s_instance;
};
