#!/bin/bash

if [[ $(arch) == 'i386' ]]; then
  	echo Intel Mac
	CMD_DEF="-D USE_NCURSES=1"
#CMD_DEF="-D USE_PRINTF=1"
	DISP=display_ncurses.cpp
#DISP=display_printf.cpp
	IDIR="/usr/local/include"
	LDIR="/usr/local/lib"
	NCL="-lncurses"
elif [[ $(arch) == 'arm64' ]]; then
  	echo M1 Mac
	CMD_DEF="-D USE_NCURSES=1"
	DISP=display_ncurses.cpp
	IDIR="/opt/homebrew/include"
	LDIR="/opt/homebrew/lib"
	NCL="-lncurses"
else
	echo Win PC
	CMD_DEF="-D USE_PRINTF=1"
	DISP=display_printf.cpp
	IDIR="/mingw64/include"
	LDIR="/mingw64/lib"
	NCL=
fi
c++ -Wno-deprecated \
	$CMD_DEF \
	room_acoustics.cpp calc_images.cpp $DISP paUtils.c \
	-I$IDIR -L$LDIR -lsndfile -lportaudio $NCL
