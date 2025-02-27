#ifndef CONSTS_H
#define CONSTS_H

#include <array>

#include <QMargins>
#include <QObject>
#include <QVector>

namespace Daitengu::Core {

inline constexpr int EXIT_CODE_REBOOT = -6987913;

inline constexpr char COMPANY[] = "to1dev";
inline constexpr char NAME[] = "tengu";
inline constexpr int MAJOR = 0;
inline constexpr int MINOR = 0;
inline constexpr int PATCH = 1;

inline constexpr char EMPTY_STRING[] = "";
inline constexpr char SPACE_CHAR[] = " ";

inline constexpr auto NAME_PATTERN = "<<i|s>v(mon|chu|zard|rtle) the X>";

inline constexpr char SPLASH_PATH[] = ":/Splash/%1";

inline constexpr char STR_ADDRESS_NAME[] = "Address %1";
inline constexpr char STR_DEFAULT_ADDRESS_NAME[] = "Address 1";

inline constexpr QMargins DEFAULT_GROUP_MARGINS = QMargins(20, 24, 20, 20);
inline constexpr int DEFAULT_SPACING = 9;
inline constexpr int DEFAULT_MAXLENGTH = 32;

inline const QString STR_LABEL_NAME = QObject::tr("名称");
inline const QString STR_LABEL_PATH = QObject::tr("路径");
inline const QString STR_LINEEDIT_WALLET_NAME_PLACEHOLDER
    = QObject::tr("输入钱包名称");
inline const QString STR_LINEEDIT_ADDRESS_NAME_PLACEHOLDER
    = QObject::tr("输入地址名称");
inline const QString STR_BUTTON_CLIPBOARD = QObject::tr("复制助记词到剪贴板");

inline const QString CONFIRM_WALLET_DELETE
    = QObject::tr("是否确定删除这个钱包！<p>本操作不可逆！请务必谨慎！</p>");
inline const QString CONFIRM_WALLET_DELETE_TITLE
    = QObject::tr("Confirm Deletion");
inline const QString NO_VALID_WALLET_NAME
    = QObject::tr("请输入有效的钱包名称！");
inline const QString NO_VALID_ADDRESS_NAME
    = QObject::tr("请输入有效的地址名称！");
inline const QString NO_VALID_DERIVATION_PATH
    = QObject::tr("请输入有效的派生路径！");
inline const QString SAME_WALLET_NAME
    = QObject::tr("这个钱包名称已经被占用了！<p>请输入新的钱包名称。</p>");
inline const QString SAME_MNEMONIC = QObject::tr("这个助记词已经存在了！");
inline const QString SAME_PRIV = QObject::tr("这个私钥已经存在了！");
inline const QString SAME_WIF = QObject::tr("这个WIF私钥已经存在了！");
inline const QString SAME_ADDRESS = QObject::tr("这个地址已经存在了！");
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

struct AvatarRange {
    int start;
    int end;
};

inline constexpr std::array<std::pair<std::string_view, AvatarRange>, 5>
    RandomAvatars = { {
        { "Avatar1", { 1, 25 } },
        { "Avatar3", { 1, 50 } },
        { "Avatar5", { 1, 43 } },
        { "Avatar6", { 1, 50 } },
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

}

#endif // CONSTS_H
