#ifndef CONSTS_H
#define CONSTS_H

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

inline constexpr char SPLASH_PATH[] = ":/Splash/%1";

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

inline QVector<QPair<QString, AvatarRange>> RandomAvatars = {
    { "Avatar1", { 1, 25 } },
    { "Avatar2", { 1, 29 } },
    { "Avatar3", { 1, 50 } },
    { "Avatar4", { 1, 60 } },
    { "Avatar5", { 1, 43 } },
    { "Avatar6", { 1, 50 } },
    { "Avatar7", { 1, 50 } },
    { "Avatar8", { 1, 50 } },
    { "Avatar9", { 1, 40 } },
};

inline constexpr int LOGO_RECT_WIDTH = 64;
inline constexpr int LOGO_RECT_HEIGHT = 64;
inline constexpr int LOGO_SIZE = 64;
inline constexpr int AVATAR_SIZE = 64;

inline QVector<QPair<QString, AvatarRange>> RandomLogos = {
    //{ "Logo1", { 1, 20 } },
    //{ "Logo2", { 1, 30 } },
    { "Logo1", { 1, 50 } },
};

}

#endif // CONSTS_H
