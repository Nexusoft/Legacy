## Install Prerequisites

Using MacPorts (or HomeBrew, but these instructions will assume MacPorts), install the required libraries. +universal option for easy compiling of i386 or x86_64 architectures

    sudo port install boost db48
    sudo port install boost +universal -no_static
    sudo port install openssl +universal
    sudo port install miniupnpc +universal 
    sudo port install qrencode +universal

Install QT (Ensure to use +framework for deployment)

    sudo port install qt4-mac +framework +universal

**Important**: The makefile and QT project file assume a build for a 32-bit target for maximum compatibility. Edit /opt/local/etc/macports/macports.conf and uncomment "build_arch i386". It is possible to have a second 32-bit install just for these builds.

## Building Nexus Daemon

Make sure yua are in your home directory, or the directory you would like the source code to be.

    git clone https://github.com/Nexusoft/Nexus.git
    cd Nexus
    make -f makefile.osx RELEASE=1 STATIC=1 USE_UPNP=1
    strip nexus

## Building Nexus-Qt
<<<<<<< HEAD
Ensure appscript is installed to run the automated python script for easy deployment
	sudo easy_install --install-dir /opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7 appscript
=======
>>>>>>> master

With the prerequisite software installed, do the following:

    git clone https://github.com/Nexusoft/Nexus.git
    cd Nexus
    
If you want to build with Oracle DB support use the following command:

    qmake RELEASE=1 USE_UPNP=1 USE_QRCODE=1 nexus-qt.pro
    make
<<<<<<< HEAD
    export QTDIR=/opt/local/libexec/qt4/share
    python2.7 release/macdeploy/macdeployqtplus Nexus-Qt.app -add-qt-tr da,de,es,hu,ru,uk,zh_CN,zh_TW -dmg -fancy release/macdeploy/fancy.plist -verbose 2
=======
    
If you want to build with LLD support use the following command:

	qmake RELEASE=1 USE_UPNP=1 USE_QRCODE=1 USE_LLD=1 nexus-qt.pro
	make
	
If you would like to deploy this *.app file, ensure appscript is installed to run the automated python script for easy deployment:
	
	sudo port install py27-pip
	port select --set pip pip27
	sudo -H pip install appscript
	
Then build the *.dmg file with the macdeployplus python script:

	export QTDIR=/opt/local/libexec/qt4/share
	python2.7 release/macdeploy/macdeployqtplus Nexus-Qt.app -dmg -fancy release/macdeploy/fancy.plist -verbose 2
>>>>>>> master

After this step, you should find Nexus-Qt.dmg in your working directory.
