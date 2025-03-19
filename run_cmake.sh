#!/bin/sh

if [ ! -f source/hal/driver-ST7735S/source/st7735s.c ]
then
	echo "source/hal/driver-ST7735S/source/st7735s.c doesn't seem to exist." 1>&2
	echo "Did you forget to do 'git submodule update --init --recursive'?" 1>&2
	exit 1
fi

cmake -S . -B build/ -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=1

