TileWorld-for-Handhelds
=======================

A port of Tile World to handheld consoles such as the Anbernic RG351P, RG351MP, RG503, RG353P, and others.

This modified version for the Anbernic handhelds will require sdl12-compat which can be found at https://github.com/libsdl-org/sdl12-compat.

This version has been modified to use the higher resolution spritesheets for improved graphical detail.

For a real treat you can also compile this and sdl12-compat on a normal (x86_64) PC running Linux which means you could run Tile World fullscreen with a larger display!

To compile for Linux Ubuntu/Debian derivitives make sure you have the necessary dependencies installed:
sudo apt install libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libpng-dev libpng16-16 libjpeg-dev libtiff-dev libtiff5-dev libmikmod-dev libfluidsynth-dev libvorbisfile3 flac libmad-ocaml-dev libasound2-dev liblzma-dev zlib1g-dev libjbig-dev libsdl2-dev libopenal-dev libglib2.0-dev libjack-dev libsndfile1-dev libreadline-dev libvorbis-dev libogg-dev libvorbisenc2 libslang2-dev libtinfo-dev libsndio-dev libxinerama-dev libxrandr-dev libxss-dev libwrap0-dev libxrender-dev liblz4-dev libffi-dev libgpg-error-dev

Then build with:
cd src
make

If you have issues building because your distro only provides a newer version of libpng, simply install that version, edit the Makefile (check perms), change the png version from 16 to your version, save changes to the Makefile and build again.

To run fullscreen after building both tileworld-for-handhelds and sdl12-compat:
LD_LIBRARY_PATH=/path/to/sdl12-compat ./tworld -F