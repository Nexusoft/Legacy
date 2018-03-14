#Create directories
mkdir /c/deps /c/installfiles
#Download Dependencies
cd /c/installfiles
wget http://download.oracle.com/berkeley-db/db-6.2.32.NC.tar.gz -O/c/installfiles/libdb.tar.gz
wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz -O/c/installfiles/boost.tar.gz
wget https://sourceforge.net/projects/libpng/files/libpng16/1.6.34/lpng1634.zip/download -O/c/installfiles/libpng.zip
wget http://miniupnp.free.fr/files/miniupnpc-2.0.20180222.tar.gz -O/c/installfiles/miniupnpc.tar.gz
wget https://fukuchi.org/works/qrencode/qrencode-4.0.0.tar.gz -O/c/installfiles/qrencode.tar.gz
wget https://www.openssl.org/source/openssl-1.0.2n.tar.gz -O/c/installfiles/openssl.tar.gz
wget http://download.qt.io/official_releases/qt/5.10/5.10.1/single/qt-everywhere-src-5.10.1.tar.xz -O/c/installfiles/qt.tar.xz
#Extract Dependencies
tar zxvf boost.tar.gz && mv boost_1_66_0 boost
tar zxvf libdb.tar.gz && mv db-6.2.32.NC libdb
unzip libpng.zip && mv lpng1634 libpng
tar zxvf qrencode.tar.gz && mv qrencode-4.0.0 qrencode
tar zxvf miniupnpc.tar.gz && mv miniupnpc-2.0.20180222 miniupnpc
tar zxvf openssl.tar.gz && mv openssl-1.0.2n openssl
tar Jxvf qt.tar.xz && mv qt-everywhere-src-5.10.1 qt
##Compile boost
cd /c/installfiles/boost
./bootstrap.sh --with-toolset=gcc --with-libraries=filesystem,program_options,system,thread --prefix=/deps
./b2 --build-type=complete variant=release link=static threading=multi runtime-link=static install
#Compile libdb
cd /c/installfiles/libdb/build_unix
../dist/configure --prefix=/c/deps --enable-mingw --enable-cxx --disable-shared --disable-replication
make
make install
#Compile libpng
cd /c/installfiles/libpng
make -f scripts/makefile.msys install-static prefix=/c/deps
#Compile qrencode
cd /c/installfiles/qrencode
./configure --enable-static --disable-shared --without-tools --prefix=/c/deps
make install
#Compile openssl
cd /c/installfiles/openssl
./configure no-shared mingw64 -m64 --prefix=/c/deps
make
make install
#Compile miniupnpc
cmd /C "cd C:/installfiles/miniupnpc && mingw32-make -f Makefile.mingw init upnpc-static"
cd /c/installfiles/miniupnpc
mkdir /c/deps/include/miniupnpc
cp *.h /c/deps/include/miniupnpc
cp *.a /c/deps/lib
cp *.exe /c/deps/bin
#Compile qt
cd /c/installfiles/qt
./configure -release -opensource -confirm-license -static -platform win32-g++ -no-sql-sqlite -qt-zlib -qt-libpng -qt-libjpeg -no-style-fusion -style-windows -style-windowsvista -nomake examples -no-opengl -no-openssl -no-dbus -skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d -skip qtdoc -skip qtgamepad -skip qtgraphicaleffects -skip qtlocation -skip qtmacextras -skip qtmultimedia -skip qtnetworkauth -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 -skip qtremoteobjects -skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qtsvg -skip qttranslations -skip qtvirtualkeyboard -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebglplugin -skip qtwebsockets -skip qtwebview -skip qtwinextras -skip qtx11extras -skip qtxmlpatterns -prefix "C:\deps" QMAKE_CFLAGS+="-m64" QMAKE_CXXFLAGS+="-m64" QMAKE_LFLAGS+="-m64"
make
make install
#Set dependent PATH environment variable and make it permanent in MinGW
cd ~
echo "export PATH=/c/deps/bin:$PATH" >> ~/.bashrc
source ~/.bashrc
#Clear screen and alert user Nexus is now ready to compile
cd /c/Nexus
clear
echo " "
echo "All dependencies are now built and Nexus is ready to compile."
echo " "