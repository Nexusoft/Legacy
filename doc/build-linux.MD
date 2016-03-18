<h2>Dependencies</h2>

 Library     Purpose           Description
 -------     -------           -----------
 libssl      SSL Support       Secure communications
 libdb4.8    Berkeley DB       Blockchain & wallet storage
 libboost    Boost             C++ Library
 miniupnpc   UPnP Support      Optional firewall-jumping support
 libqrencode QRCode generation Optional QRCode generation


<h2>Compile Options</h3>

miniupnpc may be used for UPnP port mapping.  
Set USE_UPNP to a different value to control this:
 USE_UPNP=-    No UPnP support - miniupnp not required
 USE_UPNP=0    (the default) UPnP support turned off by default at runtime
 USE_UPNP=1    UPnP support turned on by default at runtime

libqrencode may be used for QRCode image generation.
Set USE_QRCODE to control this:
 USE_QRCODE=0   (the default) No QRCode support - libqrcode not required
 USE_QRCODE=1   QRCode support enabled

Licenses of statically linked libraries:
 Berkeley DB   New BSD license with additional requirement that linked
               software must be free open source
 Boost         MIT-like license
 miniupnpc     New (3-clause) BSD license
 
 
<h2>Dependency Build Instructions: Ubuntu & Debian</h2>

First ensure you have build essentials installed:<br>
sudo apt-get install build-essentials

Install Boost:<br>
sudo apt-get install libboost-all-dev

Install Berklee DB:<br>
sudo apt-get install libdb-dev libdb++-dev

Install Open SSL:<br>
sudo apt-get install libssl-dev

Install Mini UPNP:<br>
sudo apt-get install libminiupnpc-dev

Install QrenCode:<br>
sudo apt-get install libqrencode-dev

For the Qt, install QT Framework:<br>
sudo apt-get install qt4-qmake libqt4-dev

<h2>Build Instructions (Daemon): Ubuntu & Debian</h2>

cd ~<br>
git clone https://github.com/Nexusoft/Nexus.git Nexus<br>
cd Nexus<br>
make -f makefile.unix<br>

To Execute:<br>
./nexus


<h2>Build Instructions (Qt): Ubuntu & Debian</h2>

cd ~<br>
git clone https://github.com/Nexusoft/Nexus.git Nexus<br>
cd Nexus<br>
qmake nexus-qt.pro "RELEASE=1" "USE_UPNP=1"<br>
make<br>

To Execute:<br>
./nexus-qt

