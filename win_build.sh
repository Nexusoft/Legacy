clear
#Detect OS. Run if MINGW64
export osdetect=$(uname -s | grep -o '^.......')
if [ $(uname -s | grep -o '^.......') == MINGW64 ]; then
	#if more or less than 1 argument, don't run.
	if [ $# != 1 ]; then
		echo -e "You must specify either install, update, or clean when running this script. \n"
		echo -e "install - Downloads and installs the required dependencies for compiling the Nexus wallet. \n"
		echo -e "update - Downloads newest version of Nexus wallet source code.\n"
        echo -e "clean - Removes all downloaded and installed dependents.\n"
		echo -e "For example '/c/Nexus/win_build.sh install'"
		exit
	fi
	#Install and compile all dependencies for Nexus wallet
	if [ $1 == install ]; then
		#Create directories if they don't already exist.
			if [ ! -d /c/deps ]; then mkdir /c/deps; fi
			if [ ! -d /c/installfiles ]; then mkdir /c/installfiles; fi
			#Download Dependencies
			cd /c/installfiles
			if [ ! -d libdb ]; then
				echo -e "\nDownloading Berkeley DB"
				wget -c --append-output=/dev/null --show-progress -N http://download.oracle.com/berkeley-db/db-6.2.32.NC.tar.gz
			fi
			if [ ! -d boost ]; then
				echo -e "\nDownloading Boost"
				wget -c --append-output=/dev/null --show-progress -N https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
			fi
			if [ ! -d libpng ]; then
				echo -e "\nDownloading LibPNG"
				wget -c --append-output=/dev/null --show-progress -N https://sourceforge.net/projects/libpng/files/libpng16/1.6.35/lpng1635.zip
			fi
			if [ ! -d miniupnpc ]; then
				echo -e "\nDownloading Miniupnpc"
				wget -c --append-output=/dev/null --show-progress -N http://miniupnp.free.fr/files/miniupnpc-2.1.tar.gz
			fi
			if [ ! -d qrencode ]; then
				echo -e "\nDownloading QRencode"
				wget -c --append-output=/dev/null --show-progress -N https://fukuchi.org/works/qrencode/qrencode-4.0.2.tar.gz
			fi
			if [ ! -d openssl ]; then
				echo -e "\nDownloading OpenSSL"
				wget -c --append-output=/dev/null --show-progress -N https://www.openssl.org/source/openssl-1.0.2p.tar.gz
			fi
			if [ ! -d qt ]; then
				echo -e "\nDownloading Qt"
				wget -c --append-output=/dev/null --show-progress -N http://download.qt.io/official_releases/qt/5.10/5.10.1/single/qt-everywhere-src-5.10.1.tar.xz
			fi
			#Extract Dependencies and show rough estimate progress bar.
			clear
			if [ ! -d boost ]; then
				echo -e "\nExtracting Boost"
				tar zvxf boost_1_68_0.tar.gz | pv -l -s 63468 > /dev/null && mv boost_1_68_0 boost
			fi
			if [ ! -d libdb ]; then
				echo -e "\nExtracting Berkeley DB"
				tar zvxf db-6.2.32.NC.tar.gz | pv -l -s 11349 > /dev/null && mv db-6.2.32.NC libdb
			fi
			if [ ! -d libpng ]; then
				echo -e "\nExtracting LibPNG"
				unzip lpng1635.zip | pv -l -s 379 > /dev/null && mv lpng1635 libpng
			fi
			if [ ! -d qrencode ]; then
				echo -e "\nExtracting QRencode"
				tar zvxf qrencode-4.0.2.tar.gz | pv -l -s 84 > /dev/null && mv qrencode-4.0.2 qrencode
			fi
			if [ ! -d miniupnpc ]; then
				echo -e "\nExtracting Miniupnpc"
				tar zvxf miniupnpc-2.1.tar.gz | pv -l -s 83 > /dev/null && mv miniupnpc-2.1 miniupnpc
			fi
			if [ ! -d openssl ]; then
				echo -e "\nExtracting OpenSSL"
				tar zvxf openssl-1.0.2p.tar.gz | pv -l -s 2496 > /dev/null && mv openssl-1.0.2p openssl
			fi
			if [ ! -d qt ]; then
				echo -e "\nExtracting QT"
				tar Jvxf qt-everywhere-src-5.10.1.tar.xz | pv -l -s 206858 > /dev/null && mv qt-everywhere-src-5.10.1 qt
			fi
			#If files extracted to proper folders, delete the archives.
			if [ -d boost ]; then rm boost_1_68_0.tar.gz; fi
			if [ -d libdb ]; then rm db-6.2.32.NC.tar.gz; fi
			if [ -d libpng ]; then rm lpng1635.zip; fi
			if [ -d qrencode ]; then rm qrencode-4.0.2.tar.gz; fi
			if [ -d miniupnpc ]; then rm miniupnpc-2.1.tar.gz; fi
			if [ -d openssl ]; then rm openssl-1.0.2p.tar.gz; fi
			if [ -d qt ]; then rm qt-everywhere-src-5.10.1.tar.xz; fi
			##Compile boost
			clear
			cd /c/installfiles/boost
			mkdir builddir
			./bootstrap.sh --with-toolset=gcc --without-icu --with-libraries=filesystem,program_options,system,thread --prefix=/deps
			./b2.exe -d0 -d+1 --build-type=complete --build-dir=/installfiles/boost/builddir --with-filesystem --with-program_options --with-system --with-thread variant=release link=static threading=multi runtime-link=static install
			#Compile libdb
			cd /c/installfiles/libdb/build_unix
			../dist/configure --prefix=/c/deps --silent --enable-mingw --enable-cxx --enable-static --enable-shared --with-cryptography=no --disable-replication
			make -j2
			make -j2 install
			#Compile libpng
			cd /c/installfiles/libpng
			./configure --prefix=/c/deps --enable-silent-rules --build=mingw64
            make -j2
            make -j2 install
			#Compile qrencode
			cd /c/installfiles/qrencode
			./configure --enable-silent-rules --enable-static --enable-shared --without-tools --prefix=/c/deps
			make -j2
			make -j2 install
			#Compile openssl
			cd /c/installfiles/openssl
			./configure no-hw threads mingw64 --prefix=/c/deps
			make --silent
			make --silent install
			#Compile miniupnpc
			cmd /C "cd C:/installfiles/miniupnpc && mingw32-make --silent -f Makefile.mingw init upnpc-static upnpc-shared"
			cd /c/installfiles/miniupnpc
			mkdir /c/deps/include/miniupnpc
			cp *.h /c/deps/include/miniupnpc
			cp *.a /c/deps/lib
			cp *.exe /c/deps/bin
			#Compile qt
			cd /c/installfiles/qt
			./configure -release -opensource -confirm-license -static -platform win32-g++ -silent -no-sql-sqlite -qt-zlib -qt-libpng -qt-libjpeg -nomake examples -no-opengl -no-openssl -no-dbus -skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcanvas3d -skip qtcharts -skip qtconnectivity -skip qtdatavis3d -skip qtdoc -skip qtgamepad -skip qtgraphicaleffects -skip qtimageformats -skip qtlocation -skip qtmacextras -skip qtmultimedia -skip qtnetworkauth -skip qtpurchasing -skip qtquickcontrols -skip qtquickcontrols2 -skip qtremoteobjects -skip qtscript -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtspeech -skip qtsvg -skip qttranslations -skip qtvirtualkeyboard -skip qtwayland -skip qtwebchannel -skip qtwebengine -skip qtwebglplugin -skip qtwebsockets -skip qtwebview -skip qtwinextras -skip qtx11extras -skip qtxmlpatterns -prefix "C:\deps" QMAKE_CFLAGS+="-w -m64" QMAKE_CXXFLAGS+="-w -m64" QMAKE_LFLAGS+="-w -m64"
			make -j2 --silent
			make -j2 --silent install
			#Set dependent PATH environment variable and make it permanent in MinGW
			cd ~
			echo -e "export PATH=/c/deps/bin:$PATH" >> ~/.bashrc
			source ~/.bashrc
			#Clear screen and delete installfiles folder
			clear
			echo -e " "
			echo -e " Files are being cleaned up. This may take several minutes. Please be patient"
			echo -e " "
			cd /c/
			sleep 3
			rm -frv /c/installfiles | pv -l -s $(find /c/installfiles | wc -l) > /dev/null
			#Clear screen and alert user Nexus is now ready to compile
			cd /c/Nexus
			clear
			echo -e " "
			echo -e "All dependencies are now built and Nexus is ready to compile."
			echo -e " "
		#Update Nexus source code from github and backup old Nexus source code to Nexus.bak.~#~ with
		#increasing numbers based on number of backups
		elif [ $1 == update ]; then
			clear
			echo -e "\nUpdating Nexus source to latest version.\n"
			if [ -d /c/nxstempdir ]; then rm -rf /c/nxstempdir; else mkdir /c/nxstempdir; fi
			cd /c/nxstempdir
			echo "cd /c/" > /c/nxstempdir/temp.sh
			echo "mv --backup=numbered -T /c/Nexus /c/Nexus.bak" >> /c/nxstempdir/temp.sh
			echo "git clone --depth 1 https://github.com/Nexusoft/Nexus &> /dev/null" >> /c/nxstempdir/temp.sh
			echo "cd /c/Nexus" >> /c/nxstempdir/temp.sh
			echo "/c/Nexus.bak/win_build.sh updatedsource" >> /c/nxstempdir/temp.sh
			/c/nxstempdir/temp.sh & disown
		#Special argument for finishing source update. Not to be specified manually.
		elif [ $1 == updatedsource ]; then
			clear
			rm -rf /c/nxstempdir
			echo -e "\nSource code updated.\n\nPress Enter to return to terminal"
		#Clean up all files and start over. Used if theres massive errors when building. Also shows a rough estimate progress bar.
		elif [ $1 == clean ]; then
			cd /c/
			if [ -d deps ]; then
				echo -e "\nDeleting C:\deps folder"
				rm -frv /c/deps | pv -l -s $(find /c/deps | wc -l) > /dev/null
			fi
			if [ -d installfiles ]; then
				echo -e "\nDeleting C:\installfiles folder"
				rm -frv /c/installfiles | pv -l -s $(find /c/installfiles | wc -l) > /dev/null
			fi
			if [ -d nxstempdir ]; then
				echo -e "\nDeleting C:\nxstempdir folder"
				rm -rf nxstempdir
			fi
			echo -e "\nSuccessfully removed all files.\n"
		#Invalid argument detection
		else
			echo -e "Invalid argument detected - " $1
			echo -e "\nYou must specify either install or update when running this script. \n"
			echo -e "install - Downloads and installs the required dependencies for compiling the Nexus wallet. \n"
			echo -e "update - Downloads newest version of Nexus wallet source code.\n"
			echo -e "clean - Removes all downloaded and installed dependents.\n"
			echo -e "For example '/c/Nexus/win_build.sh install'"
		fi
#Do not run if NOT MINGW64
else
	clear
	echo -e "\nThis script should only be ran on Windows using MinGW64. Exiting.\n"
fi
