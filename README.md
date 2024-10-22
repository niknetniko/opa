# Opa

A genealogy program written in Qt (and using KDE frameworks). Or at least, it will be at some point.

> [!CAUTION]
> This is very early software and WIP. Do not expect anything to work, and certainly do not rely on the program for your data.
> It might delete everything at any point without undo.
>
> Anything might break at any point!

To build it, use either the included Nix flake, or run:

```console
$ cd opa
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
$ make install
# or  su -c 'make install'  or  sudo make install
```

where `$KDEDIRS` points to your KDE installation prefix.
