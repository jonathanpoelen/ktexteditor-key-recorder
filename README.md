# ktexteditor-key-recorder

Katepart plugin to record and replay a key sequence.

## Dependencies

Debian and derived:
 - kdelibs5-dev

Other distros:
 - kdelibs5-devel


## Install

```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$(kde4-config --localprefix) -DQT_QMAKE_EXECUTABLE=/usr/bin/qmake-qt4 -DCMAKE_BUILD_TYPE=Release
make
make install
```

or

```sh
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$(kde4-config --prefix) -DQT_QMAKE_EXECUTABLE=/usr/bin/qmake-qt4 -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```
