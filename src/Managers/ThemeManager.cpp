#include "ThemeManager.h"

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

    // app_->font().setHintingPreference(QFont::PreferDefaultHinting);
}

ThemeManager::~ThemeManager()
{
}

void ThemeManager::setCursor(
    const QVector<QWidget*> widgets, const CursorName name)
{
    for (const auto& obj : widgets) {
        obj->setCursor(cursors_[name]);
    }
}

void ThemeManager::initFonts()
{
    for (std::size_t i = 0; i < Fonts.size(); i++) {
        if (Fonts.at(i).second.name.empty())
            continue;
        QFontDatabase::addApplicationFont(QString(STR_FONT).arg(
            QString::fromUtf8(Fonts.at(i).second.name.data(),
                Fonts.at(i).second.name.size())));
    }
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

    QString qss = QString(STR_THEME_QSS).arg(theme_.name);
    QFile file(qss);

    if (file.exists()) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream ts(&file);
            int i = DEFAULT_FONT_ID;
            QString extraStyles
                = QString(STR_GLOBAL_STYLE)
                      .arg(Fonts[i].first)
                      .arg(qRound(scale_ * DEFAULT_FONT_SIZE))
                      .arg(Fonts[i].second.italic ? "font-style:italic;"
                                                  : EMPTY_STRING);
            styles = extraStyles + ts.readAll();
        }
    }

    QString qss2 = QString(STR_THEME_QSS2).arg(theme_.name);
    QFile file2(qss2);
    if (file2.exists()) {
        if (file2.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream ts(&file2);
            styles += ts.readAll();
        }
    }

    app_->setStyleSheet(styles);
}

}
