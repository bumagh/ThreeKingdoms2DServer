lwanPath=/root/work/gitrep/lwan
# lwanPath=/root/lwan
buildPath="$lwanPath/build"
proPath="$lwanPath/build/src/samples/ThreeKingdoms2DServer"
cd "$buildPath"
cmake ..
cd "$proPath"
make clean
make