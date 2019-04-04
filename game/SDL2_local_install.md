# SDL2 local install (UNIX/Linux)


## Code download

Download ( SDL and SDL\_image):
```
$ git clone -b release-2.0.9 https://github.com/SDL-mirror/SDL.git
$ git clone -b release-2.0.4 https://github.com/SDL-mirror/SDL_image.git
```
alternativ: Download via [SDL2](https://www.libsdl.org/download-2.0.php) bzw.
[SDL2 Image](https://www.libsdl.org/projects/SDL_image/)


## Compile

Move in the folder, for example.:
```
$ cd SDL
$ ./configure --prefix=${HOME}/.local   # Configure
$ make -j$(nproc)                       # Compile
$ mkdir -p ${HOME}/.local               # Creation of folder
$ make install                          # Install (Copy of Data)
```


## More info
https://wiki.libsdl.org/Installation
