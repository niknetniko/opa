/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QException>

class ResourceNotFoundException : public QException {
public:
    void raise() const override {
        throw *this;
    }
    ResourceNotFoundException* clone() const override {
        return new ResourceNotFoundException(*this);
    }
};
