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

## Licence

Unless otherwise noted, the code of this project is available under GPLv3 or later.
See the standard licence header:

> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
>
> This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
>
> You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

While I would rather use a licence that is better compatible with European law (EUPL),
Qt and KDE libraries force the use of the GPL, so here we are.