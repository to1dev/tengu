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

#include "Error.h"

namespace hyperliquid {

ClientError::ClientError(int statusCode, const QString& errorCode,
    const QString& errorMessage, const json& header, const json& errorData)
    : statusCode_(statusCode)
    , errorCode_(errorCode)
    , errorMessage_(errorMessage)
    , header_(header)
    , errorData_(errorData)
{
}

ServerError::ServerError(int statusCode, const QString& message)
    : statusCode_(statusCode)
    , message_(message)
{
}

}
