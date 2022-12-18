# install-crosscompiler.sh 17/12/2022 - 18/12/2022
# script for installing the i686 cross compiler for use in KOOLBOOT
# adapted from the COOLBOOT script to work better
# make sure you run as super-user or it will fail right at the end and it's so annoying
# it does take quite a bit so you might want some food or something while you wait
# Written By Gabriel Jickells

# install dependencies
sudo apt install build-essential
sudo apt install bison
sudo apt install flex
sudo apt install libgmp3-dev
sudo apt install libmpc-dev
sudo apt install libmpfr-dev
sudo apt install texinfo
sudo apt install libisl-dev

# set global variables
export PREFIX="/usr/local/i686-elf-gcc"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# make a temporary directory for the source files
mkdir ~/toolchain-src
cd ~/toolchain-src

# get the source files
wget "https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.gz"
wget "https://mirrorservice.org/sites/sourceware.org/pub/gcc/releases/gcc-11.3.0/gcc-11.3.0.tar.gz"

# make seperate build directories
mkdir binutils-build
mkdir gcc-build

# extract the source files
tar xvf binutils-2.38.tar.gz
tar xvf gcc-11.3.0.tar.gz

# build binutils
cd binutils-build
../binutils-2.38/configure --prefix=$PREFIX --target=$TARGET --with-sysroot --disable-nls --disable-werror
make
make install

# make gcc
cd ../gcc-build
../gcc-11.3.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

# clean up
cd ~
rm -rf ~/toolchain-src

# display the installed toolchain
ls $PREFIX/bin/