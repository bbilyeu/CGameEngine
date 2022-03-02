# CGameEngine

Built between July 2016 and July 2020 to expand my understanding of C++. 

The shared "library" used by both the game's Server, Client, and AdminClient applications is intentionally separate.

## CentOS 7 setup notes
```sh
[root@desktop ~]# yum install SDL2_ttf* SDL2_mixer* SDL2 glm-devel mysql++-devel glew-devel glfw-devel mesa-libGLES-devel openssl-devel gmake cmake devtoolset-8

[root@desktop ~]# scl enable devtoolset-8 -- bash
[root@desktop ~]#source scl_source enable devtoolset-8

# (Download CodeBlocks 17, extract)
[root@desktop ~]#yum --nogpgcheck localinstall codeblocks-17.12-1.el7.centos.x86_64.rpm 
```

## CentOS 8 setup notes
```sh
## install base packages
yum install https://dl.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
dnf config-manager --set-enabled PowerTools
yum install glm-devel glew-devel openssl-devel cmake boost-devel freeglut-devel mpg123-devel libvorbis-devel flac-devel fluidsynth-devel libmodplug-devel opusfile-devel sqlite-devel libstdc++-static glibc-static

## compile SDL2
wget https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
tar -xvf SDL2-2.0.12.tar.gz
cd SDL2-2.0.12/
../configure --enable-static
make
make install

## compile SDL_ttf
wget https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.15.tar.gz
tar xvf SDL2_ttf-2.0.15.tar.gz
cd SDL2_ttf-2.0.15/
./configure
make
make install
cp SDL_ttf.h /usr/include/SDL2/.

## compile SDL_mixer
wget https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4.tar.gz
tar xvf SDL2_mixer-2.0.4.tar.gz 
cd SDL2_mixer-2.0.4/
./configure --enable-static
make
make install
cp SDL_mixer.h /usr/include/SDL2/.


## compile mysql c++ connector (8.0)
git clone https://github.com/mysql/mysql-connector-cpp.git
cd mysql-connector-cpp/
git checkout 8.0
cmake . -DBUILD_STATIC=ON
cmake --build .
cmake --build . -target install
ln -s /usr/local/mysql/connector-c++-/include/mysqlx /usr/include/mysqlx
ln -s /usr/local/mysql/connector-c++-/lib64/debug/libmysqlcppconn8-static.a /usr/lib64/libmysqlcppconn8-static.a 


## compile GLEW
git clone git@github.com:nigels-com/glew.git -b glew-2.0.0
cd glew-2.0.0/build
cmake ./cmake
make -j4 glew_s
cp lib/libGLEW.a /usr/local/lib/.


## compile OpenSSL 1.1.1
git clone https://github.com/openssl/openssl.git  -b OpenSSL_1_1_1-stable
cd openssl
./config
make
make install


## compile wxGTK (codeblocks dependency)
wget https://github.com/wxWidgets/wxWidgets/releases/download/v2.8.12/wxGTK-2.8.12.tar.gz
tar xvf wxGTK-2.8.12.tar.gz
cd wxGTK-2.8.12
mkdir build_gtk2_shared_monolithic_unicode
cd build_gtk2_shared_monolithic_unicode
export CFLAGS=-std=c99 CXXFLAGS=-std=c++98
../configure --prefix=/opt/wx/2.8 --enable-xrc --enable-monolithic --enable-unicode
make
make install
echo ‘/opt/wx/2.8/lib’ > /etc/ld.so.conf.d/lib_gtk2u-2.8.so.0
ldconfig
echo ‘export PATH=/usr/bin:/opt/wx/2.8/bin:$PATH’ >> ~/.bash_profile
source ~/.bash_profile
unset CFLAGS
unset CXXFLAGS

## compile codeblocks
<download Codeblocks source: http://www.codeblocks.org/downloads/source>
tar xvf codeblocks-20.03.tar.xz
cd codeblocks-20.03
export ACLOCAL_FLAGS="-I `wx-config --prefix`/share/aclocal"
./bootstrap 
./configure --with-contrib-plugins=all --with-boost-libdir=/usr/lib64
make
make install
```