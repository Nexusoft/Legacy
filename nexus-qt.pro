TEMPLATE = app
TARGET = nexus-qt
VERSION = 0.1.0.0
INCLUDEPATH += src src/core src/hash src/json src/net src/qt src/util src/wallet src/LLD src/LLP
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE
win32:DEFINES+= BOOST_USE_WINDOWS_H WIN32_LEAN_AND_MEAN
CONFIG += no_include_pwd optimize_full c++11 thread 
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += uitools widgets
}
else {
	QMAKE_CXXFLAGS+= -std=c++11
}

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

#Allow verbose compiling output
isEmpty(VERBOSE) {
	VERBOSE= 0
	CONFIG+= silent warn_off
}
contains(VERBOSE, 1) {
	CONFIG+= warn_off
	!build_pass:message("Showing Verbose Output")
}
greaterThan(VERBOSE, 1) {
	CONFIG+= warn_on
	!build_pass:message("Showing Extra Verbose Output")
}

#Arch Linux Test
ARCH_TEST = $$system(uname -r | grep -o '....$')
contains(ARCH_TEST, ARCH) {
	OPENSSL_LIB_PATH = /usr/lib/openssl-1.0
	OPENSSL_INCLUDE_PATH = /usr/include/openssl-1.0
	LD_LIBRARY_PATH = /usr/lib
}

#Configure for 32/64 Bit
contains(32BIT, 1) {
	!build_pass:message("Building 32-Bit Version")
	BUILD_ARCH = x86
	QMAKE_LFLAGS += -m32
	QMAKE_CFLAGS += -m32
	QMAKE_CXXFLAGS += -m32
} else {
	!build_pass:message("Building 64-Bit Version")
	64BIT= 1
	BUILD_ARCH = x64
	QMAKE_LFLAGS += -m64
	QMAKE_CFLAGS += -m64
	QMAKE_CXXFLAGS += -m64
}
DEFINES += $$BUILD_ARCH

#Define default LIB and INCLUDE path variables
win32:isEmpty(DRIVE_LETTER) {
	DRIVE_LETTER=C
}
win32:isEmpty(BASE_DRIVE_LETTER) {
    BASE_DRIVE_LETTER=C
}
isEmpty(BASE_LIB_PATH) {
	win32:BASE_LIB_PATH=$$BASE_DRIVE_LETTER:/msys64/mingw64/lib
}
isEmpty(BASE_INCLUDE_PATH) {
	win32:BASE_INCLUDE_PATH=$$BASE_DRIVE_LETTER:/msys64/mingw64/include
}
isEmpty(BOOST_LIB_SUFFIX) {
	win32:BOOST_LIB_SUFFIX=-mgw82-mt-s-$$BUILD_ARCH-1_68
    macx:BOOST_LIB_SUFFIX = -mt
}
isEmpty(BOOST_LIB_PATH) {
	win32:BOOST_LIB_PATH=$$DRIVE_LETTER:/deps/lib
    macx:BOOST_LIB_PATH = /usr/local/opt/boost/lib
	!macx:!win32:BOOST_LIB_PATH=/usr/lib/x86_64-linux-gnu/
}
isEmpty(BOOST_INCLUDE_PATH) {
	win32:BOOST_INCLUDE_PATH=$$DRIVE_LETTER:/deps/include/boost-1_68
    macx:BOOST_INCLUDE_PATH = /usr/local/opt/boost/include
	!macx:!win32:BOOST_INCLUDE_PATH=/usr/include/boost
}
isEmpty(OPENSSL_LIB_PATH) {
	win32:OPENSSL_LIB_PATH=$$DRIVE_LETTER:/deps/lib
    macx:OPENSSL_LIB_PATH = /usr/local/opt/openssl/lib
}
isEmpty(OPENSSL_INCLUDE_PATH) {
	win32:OPENSSL_INCLUDE_PATH=$$DRIVE_LETTER:/deps/include
    macx:OPENSSL_INCLUDE_PATH = /usr/local/opt/openssl/include
}
isEmpty(BDB_LIB_PATH) {
	win32:BDB_LIB_PATH=$$DRIVE_LETTER:/deps/lib
    macx:BDB_LIB_PATH = /usr/local/opt/db/lib
}
isEmpty(BDB_LIB_SUFFIX) {
    macx:BDB_LIB_SUFFIX = -18.1
}
isEmpty(BDB_INCLUDE_PATH) {
	win32:BDB_INCLUDE_PATH=$$DRIVE_LETTER:/deps/include
    macx:BDB_INCLUDE_PATH = /usr/local/opt/db/include
}
isEmpty(MINIUPNPC_LIB_PATH) {
	win32:MINIUPNPC_LIB_PATH=$$DRIVE_LETTER:/deps/lib
    macx:MINIUPNPC_LIB_PATH = /usr/local/opt/miniupnpc/lib
}
isEmpty(MINIUPNPC_INCLUDE_PATH) {
	win32:MINIUPNPC_INCLUDE_PATH=$$DRIVE_LETTER:/deps/include
    macx:MINIUPNPC_INCLUDE_PATH = /usr/local/opt/miniupnpc/include
}
isEmpty(QRENCODE_LIB_PATH) {
	win32:QRENCODE_LIB_PATH=$$DRIVE_LETTER:/deps/lib
	macx:QRENCODE_LIB_PATH = /usr/local/opt/qrencode/lib
}
isEmpty(QRENCODE_INCLUDE_PATH) {
	win32:QRENCODE_INCLUDE_PATH=$$DRIVE_LETTER:/deps/include
	macx:QRENCODE_INCLUDE_PATH = /usr/local/opt/qrencode/include
}
isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

#Build Type
isEmpty(DEBUG) {
	isEmpty(RELEASE) {
		RELEASE = 1
	}
}

contains(DEBUG, 1) {
	contains (RELEASE, 1) {
		DEBUG_AND_RELEASE = 1
	}
}
contains(DEBUG_AND_RELEASE, 1) {
	!build_pass:message("Building Debug and Release Version")
	DEBUG = 1
	RELEASE = 1
	CONFIG += debug_and_release
}

#Debug Version Config
contains(DEBUG, 1) {
	isEmpty(RELEASE) {
		!build_pass:message("Building Debug Version")
		CONFIG += debug
	}
	QMAKE_CXXFLAGS += -g
}

#Release Version Config
contains(RELEASE, 1) {
	isEmpty(DEBUG) {
		!build_pass:message("Building Release Version")
		CONFIG += release
		DEFINES += NDEBUG
	}
    !build_pass:message("Building with Static Linking")
    # Mac: compile for Yosemite and Above (10.10, 32/64-bit)
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.10 -arch x86_64

    #-sysroot /Developer/SDKs/MacOSX.sdk

    #Static Configuration
    win32:CONFIG += STATIC
	
    win32:QMAKE_LFLAGS += -Wl,--dynamicbase -Wl,--nxcompat -static -static-libgcc -static-libstdc++
    !win32:!macx {
        # Linux: static link
        LIBS += -Wl,-Bstatic
    }
}

#QREncode Support Config
contains(USE_QRCODE, 1) {
    !build_pass:message("Building with QRCode support")
    DEFINES += USE_QRCODE
	INCLUDEPATH+= $$QRENCODE_INCLUDE_PATH
    LIBS += $$join(QRENCODE_LIB_PATH,,-L,) -lqrencode
	HEADERS += src/qt/dialogs/qrcodedialog.h
	SOURCES += src/qt/dialogs/qrcodedialog.cpp
	FORMS += src/qt/forms/qrcodedialog.ui
}

#UPNP Support Config
contains(NO_UPNP, 1) {
    !build_pass:message("Building without UPNP support")
	DEFINES += USE_UPNP=0
} else {
	!build_pass:message("Building with UPNP support")
    DEFINES += USE_UPNP=1 MINIUPNP_STATICLIB
    INCLUDEPATH += $$MINIUPNPC_INCLUDE_PATH
    LIBS += $$join(MINIUPNPC_LIB_PATH,,-L,) -lminiupnpc
    win32:LIBS += -liphlpapi
}

#Oracle DB Support Config
contains(ORACLE, 1) {
	!build_pass:message("Building with Berkeley Database Support (Oracle)")
} else {
	!build_pass:message("Building with Lower Level Database Support")
	DEFINES += USE_LLD
}

#DBUS Support Config
contains(USE_DBUS, 1) {
    !build_pass:message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
    QT += dbus
}

# use: qmake "MESSAGE_TAB=1"
contains(MESSAGE_TAB, 1) {
    !build_pass:message(Building with Messaging Tab Enabled)
    DEFINES += FIRST_CLASS_MESSAGING
}

#QTPLUGIN helper
contains(NEXUS_NEED_QT_PLUGINS, 1) {
    DEFINES += NEXUS_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

!win32 {
    # for extra security against potential buffer overflows
    QMAKE_CXXFLAGS += -fstack-protector
    QMAKE_LFLAGS += -fstack-protector
    # do not enable this on windows, as it will result in a non-working executable!
}

!macx:QMAKE_LFLAGS += -s
QMAKE_CFLAGS += -s
QMAKE_CXXFLAGS += -s -D_FORTIFY_SOURCE=2 -fpermissive
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wextra -Wformat -Wformat-security -Wno-invalid-offsetof -Wno-sign-compare -Wno-unused-parameter
!macx {
    QMAKE_CXXFLAGS_WARN_ON += -fdiagnostics-show-option

    #To Compile the Keccak C Files (For Linux and Windoze)
    MAKE_EXT_CPP  += .c
}

#DISTCLEAN function helper
QMAKE_DEL_FILE = rm -rf
QMAKE_DISTCLEAN += build/moc build/obj build/ui
macx:QMAKE_DISTCLEAN += nexus-qt.dmg dist
build_pass:DebugBuild {
	QMAKE_DISTCLEAN += object_script.nexus-qt.Debug
	win32:QMAKE_DISTCLEAN += debug
}
build_pass:ReleaseBuild {
	QMAKE_DISTCLEAN += object_script.nexus-qt.Release
}
					
#Source File List
DEPENDPATH += src \
    src/LLP \
    src/LLD \
    src/core \
    src/hash \
    src/json \
    src/net \
    src/util \
    src/qt \
    src/wallet \
    src/qt/core \
    src/qt/dialogs \
    src/qt/forms \
    src/qt/models \
    src/qt/pages \
    src/qt/res \
    src/qt/util \
    src/qt/wallet
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
    src/LLD/trustkeys.h \
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
    src/qt/core/qtipcserver.h \
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
    src/LLD/trustkeys.cpp \
    src/LLD/index.cpp
RESOURCES += src/qt/nexus.qrc
FORMS += src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/messagepage.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui
OTHER_FILES += src/qt/res/nexus-qt.rc

#Translation Support config
TRANSLATIONS = $$files(src/qt/locale/nexus_*.ts)
TS_DIR = src/qt/locale

#Translation Support Custom Compiler
TSQM.name = translator
TSQM.input = TRANSLATIONS
TSQM.output = $$TS_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM
PRE_TARGETDEPS += compiler_TSQM_make_all

#Windows build helper
win32:LIBS += -lws2_32 -lshlwapi
win32:DEFINES += WIN32
win32:RC_FILE = src/qt/res/nexus-qt.rc
win32:!contains(MINGW_THREAD_BUGFIX, 0) {
    # At least qmake's win32-g++-cross profile is missing the -lmingwthrd
    # thread-safety flag. GCC has -mthreads to enable this, but it doesn't
    # work with static linking. -lmingwthrd must come BEFORE -lmingw, so
    # it is prepended to QMAKE_LIBS_QT_ENTRY.
    # It can be turned off with MINGW_THREAD_BUGFIX=0, just in case it causes
    # any problems on some untested qmake profile now or in the future.
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

#OSX build helper
macx:HEADERS += src/qt/util/macdockiconhandler.h
macx:OBJECTIVE_SOURCES += src/qt/util/macdockiconhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0 Q_WS_MAC
macx:ICON = src/qt/res/icons/nexus.icns
macx:RC_ICONS = src/qt/res/icons/nexus.icns

#Build final Includes and Libraries
INCLUDEPATH += $$BOOST_INCLUDE_PATH \
	$$BDB_INCLUDE_PATH \
	$$OPENSSL_INCLUDE_PATH \
	$$BASE_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) \
	$$join(BDB_LIB_PATH,,-L,) \
	$$join(OPENSSL_LIB_PATH,,-L,) \
	$$join(BASE_LIB_PATH,,-L,) \
	-ldb_cxx$$BDB_LIB_SUFFIX \
	-lboost_system$$BOOST_LIB_SUFFIX \
	-lboost_filesystem$$BOOST_LIB_SUFFIX \
	-lboost_program_options$$BOOST_LIB_SUFFIX \
	-lboost_thread$$BOOST_LIB_SUFFIX \
	-lssl \
	-lcrypto

win32:LIBS += -lole32 -luuid -lgdi32

#Fix for Arch Linux OpenSSL Version
contains(ARCH_TEST, ARCH) {
	LIBS -= -Wl,-Bstatic \
		-lssl \
		-lcrypto
	LIBS += /usr/lib/openssl-1.0/libssl.so \
		/usr/lib/openssl-1.0/libcrypto.so
}

#Fix for linux dynamic linking
!macx:!win32:DEFINES += LINUX
!macx:!win32:contains(RELEASE, 1) {
	LIBS+=-Wl,-Bdynamic -ldl -lrt
}
#Perform Translations
!build_pass:system($$QMAKE_LRELEASE -silent $$TRANSLATIONS)
!build_pass:message("Translations Generated")

#Extra Console Output
!build_pass:message("Finishing up... Type 'make' to start compiling when finished")
#Ending makefile text
complete.target= complete
win32:complete.commands= @echo '' && echo 'Finished Building nexus-qt.exe' && echo ''
macx:complete.commands= { echo ' '; echo 'Finished building nexus-qt.app'; echo ' '; } 2> /dev/null
!win32:!macx:complete.commands= @echo '' && echo 'Finished Building nexus-qt' && echo ''
QMAKE_EXTRA_TARGETS+= complete
POST_TARGETDEPS+= complete
