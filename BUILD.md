# Building Instructions:

This guide is optimized for centos7 (you can adapt this for your OS), and the generated binary should be compatible with other neewer distributions...

First, as prerequisite, you must have installed libMantids (as static libs, and better if MinSizeRel)

You also need to have openssl compiled in static mode in /opt/osslibs, and better not to have `openssl-devel`

then, you can proceed to build this:

```
git clone https://github.com/unmanarc/uEtherDwarf
cd uEtherDwarf
cmake3 . -DPORTABLE=ON -DCMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_INSTALL_PREFIX:PATH=/opt/osslibs -DEXTRAPREFIX:PATH=/opt/osslibs -B../uEtherDwarf-build
cd ../uEtherDwarf-build
make -j8 install
```

And if you want to reduce the binary size:

```
strip -x -X -s /opt/osslibs/bin/uEtherDwarf
upx -9 --ultra-brute /opt/osslibs/bin/uEtherDwarf
```
