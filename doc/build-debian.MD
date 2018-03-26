## Dependency Build Instructions: Ubuntu & Debian

1. First ensure you have build essentials installed:

       sudo apt-get install git build-essential libboost-all-dev libssl-dev libdb-dev libdb++-dev libminiupnpc-dev libqrencode-dev lib32z1-dev qt5-default qttools5-dev*

## Build Instructions (Daemon): Ubuntu & Debian

    cd ~
    git clone --depth 1 https://github.com/Nexusoft/Nexus
    cd Nexus
    make -f makefile.cli

To execute:

    ./nexus

## Build Instructions (Qt): Ubuntu & Debian

    cd ~
    git clone --depth 1 https://github.com/Nexusoft/Nexus
    cd Nexus
    qmake
    make

To execute:

    ./nexus-qt
    
    
