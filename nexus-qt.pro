TEMPLATE = app
TARGET = nexus-qt
VERSION = 0.1.0.0
INCLUDEPATH += src src/core src/hash src/json src/keys src/net src/qt src/util src/wallet src/LLD src/LLP
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
CONFIG += no_include_pwd
CONFIG += warn_off

#Manually Link to Windoze Library Locations
windows:BOOST_LIB_SUFFIX=-mgw49-mt-s-1_55
windows:BOOST_INCLUDE_PATH=C:/Deps/boost_1_55_0
windows:BOOST_LIB_PATH=C:/Deps/boost_1_55_0/stage/lib
windows:BDB_INCLUDE_PATH=C:/Deps/db-4.8.30.NC/build_unix
windows:BDB_LIB_PATH=C:/Deps/db-4.8.30.NC/build_unix
windows:OPENSSL_INCLUDE_PATH=C:/Deps/openssl-1.0.1p/include
windows:OPENSSL_LIB_PATH=C:/Deps/openssl-1.0.1p
windows:MINIUPNPC_INCLUDE_PATH=C:/Deps/
windows:MINIUPNPC_LIB_PATH=C:/Deps/miniupnpc
windows:QRENCODE_INCLUDE_PATH=C:/Deps/qrencode-3.4.3
windows:QRENCODE_LIB_PATH=C:/Deps/qrencode-3.4.3/.libs


# for boost 1.37, add -mt to the boost libraries 
# use: qmake BOOST_LIB_SUFFIX=-mt
# for boost thread win32 with _win32 sufix
# use: BOOST_THREAD_LIB_SUFFIX=_win32-...
# or when linking against a specific BerkelyDB version: BDB_LIB_SUFFIX=-4.8

# Dependency library locations can be customized with BOOST_INCLUDE_PATH, 
#    BOOST_LIB_PATH, BDB_INCLUDE_PATH, BDB_LIB_PATH
#    OPENSSL_INCLUDE_PATH and OPENSSL_LIB_PATH respectively

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    # Mac: compile for Yosemite and Above (10.10, 32-bit)
    macx:QMAKE_CXXFLAGS += -O3 -mmacosx-version-min=10.10 -arch x86_64

    #TODO compile qt with +framework and +universal option (-arch i386)

    #-sysroot /Developer/SDKs/MacOSX10.10.sdk

    #Static Configuration
    windows:CONFIG += STATIC
	
	windows:QMAKE_LFLAGS += -Wl,--dynamicbase -Wl,--nxcompat
	windows:QMAKE_LFLAGS += -Wl,--large-address-aware -static
	windows:QMAKE_LFLAGS += -static-libgcc -static-libstdc++

    !windows:!macx {
        # Linux: static link
        LIBS += -Wl,-Bstatic
    }
}

# use: qmake "USE_QRCODE=1"
# libqrencode (http://fukuchi.org/works/qrencode/index.en.html) must be installed for support
contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    LIBS += -lqrencode
}

# use: qmake "USE_UPNP=1" ( enabled by default; default)
#  or: qmake "USE_UPNP=0" (disabled by default)
#  or: qmake "USE_UPNP=-" (not supported)
# miniupnpc (http://miniupnp.free.fr/files/) must be installed for support
contains(USE_UPNP, -) {
    message(Building without UPNP support)
} else {
    message(Building with UPNP support)
    count(USE_UPNP, 0) {
        USE_UPNP=1
    }
    DEFINES += USE_UPNP=$$USE_UPNP STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
}

#handle the LLD build option. Default on LLD branch is to use LLD
contains(USE_LLD, 0) {
	message(Building without Lower Level Database Support)
}
else {
	message(Building with Lower Level Database Support)
	
	DEFINES += USE_LLD
}

# use: qmake "USE_DBUS=1"
contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "FIRST_CLASS_MESSAGING=1"
contains(FIRST_CLASS_MESSAGING, 1) {
    message(Building with first-class messaging)
    DEFINES += FIRST_CLASS_MESSAGING
}

contains(BITCOIN_NEED_QT_PLUGINS, 1) {
    DEFINES += BITCOIN_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

!windows {
    # for extra security against potential buffer overflows
    QMAKE_CXXFLAGS += -fstack-protector
    QMAKE_LFLAGS += -fstack-protector
    # do not enable this on windows, as it will result in a non-working executable!
}

QMAKE_CXXFLAGS += -D_FORTIFY_SOURCE=2 -fpermissive

QMAKE_CXXFLAGS_WARN_ON = -Wall -Wextra -Wformat -Wformat-security -Wno-invalid-offsetof -Wno-sign-compare -Wno-unused-parameter
!macx {
    QMAKE_CXXFLAGS_WARN_ON += -fdiagnostics-show-option

    #To Compile the Keccak C Files (For Linux and Windoze)
    MAKE_EXT_CPP  += .c
}

# Input
DEPENDPATH += src src/LLP sr/LLD src/core src/hash src/json src/net src/util src/qt src/wallet src/qt/core src/qt/dialogs src/qt/forms src/qt/models src/qt/pages src/qt/res src/qt/util src/qt/wallet
HEADERS += src/qt/core/gui.h \
    src/qt/models/transactiontablemodel.h \
    src/qt/models/addresstablemodel.h \
    src/qt/dialogs/optionsdialog.h \
    src/qt/dialogs/sendcoinsdialog.h \
    src/qt/pages/addressbookpage.h \
    src/qt/pages/messagepage.h \
    src/qt/dialogs/aboutdialog.h \
    src/qt/dialogs/editaddressdialog.h \
    src/qt/util/addressvalidator.h \
    src/net/addrman.h \
    src/wallet/base58.h \
    src/util/bignum.h \
    src/util/compat.h \
    src/util/util.h \
	src/hash/uint1024.h \
	src/hash/templates.h \
    src/util/serialize.h \
    src/util/strlcpy.h \
    src/core/core.h \
	src/core/unifiedtime.h \
    src/net/net.h \
    src/wallet/key.h \
    src/wallet/db.h \
    src/wallet/walletdb.h \
    src/wallet/script.h \
    src/main.h \
	src/LLP/coreserver.h \
	src/LLP/server.h \
	src/LLP/types.h \
	src/LLD/index.h \
	src/LLD/keychain.h \
	src/LLD/key.h \
	src/LLD/sector.h \
    src/util/mruset.h \
    src/hash/brg_endian.h \
    src/hash/brg_types.h \
    src/hash/crypto_hash.h \
    src/hash/KeccakDuplex.h \
    src/hash/KeccakF-1600-interface.h \
    src/hash/KeccakHash.h \
    src/hash/KeccakSponge.h \
    src/hash/skein.h \
    src/hash/skein_iv.h \
    src/hash/skein_port.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/models/clientmodel.h \
    src/qt/util/guiutil.h \
    src/qt/wallet/transactionrecord.h \
    src/qt/core/guiconstants.h \
    src/qt/models/optionsmodel.h \
    src/qt/util/monitoreddatamapper.h \
    src/qt/wallet/transactiondesc.h \
    src/qt/dialogs/transactiondescdialog.h \
    src/qt/util/amountfield.h \
    src/wallet/wallet.h \
    src/wallet/keystore.h \
    src/qt/wallet/transactionfilterproxy.h \
    src/qt/wallet/transactionview.h \
    src/qt/models/walletmodel.h \
    src/net/rpcserver.h \
    src/qt/pages/overviewpage.h \
    src/qt/util/csvmodelwriter.h \
    src/wallet/crypter.h \
    src/qt/wallet/sendcoinsentry.h \
    src/qt/util/qvalidatedlineedit.h \
    src/qt/core/units.h \
    src/qt/util/qvaluecombobox.h \
    src/qt/dialogs/askpassphrasedialog.h \
    src/net/protocol.h \
	src/core/version.h \
    src/qt/util/notificator.h \
    #src/qt/core/qtipcserver.h \
    src/util/allocators.h \
    src/util/ui_interface.h \
    src/qt/core/rpcconsole.h

SOURCES += src/core/block.cpp \
	src/core/dispatch.cpp \
	src/core/message.cpp \
	src/core/transaction.cpp \
	src/core/mining.cpp \
	src/core/checkpoints.cpp \
    src/qt/main-qt.cpp \
	src/qt/core/gui.cpp \
    src/qt/models/transactiontablemodel.cpp \
    src/qt/models/addresstablemodel.cpp \
    src/qt/dialogs/optionsdialog.cpp \
    src/qt/dialogs/sendcoinsdialog.cpp \
    src/qt/pages/addressbookpage.cpp \
    src/qt/pages/messagepage.cpp \
    src/qt/dialogs/aboutdialog.cpp \
    src/qt/dialogs/editaddressdialog.cpp \
    src/qt/util/addressvalidator.cpp \
    src/core/version.cpp \
	src/core/difficulty.cpp \
	src/core/prime.cpp \
	src/core/debug.cpp \
    src/util/util.cpp \
    src/net/netbase.cpp \
    src/wallet/key.cpp \
    src/wallet/script.cpp \
    src/hash/skein.cpp \
    src/hash/skein_block.cpp \
    src/hash/KeccakDuplex.c \
    src/hash/KeccakSponge.c \
    src/hash/Keccak-compact64.c \
    src/hash/KeccakHash.c \
    src/net/net.cpp \
    src/net/addrman.cpp \
	src/core/release.cpp \
	src/core/unifiedtime.cpp \
    src/wallet/db.cpp \
    src/wallet/walletdb.cpp \
    src/json/json_spirit_writer.cpp \
    src/json/json_spirit_value.cpp \
    src/json/json_spirit_reader.cpp \
    src/qt/models/clientmodel.cpp \
    src/qt/util/guiutil.cpp \
    src/qt/wallet/transactionrecord.cpp \
    src/qt/models/optionsmodel.cpp \
    src/qt/util/monitoreddatamapper.cpp \
    src/qt/wallet/transactiondesc.cpp \
    src/qt/dialogs/transactiondescdialog.cpp \
    src/qt/core/strings.cpp \
    src/qt/util/amountfield.cpp \
    src/wallet/wallet.cpp \
    src/wallet/keystore.cpp \
    src/qt/wallet/transactionfilterproxy.cpp \
    src/qt/wallet/transactionview.cpp \
    src/qt/models/walletmodel.cpp \
    src/net/rpcserver.cpp \
    src/net/rpcdump.cpp \
    src/qt/pages/overviewpage.cpp \
    src/qt/util/csvmodelwriter.cpp \
    src/wallet/crypter.cpp \
    src/qt/wallet/sendcoinsentry.cpp \
    src/qt/util/qvalidatedlineedit.cpp \
    src/qt/core/units.cpp \
    src/qt/util/qvaluecombobox.cpp \
    src/qt/dialogs/askpassphrasedialog.cpp \
    src/net/protocol.cpp \
    src/qt/util/notificator.cpp \
    src/qt/core/qtipcserver.cpp \
    src/qt/core/rpcconsole.cpp \
    src/core/kernel.cpp \
	src/main.cpp \
	src/core/global.cpp \
	src/LLD/keychain.cpp \
	src/LLD/index.cpp

RESOURCES += \
    src/qt/nexus.qrc \
    src/qt/Nexus.qrc

FORMS += \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/messagepage.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui

contains(USE_QRCODE, 1) {
HEADERS += src/qt/dialogs/qrcodedialog.h
SOURCES += src/qt/dialogs/qrcodedialog.cpp
FORMS += src/qt/forms/qrcodedialog.ui
}

CODECFORTR = UTF-8

# for lrelease/lupdate
# also add new translations to src/qt/nexus.qrc under translations/
TRANSLATIONS = $$files(src/qt/locale/nexus_*.ts)

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(TS_DIR):TS_DIR = src/qt/locale
# automatically build translations, so they can be included in resource file
TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$TS_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM
PRE_TARGETDEPS += compiler_TSQM_make_all

# "Other files" to show in Qt Creator
OTHER_FILES += \
    doc/*.rst doc/*.txt doc/README src/qt/res/nexus-qt.rc

# platform specific defaults, if not overridden on command line
isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    windows:BOOST_LIB_SUFFIX = -mgw49-mt-1_55
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH) {
    macx:BDB_LIB_PATH = /opt/local/lib/db48
}

isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -4.8
}

isEmpty(BDB_INCLUDE_PATH) {
    macx:BDB_INCLUDE_PATH = /opt/local/include/db48
}

isEmpty(BOOST_LIB_PATH) {
    BOOST_LIB_PATH=/usr/local/lib
    macx:BOOST_LIB_PATH = /opt/local/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    BOOST_INCLUDE_PATH=/usr/include/boost
    macx:BOOST_INCLUDE_PATH = /opt/local/include
}

windows:LIBS += -lws2_32 -lshlwapi
windows:DEFINES += WIN32
windows:RC_FILE = src/qt/res/nexus-qt.rc

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!windows:!macx {
    DEFINES += LINUX
    LIBS += -lrt
}

macx:HEADERS += src/qt/util/macdockiconhandler.h
macx:OBJECTIVE_SOURCES += src/qt/util/macdockiconhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0 Q_WS_MAC
macx:ICON = src/qt/res/icons/nexus.icns
macx:RC_ICONS = src/qt/res/icons/nexus.icns
macx:TARGET = "Nexus-Qt"

# Set libraries and includes at end, to use platform-defined defaults if not overridden
INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx$$BDB_LIB_SUFFIX
# -lgdi32 has to happen after -lcrypto (see  #681)
windows:LIBS += -lole32 -luuid -lgdi32
LIBS += -lboost_system$$BOOST_LIB_SUFFIX -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX

contains(RELEASE, 1) {
    !windows:!macx {
        # Linux: turn dynamic linking back on for c/c++ runtime libraries
        LIBS += -Wl,-Bdynamic -ldl
    }
}

system($$QMAKE_LRELEASE -silent $$_PRO_FILE_)
