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

#include "ThemeManager.h"

#include <filesystem>

namespace fs = std::filesystem;

#include "Utils/PathUtils.hpp"

using namespace Daitengu::Utils;

namespace Daitengu::Core {

ThemeManager::ThemeManager()
    : app_(qApp)
    , scale_(1.0)
{
#ifdef no_app
    if (nullptr == app_) {
        app_ = qobject_cast<QApplication*>(QApplication::instance());
    }
#endif

    if (FIXED_DPI == QGuiApplication::primaryScreen()->logicalDotsPerInch())
        scale_ = SCALED_125;
    else
        scale_ = SCALED_150;

    initFonts();
    initThemes();

    /*QFont font = QApplication::font();
    font.setHintingPreference(QFont::PreferFullHinting);
    QApplication::setFont(font);*/
}

void ThemeManager::initFonts()
{
    for (const auto& font : Fonts) {
        if (font.second.name.empty())
            continue;

        if (fontDb_.addApplicationFont(QString(STR_FONT).arg(QString::fromUtf8(
                font.second.name.data(), font.second.name.size())))
            == -1) {
            qWarning() << "Failed to load font:" << font.first.data();
        }
    }
}

std::optional<QString> ThemeManager::loadCustomFont()
{
    auto path = PathUtils::getAppDataPath(COMPANY) / NAME / "main.ttf";

    if (!fs::exists(path)) {
        return std::nullopt;
    }

    int fid = fontDb_.addApplicationFont(path.string().c_str());
    if (fid == -1) {
        qWarning() << "Failed to load custom font from";
        return std::nullopt;
    }

    auto list = fontDb_.applicationFontFamilies(fid);
    if (list.isEmpty()) {
        qWarning() << "No font families loaded from";
        return std::nullopt;
    }

    return list.first();
}

void ThemeManager::initThemes()
{
    theme_.name = QString(DEFAULT_THEME_NAME).toLower();
    theme_.path = EMPTY_STRING;

    parseTheme();
    initStyle();
}

void ThemeManager::parseTheme()
{
    QFile file(QString(STR_THEME_STYlE).arg(theme_.name));
    if (file.exists()) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            try {
                json jObj = json::parse(file.readAll().toStdString());

                if (jObj.contains(STR_JSON_STYLE)) {
                    theme_.style = QString::fromStdString(jObj[STR_JSON_STYLE]);
                }

                app_->setStyle(
                    theme_.style.isEmpty() ? STR_DEFAULT_STYLE : theme_.style);

#ifdef custom_style
                qApp->setStyle(new MyProxyStyle(qApp->style()));
#endif

                if (jObj.contains(STR_JSON_CURSORS)
                    && jObj[STR_JSON_CURSORS].is_array()) {
                    initCursors(jObj[STR_JSON_CURSORS]);
                }

                if (jObj.contains(STR_JSON_PALETTE)
                    && jObj[STR_JSON_PALETTE].is_object()) {
                    initPalette(jObj[STR_JSON_PALETTE]);
                }

            } catch (nlohmann::json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }

            file.close();
        }
    }
}

void ThemeManager::initCursors(const json& cursors)
{
}

void ThemeManager::initPalette(const json& palette)
{
    QPalette pal;

    if (palette.is_object()) {

        for (const auto& value : ColorRoles1) {
            if (palette.contains(value.first)
                && palette[value.first].is_object()) {
                json jColor = palette[value.first];
                if (jColor.contains(STR_JSON_PALETTE_COLOR)) {
                    QColor color(
                        QString::fromStdString(jColor[STR_JSON_PALETTE_COLOR]));
                    pal.setColor(value.second, color);
                }

                if (jColor.contains(STR_JSON_PALETTE_DISABLED)) {
                    QColor disabled(QString::fromStdString(
                        jColor[STR_JSON_PALETTE_DISABLED]));
                    pal.setColor(QPalette::Disabled, value.second, disabled);
                }
            }
        }

        app_->setPalette(pal);
    }
}

void ThemeManager::initStyle()
{
    QString styles;

    QFile file(QString(STR_THEME_QSS).arg(theme_.name));
    if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&file);
        QString extraStyles;

        auto customFont = loadCustomFont();
        if (customFont) {
            extraStyles = QString(STR_GLOBAL_STYLE)
                              .arg(*customFont)
                              .arg(qRound(scale_ * DEFAULT_FONT_SIZE))
                              .arg("");
        } else {
            int i = DEFAULT_FONT_ID;
            extraStyles = QString(STR_GLOBAL_STYLE)
                              .arg(Fonts[i].first.data())
                              .arg(qRound(scale_ * DEFAULT_FONT_SIZE))
                              .arg(Fonts[i].second.italic ? "font-style:italic;"
                                                          : EMPTY_STRING);
        }
        styles = extraStyles + ts.readAll();
    }

    QFile file2(QString(STR_THEME_QSS2).arg(theme_.name));
    if (file2.exists() && file2.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream ts(&file2);
        styles += ts.readAll();
    }

    app_->setStyleSheet(styles);
}

}
