## Install Prerequisites

Open Finder, go to Utilities, and open Terminal.

To install the xcode command line tools, run:

	xcode-select --install

Click Install, then Agree. When its done, run:

	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

Press ENTER to continue when prompted, then type your password to install. When it finishes, run:
	
    brew install berkeley-db boost git qrencode miniupnpc openssl qt
	echo 'export PATH="/usr/local/opt/qt/bin:$PATH"' >> ~/.bash_profile
	source ~/.bash_profile

## Building Nexus Daemon

Make sure you are in your home directory, or the directory you would like the source code to be.

    git clone --depth 1 https://github.com/Nexusoft/Nexus.git
    cd Nexus
	make -f makefile.osx
(If you wish to build Oracle DB version, use "make -f makefile.osx ORACLE=1" instead)	

## Building Nexus-Qt

With the prerequisite software installed, do the following:

    git clone --depth 1 https://github.com/Nexusoft/Nexus.git
    cd Nexus
	qmake
	make
    
(If you wish to build Oracle DB version, use "qmake ORACLE=1" instead)

To deploy this *.app file (Create a dmg), do the following:
	
	./create_dmg.sh

After this step, you should find Nexus-Qt.dmg in your working directory.
