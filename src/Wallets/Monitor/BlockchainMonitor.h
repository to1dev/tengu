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

#include <concepts>
#include <memory>
#include <optional>

#include <QDebug>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Daitengu::Wallets {

struct TokenInfo {
    QString tokenAddress;
    QString symbol;
    QString name;
    QString balance;
    int decimals;
    QUrl iconUrl;

    TokenInfo() = default;

    TokenInfo(QString address, QString sym, QString nm, QString bal, int dec,
        QUrl icon)
        : tokenAddress(std::move(address))
        , symbol(std::move(sym))
        , name(std::move(nm))
        , balance(std::move(bal))
        , decimals(dec)
        , iconUrl(std::move(icon))
    {
    }
};

class BlockchainMonitor : public QObject {
    Q_OBJECT

public:
    explicit BlockchainMonitor(QObject* parent = nullptr);
    virtual ~BlockchainMonitor();

    virtual void setAddress(const QString& address);

    [[nodiscard]] QString address() const
    {
        return currentAddress_;
    }

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    virtual void refreshBalance() = 0;
    virtual void refreshTokens() = 0;

    [[nodiscard]] virtual bool isConnected() const
    {
        return connected_;
    }

    [[nodiscard]] virtual bool isValidAddress(const QString& address) const = 0;

    void setRefreshInterval(int milliseconds);

    [[nodiscard]] int refreshInterval() const
    {
        return refreshInterval_;
    }

Q_SIGNALS:
    void balanceUpdated(const QString& address, const QString& balance);
    void tokensUpdated(const QString& address, const QList<TokenInfo>& tokens);
    void connectionStatusChanged(bool connected);
    void error(const QString& message);

protected:
    QString currentAddress_;
    bool connected_ { false };
    int refreshInterval_ { 30000 };
    std::unique_ptr<QTimer> refreshTimer_;

    virtual void setupConnection() = 0;

    void startRefreshTimer();

    template <typename T>
    static std::optional<T> getJsonValue(const json& j, const std::string& key)
    {
        if (j.contains(key) && !j[key].is_null()) {
            try {
                return j[key].get<T>();
            } catch (const std::exception& e) {
                qWarning() << "Error extracting JSON value: " << e.what();
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
};

}
