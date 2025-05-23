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

#ifndef CONSTS_H
#define CONSTS_H

#include <array>

#include <QMargins>
#include <QObject>
#include <QVector>

#include "Wallets/Core/Types.h"

using namespace Daitengu::Wallets;

namespace Daitengu::Core {

inline constexpr int EXIT_CODE_REBOOT = -6987913;

inline constexpr char COMPANY[] = "to1dev";
inline constexpr char NAME[] = "tengu";
inline constexpr int MAJOR = 0;
inline constexpr int MINOR = 0;
inline constexpr int PATCH = 1;
inline constexpr char PRERELEASE[] = "alpha";
inline constexpr char BUILD_METADATA[] = "20250301";

inline constexpr char EMPTY_STRING[] = "";
inline constexpr char SPACE_CHAR[] = " ";

inline constexpr auto NAME_PATTERN = "w the W";

inline constexpr char SPLASH_PATH[] = ":/Splash/%1";

inline constexpr char STR_ADDRESS_NAME[] = "Address %1";
inline constexpr char STR_DEFAULT_ADDRESS_NAME[] = "Address 1";

inline constexpr QMargins DEFAULT_GROUP_MARGINS = QMargins(20, 24, 20, 20);
inline constexpr int DEFAULT_SPACING = 9;
inline constexpr int DEFAULT_MAXLENGTH = 42;
inline constexpr int DEFAULT_ADDRESS_MAXLENGTH = 128;

inline const QString STR_LABEL_NAME = QObject::tr("名称");
inline const QString STR_LABEL_PATH = QObject::tr("路径");
inline const QString STR_LABEL_ADDRESS = QObject::tr("地址");
inline const QString STR_LABEL_CHAIN = QObject::tr("公链");
inline const QString STR_LABEL_DESC = QObject::tr("备注");
inline const QString STR_LABEL_NETWORK = QObject::tr("网络");
inline const QString STR_LINEEDIT_WALLET_NAME_PLACEHOLDER
    = QObject::tr("输入钱包名称");
inline const QString STR_LINEEDIT_ADDRESS_NAME_PLACEHOLDER
    = QObject::tr("输入地址名称");
inline const QString STR_LINEEDIT_ADDRESS_PLACEHOLDER = QObject::tr("输入地址");
inline const QString STR_BUTTON_CLIPBOARD = QObject::tr("复制助记词到剪贴板");
inline const QString CONFIRM_WALLET_DELETE_TITLE
    = QObject::tr("Confirm Deletion");
inline const QString NO_VALID_WALLET_NAME
    = QObject::tr("请输入有效的钱包名称！");
inline const QString NO_VALID_ADDRESS_NAME
    = QObject::tr("请输入有效的地址名称！");
inline const QString NO_VALID_ADDRESS = QObject::tr("请输入有效的地址！");
inline const QString NO_VALID_DERIVATION_PATH
    = QObject::tr("请输入有效的派生路径！");
inline const QString SAME_WALLET_NAME
    = QObject::tr("这个钱包名称已经被占用了！<p>请输入新的钱包名称。</p>");
inline const QString SAME_ADDRESS_NAME
    = QObject::tr("当前钱包已经有这个地址名称了！<p>请输入新的地址名称。</p>");
inline const QString SAME_MNEMONIC
    = QObject::tr("这个助记词已经存在了！<p>请输入新的助记词。</p>");
inline const QString SAME_PRIV
    = QObject::tr("这个私钥已经存在了！<p>请输入新的私钥。</p>");
inline const QString SAME_WIF = QObject::tr("这个WIF私钥已经存在了！");
inline const QString SAME_ADDRESS
    = QObject::tr("这个地址已经存在了！<p>请输入新的地址。</p>");
inline const QString INVALID_MNEMONIC = QObject::tr("这是一个无效助记词！");
inline const QString INVALID_PRIV = QObject::tr("这是一个无效私钥！");
inline const QString INVALID_ADDRESS = QObject::tr("这是一个无效地址！");

inline const QString STR_MENU_APP = QObject::tr("快捷方式");
inline const QString STR_WINDOW_CENTER = QObject::tr("窗体复位");
inline const QString STR_APP_REBOOT = QObject::tr("重启程序");
inline const QString STR_APP_SOCKET = QObject::tr("网络测试");
inline const QString STR_APP_RESET_DB = QObject::tr("重置数据");
inline const QString STR_MENU_ABOUT = QObject::tr("关于程序");
inline const QString STR_MENU_EXIT = QObject::tr("退出程序");

inline const QString DEFAULT_ADDRESS_NAME = QObject::tr("无效地址");
inline constexpr char DEFAULT_ADDRESS[]
    = "0x0000000000000000000000000000000000000000000000000000000000";

struct AvatarRange {
    int start;
    int end;
};

inline constexpr std::array<std::pair<std::string_view, AvatarRange>, 4>
    RandomAvatars = { {
        { "Avatar1", { 1, 25 } },
        { "Avatar3", { 1, 50 } },
        { "Avatar5", { 1, 43 } },
        { "Avatar9", { 1, 40 } },
    } };

inline constexpr int LOGO_RECT_WIDTH = 64;
inline constexpr int LOGO_RECT_HEIGHT = 64;
inline constexpr int LOGO_SIZE = 64;
inline constexpr int AVATAR_SIZE = 64;

inline constexpr std::array<std::pair<std::string_view, AvatarRange>, 1>
    RandomLogos = { {
        { "Logo1", { 1, 50 } },
    } };

struct ChainItem {
    std::string_view name;
    bool enabled;
};

inline constexpr std::array<std::pair<ChainType, ChainItem>, 4> Chains = { {
    {
        ChainType::BITCOIN,
        { "Bitcoin", true },
    },
    {
        ChainType::ETHEREUM,
        { "Ethereum", true },
    },
    {
        ChainType::SOLANA,
        { "Solana", true },
    },
    {
        ChainType::SUI,
        { "Sui", false },
    },
} };
}

#endif // CONSTS_H
