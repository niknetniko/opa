/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QVariant>

/**
 * Converts a QVariant to a specified type.
 *
 * This function uses compile-time checks to determine the desired target type
 * and performs the appropriate conversion of the QVariant.
 *
 * If an unsupported type is specified, a compile-time error is generated.
 */
template<typename T>
T getConverted(const QVariant& v) {
    if constexpr (std::is_same_v<T, QString>) {
        return v.toString();
    } else if constexpr (std::is_same_v<T, int>) {
        return v.toInt();
    } else if constexpr (std::is_same_v<T, long long>) {
        return v.toLongLong();
    } else if constexpr (std::is_same_v<T, double>) {
        return v.toDouble();
    } else {
        static_assert(false, "Unsupported QVariant conversion type");
        std::unreachable();
    }
}

template<typename T>
std::optional<T> validOrOptional(const QVariant& v) {
    if (v.isValid() && !v.isNull()) {
        return getConverted<T>(v);
    } else {
        return std::nullopt;
    }
}
