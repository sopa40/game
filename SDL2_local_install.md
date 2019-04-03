# SDL2 lokal installieren (UNIX/Linux)


## Code runterladen 

Mit git die entsprechenden Repositories runterladen (hier SDL und SDL\_image):
```
$ git clone -b release-2.0.9 https://github.com/SDL-mirror/SDL.git
$ git clone -b release-2.0.4 https://github.com/SDL-mirror/SDL_image.git
```
alternativ: Download über [SDL2](https://www.libsdl.org/download-2.0.php) bzw.
[SDL2 Image](https://www.libsdl.org/projects/SDL_image/)


## Kompilieren

Jeweils in den Ordner gehen und bauen, bspw.:
```
$ cd SDL
$ ./configure --prefix=${HOME}/.local   # Konfigurieren des Installationsortes
$ make -j$(nproc)                       # Kompilieren
$ mkdir -p ${HOME}/.local               # Erstellen des Ordners
$ make install                          # Installieren (kopieren der Dateien)
```

## Einbindung ins Build

Wir setzen im Makefile eine Variable `SDL2` die den Installationsort beinhaltet
(`--prefix` von oben):
```
SDL2 ?= $(HOME)/.local
```

Wir müssen nun den Include-Pfad hinzufügen und nutzen hierbei unsere Variable:
```
CPPFLAGS=... -isystem$(SDL2)/include ..
```

Genauso müssen wir den Link-Pfad für die Bibliothek hinzufügen, und zwar zwei
Mail;  einmal für das Linken bei der Kompilation, einmal für das dynamische
Linken zur Laufzeit:
```
LDFLAGS=-L$(SDL2)/lib -Wl,-rpath,$(SDL2)/lib
```

## Weiterführende Links

https://wiki.libsdl.org/Installation
