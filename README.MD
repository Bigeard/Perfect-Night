# Perfect Night

With the game Perfect Night you don't need to have multiple controllers lying around. All you need is your trusty smartphone 📱 and a QR code ! Simply scan the QR code on the main screen 📺 and watch as your phone transforms into a controller, giving you the chance to unleash your inner fighter and take down your opponents.  

Don’t wait grab your phone, call your friends, and get ready to rumble in the ultimate fighting showdown !  

Are you ready to become the champion of the ring? ⚔️. 

Click on this link to play ! 👾  
🔗 - https://bigeard.github.io/Perfect-Night 

## RUN / COMPILE THE GAME :

> make PLATFORM=PLATFORM_WEB -B
> make PLATFORM=PLATFORM_WEB BUILD_MODE=DEBUG -B

### TURN SERVER
> turnserver --log-file stdout

### PEERJS SERVER
> cd peerjs_server && npm start

### TRUN WIFI ON
> wihotspot

and use Chromium !


## INSTALL :

- https://emscripten.org/docs/getting_started/downloads.html  
- https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)

### PACKAGE VSCODE :

- C/C++ (Microsoft)
- Cmake
- Code Spell Checker
- GitLens
- Liver Server

### COMPILE `libxml2`

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

## CHECKLIST :

> Go to the project task list in `Projects` to see everything.

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
