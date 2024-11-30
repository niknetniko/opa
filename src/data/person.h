/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <KExtraColumnsProxyModel>
#include <QString>

namespace Data::Person {
    namespace Table {
        const auto ID = QStringLiteral("id");
        const auto GIVEN_NAMES = QStringLiteral("given_names");
        const auto NICK_NAME = QStringLiteral("nick_name");
        const auto CALL_NAME = QStringLiteral("call_name");
        const auto SUFFIX = QStringLiteral("suffix");
        const auto SEX = QStringLiteral("sex");
    } // namespace Table

    namespace Sex {
        const auto MALE = QStringLiteral("male");
        const auto FEMALE = QStringLiteral("female");
        const auto UNKNOWN = QStringLiteral("unknown");

        QString toDisplay(const QString& sex);

        QString toIcon(const QString& sex);
    } // namespace Sex
} // namespace Data::Person

namespace DisplayNameModel {
    constexpr int ID = 0;
    constexpr int NAME = 1;
    constexpr int ROOT = 2;
} // namespace DisplayNameModel

namespace PersonDetailModel {
    constexpr int ID = 0;
    constexpr int TITLES = 1;
    constexpr int GIVEN_NAMES = 2;
    constexpr int PREFIXES = 3;
    constexpr int SURNAME = 4;
    constexpr int ROOT = 5;
    constexpr int SEX = 6;
    constexpr int DISPLAY_NAME = 7;
}; // namespace PersonDetailModel
