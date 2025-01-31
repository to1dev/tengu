QT       += core gui network svg charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets websockets
lessThan(QT_MAJOR_VERSION, 6): QT += winextras

CONFIG += c++2a

CONFIG(release, debug|release): QMAKE_CXXFLAGS += -Wno-unused-parameter \
    -Wno-template-id-cdtor -Wno-tautological-compare -Wno-unused-local-typedefs \
    -Wno-volatile
else: CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Wno-unused-parameter \
    -Wno-template-id-cdtor -Wno-tautological-compare -Wno-unused-local-typedefs \
    -Wno-volatile

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:contains(QT_ARCH, i386) {
} else {
    win32-g++ {
        greaterThan(QT_MAJOR_VERSION, 5) {
            QMAKE_LIBS += -lgraphite2 -lbz2 -lusp10 -lRpcrt4 -lOleAut32
            message("Qt6 or later")
        }
        LIBS += -L$$PWD/3rd/lib -lxxhash -lsodium -lsqlite3
        INCLUDEPATH += $$PWD/3rd/inc $$PWD/3rd/inc/izanagi

        DEFINES += USE_TEST

        LIBS += -lizanagi

        contains(DEFINES, USE_TEST) {
            LIBS += -lcatch2

        }

    }
}

SOURCES += \
    Components/Splash.cpp \
    Components/TitleBar.cpp \
    Databases/InternalDatabase.cpp \
    Main.cpp \
    Managers/GlobalManager.cpp \
    Managers/ResourceManager.cpp \
    Managers/SettingManager.cpp \
    Managers/ThemeManager.cpp \
    Managers/WindowManager.cpp \
    Tengu.cpp \
    Tests/Test_BIP39.cpp \
    Tests/Test_DotEnv.cpp \
    Tests/Test_Encryption.cpp \
    UI/Frameless.cpp \
    Utils/Base58.cpp \
    Utils/Encryption.cpp \
    Utils/RunGuard.cpp \
    Wallets/Mnemonic.cpp \
    Wallets/SolanaWallet.cpp \
    Wallets/Wallet.cpp

HEADERS += \
    Components/Splash.h \
    Components/TitleBar.h \
    Consts.h \
    Databases/InternalDatabase.h \
    Globals.h \
    Managers/GlobalManager.h \
    Managers/ResourceManager.h \
    Managers/SettingManager.h \
    Managers/ThemeManager.h \
    Managers/WindowManager.h \
    Rust.h \
    Security/Security.h \
    Tengu.h \
    UI/Frameless.h \
    Utils/Base58.h \
    Utils/DotEnv.hpp \
    Utils/Encryption.h \
    Utils/RunGuard.h \
    Wallets/Mnemonic.h \
    Wallets/SolanaWallet.h \
    Wallets/Wallet.h

FORMS += \
    Tengu.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/Fonts.qrc \
    res/Global.qrc \
    res/Images.qrc \
    res/Logos.qrc \
    res/Themes/dark/dark.qrc \
    res/Themes/light/light.qrc

RC_FILE = res/Tengu.rc
