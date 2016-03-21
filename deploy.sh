# build openflow switch support
cd ofsid/
./waf configure
./waf build
var=$(pwd)
cd ../


# configure ns-3 enabling openflow support
./waf distclean
./waf configure --with-openflow=$var ###add here further options if required
cp -r ofsid/include/openflow/ build/


# build ns-3
./waf build
