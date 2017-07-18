<h2>Dependencies</h2>

<table>
<tr><td>Library</td><td>Purpose</td><td>Description</td></tr>
<tr><td>libssl</td><td>SSL Support</td><td>Secure communications</td></tr>
<tr><td>libdb4.8</td><td>Berkeley DB</td><td>Blockchain & wallet storage</td></tr>
<tr><td>libboost</td><td>Boost</td><td>C++ Library</td></tr>
<tr><td>miniupnpc</td><td>UPnP Support</td><td>Optional firewall-jumping support</td></tr>
<tr><td>libqrencode</td><td>QRCode generation</td><td>Optional QRCode generation</td></tr>
</table>


<h2>Compile Options</h3>

<table>
<tr><td colspan="2">miniupnpc may be used for UPnP port mapping.</td></tr>
<tr><td colspan="2">Set USE_UPNP to a different value to control this:</td></tr>
<tr><td>USE_UPNP=-</td><td>No UPnP support - miniupnp not required</td></tr>
<tr><td>USE_UPNP=0</td><td>(the default) UPnP support turned off by default at runtime</td></tr>
<tr><td>USE_UPNP=1</td><td>UPnP support turned on by default at runtime</td></tr>
</table>

<table>
<tr><td colspan="2">libqrencode may be used for QRCode image generation.</td></tr>
<tr><td colspan="2">Set USE_QRCODE to control this:</td></tr>
<tr><td>USE_QRCODE=0</td><td>(the default) No QRCode support - libqrcode not required</td></tr>
<tr><td>USE_QRCODE=1</td><td>QRCode support enabled</td></tr>
</table>
 
<h2>Dependency Build Instructions: Ubuntu & Debian</h2>

<b>First ensure you have build essentials installed:</b><br>
<i>sudo apt-get install build-essential</i>

<b>Install Boost:</b><br>
<i>sudo apt-get install libboost-all-dev</i>

<b>Install Berklee DB:</b><br>
<i>sudo apt-get install libdb-dev libdb++-dev</i>

<b>Install Open SSL:</b><br>
<i>sudo apt-get install libssl1.0-dev</i>

If this fails to install because your linux system still supports 0.X versions of openssl use this command:

<i>sudo apt-get install libssl-dev</i>

<b>Install Mini UPNP:</b><br>
<i>sudo apt-get install libminiupnpc-dev</i>

<b>Install QrenCode:</b><br>
<i>sudo apt-get install libqrencode-dev</i>

<b>For the Qt, install QT Framework:</b><br>
<i>sudo apt-get install qt4-qmake libqt4-dev</i>

<b>If you get a filed to build because "-lz" was not found this is due to the fact that your linux operating system is trying to use a 64 bit version of zlib. You can solve this by installing the 32 bit version here:
<i>sudo apt-get install lib32z1-dev</i>


<h2>Build Instructions (Daemon): Ubuntu & Debian</h2>

cd ~<br>
git clone https://github.com/Nexusoft/Nexus.git Nexus<br>
cd Nexus<br>
make -f makefile.unix<br>

<b>To Execute:</b><br>
./nexus


<h2>Build Instructions (Qt): Ubuntu & Debian</h2>

cd ~<br>
git clone https://github.com/Nexusoft/Nexus.git Nexus<br>
cd Nexus<br>
qmake nexus-qt.pro "RELEASE=1" "USE_UPNP=1"<br>
make<br>

<b>To Execute:</b><br>
./nexus-qt

