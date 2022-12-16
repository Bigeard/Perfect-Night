# Perfect Night

RUN :

> make PLATFORM=PLATFORM_WEB  
> make PLATFORM=PLATFORM_WEB -B

INSTALL :

- https://emscripten.org/docs/getting_started/downloads.html  
- https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)

PACKAGE VSCODE :

- clangd
- Cmake
- Code Spell Checker
- GitLens
- Liver Server

COMPILE libxml2

mkdir -p ./build
mkdir -p ./libxml2/m4
cd ./libxml2
autoreconf -if -Wall
cd ../build
emconfigure ../libxml2/configure --with-http=no --with-ftp=no --with-python=no --with-threads=no --enable-shared=no
emmake make
cd ..
