/*
 * SPDX-FileCopyrightText: 2022 Jiří Wolker <woljiri@gmail.com>
 * SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "recent_item_model.h"

#include <QDir>
#include <QMimeDatabase>

RecentItemsModel::RecentItemsModel(QObject* parent) : QAbstractListModel(parent) {
}

QVariant RecentItemsModel::data(const QModelIndex& index, int role) const {
    if (index.isValid()) {
        const size_t row = index.row();
        if (row < m_recentItems.size()) {
            const auto& [icon, name, url] = m_recentItems.at(row);
            switch (role) {
                case Qt::DisplayRole:
                    return name;
                case Qt::DecorationRole:
                    return icon;
                case Qt::ToolTipRole:
                    return url.toString(QUrl::PreferLocalFile);
                default:
                    break;
            }
        }
    }

    return {};
}

int RecentItemsModel::rowCount([[maybe_unused]] const QModelIndex& parent) const {
    return static_cast<int>(m_recentItems.size());
}

QString niceFileNameWithPath(const QUrl& url) { // NOLINT(*-use-internal-linkage)
    // we want some filename @ folder output to have chance to keep important stuff even on elide
    if (url.isLocalFile()) {
        // perhaps shorten the path
        const QString homePath = QDir::homePath();
        QString path = url.toString(QUrl::RemoveFilename | QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
        if (path.startsWith(homePath)) {
            path = QLatin1String("~") + path.right(path.length() - homePath.length());
        }
        return url.fileName() + QStringLiteral(" @ ") + path;
    }
    return url.toDisplayString();
}


void RecentItemsModel::refresh(const QList<QUrl>& urls) {
    std::vector<RecentItemInfo> recentItems;
    recentItems.reserve(urls.size());

    QIcon icon;
    for (const QUrl& url: urls) {
        // lookup mime type without accessing file to avoid stall for e.g. NFS/SMB
        const QMimeType mimeType = QMimeDatabase().mimeTypeForFile(url.path(), QMimeDatabase::MatchExtension);
        if (url.isLocalFile() || !mimeType.isDefault()) {
            icon = QIcon::fromTheme(mimeType.iconName());
        } else {
            icon = QIcon::fromTheme(QStringLiteral("network-server"));
        }

        // we want some filename @ folder output to have chance to keep important stuff even on elide, see bug 472981
        const QString name = niceFileNameWithPath(url);

        recentItems.push_back({icon, name, url});
    }

    beginResetModel();
    m_recentItems = std::move(recentItems);
    endResetModel();
}

QUrl RecentItemsModel::url(const QModelIndex& index) const {
    if (index.isValid()) {
        if (const size_t row = index.row(); row < m_recentItems.size()) {
            return m_recentItems.at(row).url;
        }
    }

    return {};
}
