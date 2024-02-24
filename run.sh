lwanPath=/root/work/gitrep/lwan
tkPath="$lwanPath/build/src/samples/ThreeKingdoms2DServer/tk"
proPath="$lwanPath/build/src/samples/ThreeKingdoms2DServer"
tkConfPath="$proPath/tk.conf"
"$tkPath" -c "$tkConfPath"

#lwan_thread_init