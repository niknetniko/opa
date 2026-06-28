/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "media_service.h"

#include "opaSettings.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

using namespace Qt::StringLiterals;

MediaService* MediaService::s_instance = nullptr;

MediaService::MediaService(QString mediaRoot) : m_mediaRoot(std::move(mediaRoot)) {
}

void MediaService::initialize(const QString& databasePath) {
    delete s_instance;

    QString root = opaSettings::mediaDirectory();
    if (root.isEmpty()) {
        root = QFileInfo(databasePath).dir().filePath(u"media"_s);
    }

    s_instance = new MediaService(root);
    qDebug() << "MediaService initialized with root:" << root;
}

void MediaService::reset() {
    delete s_instance;
    s_instance = nullptr;
}

bool MediaService::isActive() {
    return s_instance != nullptr;
}

MediaService& MediaService::instance() {
    Q_ASSERT_X(s_instance != nullptr, "MediaService::instance", "MediaService not initialized");
    return *s_instance;
}

QString MediaService::mediaRoot() const {
    return m_mediaRoot;
}

QString MediaService::resolveAbsolutePath(const QString& relativePath) const {
    return QDir(m_mediaRoot).filePath(relativePath);
}

std::optional<QString> MediaService::importFile(const QString& absoluteSourcePath, const QString& subfolder) {
    QFileInfo sourceInfo(absoluteSourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        qWarning() << "MediaService::importFile: source does not exist:" << absoluteSourcePath;
        return std::nullopt;
    }

    QDir targetDir(m_mediaRoot);
    if (!subfolder.isEmpty()) {
        targetDir = QDir(targetDir.filePath(subfolder));
    }

    if (!targetDir.mkpath(u"."_s)) {
        qWarning() << "MediaService::importFile: could not create directory:" << targetDir.absolutePath();
        return std::nullopt;
    }

    QString targetName = sourceInfo.fileName();
    QString targetAbsolute = targetDir.filePath(targetName);

    // If a file with the same name already exists, append a counter.
    if (QFile::exists(targetAbsolute)) {
        const QString baseName = sourceInfo.completeBaseName();
        const QString suffix = sourceInfo.suffix();
        int counter = 1;
        while (QFile::exists(targetAbsolute)) {
            targetName = u"%1_%2.%3"_s.arg(baseName).arg(counter++).arg(suffix);
            targetAbsolute = targetDir.filePath(targetName);
        }
    }

    if (!QFile::copy(absoluteSourcePath, targetAbsolute)) {
        qWarning() << "MediaService::importFile: copy failed from" << absoluteSourcePath << "to" << targetAbsolute;
        return std::nullopt;
    }

    // Return path relative to the media root.
    return QDir(m_mediaRoot).relativeFilePath(targetAbsolute);
}
