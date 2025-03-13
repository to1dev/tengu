// SPDX-License-Identifier: AGPL-3.0-or-later
/*
 * Copyright (C) 2025 to1dev <https://arc20.me/to1dev>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QException>
#include <QString>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace hyperliquid {

class Error : public QException {
public:
    void raise() const override
    {
        throw *this;
    }

    Error* clone() const override
    {
        return new Error(*this);
    }
};

class ClientError : public Error {
public:
    ClientError(int statusCode, const QString& errorCode,
        const QString& errorMessage, const json& header,
        const json& errorData = json());

    void raise() const override
    {
        throw *this;
    }

    ClientError* clone() const override
    {
        return new ClientError(*this);
    }

    int statusCode() const
    {
        return statusCode_;
    }

    QString errorCode() const
    {
        return errorCode_;
    }

    QString errorMessage() const
    {
        return errorMessage_;
    }

    json header() const
    {
        return header_;
    }

    json errorData() const
    {
        return errorData_;
    }

private:
    int statusCode_;
    QString errorCode_;
    QString errorMessage_;
    json header_;
    json errorData_;
};

class ServerError : public Error {
public:
    ServerError(int statusCode, const QString& message);

    void raise() const override
    {
        throw *this;
    }

    ServerError* clone() const override
    {
        return new ServerError(*this);
    }

    int statusCode() const
    {
        return statusCode_;
    }

    QString message() const
    {
        return message_;
    }

private:
    int statusCode_;
    QString message_;
};

}
