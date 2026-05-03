# Musl cross-compiler and FFmpeg setup

## muslc libc setup

In order to cross-compile with musl libc, you need to install the necessary compiler:
```bash
wget https://musl.cc/arm-linux-musleabihf-cross.tgz
tar -xf arm-linux-musleabihf-cross.tgz
```

If you want it to be available globally, add its path to the `$PATH`:
```bash
export PATH="$PWD/arm-linux-musleabihf-cross/bin:$PATH"
```

You can verify installation with
```bash
arm-linux-musleabihf-gcc --version
```

## FFmpeg libraries setup

Firstly, you need to get the FFmpeg repository:

```bash
git clone https://git.ffmpeg.org/ffmpeg.git
cd ffmpeg
```

Then you need to configure and build it:
```bash
./configure \
    --arch=arm \
    --target-os=linux \
    --cross-prefix=${CROSS}- \
    --enable-cross-compile \
    --prefix=$PREFIX \
    --extra-cflags="-I$PREFIX/include" \
    --extra-ldflags="-L$PREFIX/lib" \
    --enable-static \
    --disable-shared

make -j$(nproc)
make install
```

## Compiling C program to ARM executable

With everything installed, you can compile program like:

```bash
arm-linux-musleabihf-gcc \
    -o fbplayer main.c \
    -I$PREFIX/include \
    -L$PREFIX/lib \
    -static \
    -Wl,--start-group \
    -lavformat -lavcodec -lswscale -lswresample -lavutil \
    -latomic \
    -lm -lpthread \
    -Wl,--end-group
```

Best way to test how program works is by copying it to VM using SSH:
```bash
scp -P <port> <program_name> root@localhost:/root/
```
(also copy some video files).
