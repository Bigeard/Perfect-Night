<p align="center">
  <img src="https://github.com/Bigeard/Perfect-Night/blob/main/design/media/title_perfect_night.png?raw=true" alt="Image Title: Perfect Night"/>
</p>

# Perfect Night

With the game Perfect Night you don't need to have multiple controllers lying around. All you need is your trusty smartphone 📱 and a QR code ! Simply scan the QR code on the main screen 📺 and watch as your phone transforms into a controller, giving you the chance to unleash your inner fighter and take down your opponents.  

Don’t wait grab your phone, call your friends, and get ready to rumble in the ultimate fighting showdown !  

Are you ready to become the champion of the ring? ⚔️. 

Click on this link to play ! 👾  
🔗 - https://bigeard.github.io/Perfect-Night

<br/>  

[Link of the Discord 💬](https://discord.com/channels/1074855981508472923/1074855981986619474)

<p align="center">
  <img src="https://github.com/Bigeard/Perfect-Night/blob/main/design/media/2023_02_24.png?raw=true" alt="Image Gameplay"/>
</p>


## RUN / COMPILE THE GAME :

> cd dev && make

### TURN SERVER

> turnserver --log-file stdout
> turnserver -u perfect:perfect --log-file stdout

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

- (optional) customization of the player (color, appearance)
- Add the player's reaction (happy, sad, angry)
- Add different types of weapons (proximity, explosion)
- Create new maps
- A custom map loader/upload
