#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <iostream>

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QPalette>
#include <QScreen>
#include <QTextStream>
#include <QWidget>

#include "Consts.h"

#include "ankerl/unordered_dense.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Daitengu::Core {

inline constexpr int FIXED_DPI = 96;
inline constexpr double SCALED_125 = 1.25;
inline constexpr double SCALED_150 = 1.50;

inline constexpr char STR_HIGH_DPI[] = "@2x";
inline constexpr char STR_LOW_DPI[] = "";

inline constexpr int DEFAULT_FONT_ID = 0;
inline constexpr bool DEFAULT_FONT_ITALIC = true;
inline constexpr int DEFAULT_FONT_SIZE = 12;
inline constexpr bool DEFAULT_FONT_SYSTEM = false;
inline constexpr char DEFAULT_LANG_NAME[] = "zh-CN";

inline constexpr char STR_THEME_ROOT[] = "%1/themes";
inline constexpr char STR_THEME_PATH[] = "%1/%2.theme";
inline constexpr char STR_THEME_EXT[] = "*.theme";
inline constexpr char STR_DEFAULT_THEME_PATH[] = "runtime/default.bin";
inline constexpr char STR_DEFAULT_STYLE[] = "Fusion";
inline constexpr char STR_FONT[] = ":/fonts/%1";
inline constexpr char STR_GLOBAL_STYLE[]
    = "* {font-family: '%1';font-size: %2px;%3}";
inline constexpr char STR_CURSOR[] = ":/theme/%1/cursors/%2";
inline constexpr char STR_THEME_STYlE[] = ":/theme/%1/style.json";
inline constexpr char STR_THEME_QSS[] = ":/theme/%1/main.qss";
inline constexpr char STR_THEME_QSS2[] = ":/theme/%1/custom.qss";

inline constexpr char STR_JSON_THEME_NAME[] = "Name";
inline constexpr char STR_JSON_STYLE[] = "Style";
inline constexpr char STR_JSON_CURSORS[] = "Cursors";
inline constexpr char STR_JSON_CURSOR_ALIAS[] = "alias";
inline constexpr char STR_JSON_CURSOR_INDEX[] = "index";
inline constexpr char STR_JSON_CURSOR_CALC_HOTSPOT[] = "calcSpot";
inline constexpr char STR_JSON_CURSOR_SCALE[] = "scale";
inline constexpr char STR_JSON_CURSOR_HOTX[] = "hotX";
inline constexpr char STR_JSON_CURSOR_HOTY[] = "hotY";
inline constexpr char STR_JSON_PALETTE[] = "Palette";
inline constexpr char STR_JSON_PALETTE_COLOR[] = "color";
inline constexpr char STR_JSON_PALETTE_DISABLED[] = "disabled";

inline constexpr char DEFAULT_THEME_NAME[] = "Dark";

enum class CursorName {
    CUR_DEFAULT = 0,
    CUR_GOLD,
    CUR_WHITE,
    CUR_TEXT,
    CUR_POINTER,
    CUR_HAND,
    CUR_DRAG,
};

struct FontData {
    bool system;
    bool italic;
    QString name;
};

inline const ankerl::unordered_dense::map<std::string, QPalette::ColorRole>
    ColorRoles1 = {
        { "windowText", QPalette::WindowText },
        { "button", QPalette::Button },
        { "light", QPalette::Light },
        { "midLight", QPalette::Midlight },
        { "dark", QPalette::Dark },
        { "mid", QPalette::Mid },
        { "text", QPalette::Text },
        { "brightText", QPalette::BrightText },
        { "buttonText", QPalette::ButtonText },
        { "base", QPalette::Base },
        { "window", QPalette::Window },
        { "shadow", QPalette::Shadow },
        { "highlight", QPalette::Highlight },
        { "highlightedText", QPalette::HighlightedText },
        { "link", QPalette::Link },
        { "linkVisited", QPalette::LinkVisited },
        { "alternateBase", QPalette::AlternateBase },
        { "toolTipBase", QPalette::ToolTipBase },
        { "toolTipText", QPalette::ToolTipText },
        { "placeholderText", QPalette::PlaceholderText },
    };

static QVector<QPair<QString, FontData>> Fonts = {
    { "TsangerLiyuan", { false, false, "TsangerLiyuan" } },
    { "Lobster Two", { false, false, "LobsterTwo" } },
    { "Pirata One", { false, false, "PirataOne" } },
    { "Jockey One", { false, false, "JockeyOne" } },
    { "Sigmar", { false, false, "Sigmar" } },
};

typedef ankerl::unordered_dense::map<CursorName, QCursor> CursorList;

class ThemeManager {

    struct Theme {
        QString name;
        QString style;
        QString path;
    };

public:
    ThemeManager(QApplication* app = nullptr);
    ~ThemeManager();

    void setCursor(const QVector<QWidget*> widgets, const CursorName name);

private:
    void initFonts();
    void initThemes();
    void parseTheme();
    void initCursors(const json& cursors);
    void initPalette(const json& palette);
    void initStyle();

private:
    QApplication* app_;
    CursorList cursors_;
    QStringList themes_;
    Theme theme_;
    double scale_;
};

}
#endif // THEMEMANAGER_H
