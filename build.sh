echo "Building Nexus..."
rm -f nexus
make -f makefile.unix USE_LLD=1

echo "Executing Nexus..."
./nexus -printtoconsole -verbose=2
