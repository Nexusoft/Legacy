## What is a Bootstrap?
A bootstrap is a copy of the blockchain database. This particular copy is for
the LLD (Lower Level Database) that was programmed and designed specifically
for nexus and other template database systesm. This tutorial will help you
learn how to copy in your bootstrap from your linux server / desktop to have a
faster syncing time.

## Steps to Complete
First ensure you have unzip installed

    su
    apt-get install unzip
    exit

Go back to your home folder

    cd ~

Nexus download a bootstrap from this URL

    wget https://nexusearth.com/bootstrap/LLD-Database/recent.zip
    unzip recent.zip

Now proceed to boot up the LLD node as normal.
