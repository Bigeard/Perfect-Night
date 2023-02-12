# Perfect Night

RUN / COMPILE THE GAME :

> make PLATFORM=PLATFORM_WEB -B
> make PLATFORM=PLATFORM_WEB BUILD_MODE=DEBUG -B

TURN SERVER
> turnserver --log-file stdout

PeerJS SERVER
> cd server && npm start

TRUN WIFI ON
> wihotspot

and use chromium !


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

CHECKLIST :

- Fix bug of collision (need review)
- Improve the design
    - Add design for the collision boxes
    - (optional) customization of the player (color, appearance)
    - Change UI/UX of the phone gamepad (add info game play / p2p button)
    - Create the design of the pages
    - Design the background limit
    - Add the player's reaction (happy, sad, angry)
- Add the pages
    - title page (menu)
    - Games Page
    - Versus Page (select player)
    - Map Page
    - Game Page
- Add different types of weapons (proximity, explosion)
- Create new maps
- A custom map loader/upload
