### Installing Dependencies ###

##### Install Xcode Tools and Brew #####

Open Finder, go to Utilities, and open Terminal.

To install the xcode command line tools, run:

*	xcode-select --install

Click Install, then Agree. When its done, run:

*	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

Press ENTER to continue when prompted, then type your password to install. When it finishes, run:
	
*	brew install berkeley-db boost git qrencode miniupnpc openssl qt
*	echo 'export PATH="/usr/local/opt/qt/bin:$PATH"' >> ~/.bash_profile
*	source ~/.bash_profile

### Compiling Wallet ###

Choose which version you want to use below and follow the instructions for it.
Almost all standard users will want to build the Nexus-Qt (GUI) version.

##### To Build the Nexus-Qt version (GUI) #####

With the prerequisite software installed, do the following:

*	git clone --depth 1 https://github.com/Nexusoft/Nexus.git
*	cd Nexus
*	qmake
*	make
    
(If you wish to build Oracle DB version, use "qmake ORACLE=1" instead)

To deploy this *.app file (Create a dmg), do the following:

*	chmod +x create_dmg.sh
*	./create_dmg.sh

After this step, you should find Nexus-Qt.dmg in your working directory.

### To build the command line version (CLI) #####

Make sure you are in your home directory, or the directory you would like the source code to be.

*	git clone --depth 1 https://github.com/Nexusoft/Nexus.git
*	cd Nexus
*	make -f makefile.cli
