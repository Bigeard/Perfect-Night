# Perfect Night

RUN / COMPILE THE GAME :

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

```shell
mkdir -p ./build
mkdir -p ./libxml2/m4
cd ./libxml2
autoreconf -if -Wall
cd ../build
emconfigure ../libxml2/configure --with-http=no --with-ftp=no --with-python=no --with-threads=no --enable-shared=no
emmake make
cd ..
```

IDEA :

- Change color of joystick according to the player's color
- Fix joystick detector 
- Fix bug of collision
- Improve the design
    - Add design for the collision boxes
    - Change graphic of the player info (life / amm.)
    - (optional) customisation of the player (color, appearance)
    - Change UI/UX of the phone gamepad (add info game play / p2p button)
    - Create the design of the pages
    - Design the background limit
- Add the pages
    - title page (menu)
    - Games Page
    - Versus Page (select player)
    - Map Page
    - Game Page
- Review QrCode generation (imp. in clang)
- Create new maps
- A custom map loader/upload

