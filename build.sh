lwanPath=/root/work/gitrep/lwan
buildPath="$lwanPath/build"
proPath="$lwanPath/build/src/samples/ThreeKingdoms2DServer"
cd "$buildPath"
cmake ..
cd "$proPath"
make