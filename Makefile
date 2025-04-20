CC=gcc
PROJECT=2b
MINGW_CPP=x86_64-w64-mingw32-gcc
PREFIX ?= /usr/local/bin
SHELL ?= /bin/bash
FF=$(shell which ffmpeg || echo /usr/bin/does_not_exist )

#Windows/Wine
#API=DirectSound KernelStreaming XAudio2 MME WASAPI

#Termux
API=OPENSL-ES

PLATFORM=termux
#PLATFORM=linux

all: $(PLATFORM)

termux: OpenSL-ES AAudio

linux: wine

wine: $(API) Windows-binary-cat.exe

config.h:	config.mingw.h
	echo Defaulting to MinGW config. Symlink a different config for other platforms.
	#[ echo $$MSYS | egrep -i winsymlinks:[ln] ] || { echo Set winsymlinks in MSYS= ; exit 1 ; }
	ln -s config.mingw.h config.h

$(FF):	
	@echo Warning: FFMPEG executable not found. 2n will build but not function.

tmp.raw: $(FF)
	ffmpeg -hide_banner -loglevel 16 -i /storage/3531-6562/music/artists/prodigy/The\ Prodigy\ -\ 1994-07-04\ -\ Music\ for\ the\ Jilted\ Generation/$(shell printf '%02d' $$(( $$RANDOM % 13 + 1 )))*.m4a -ac 2 -ar 44100 -f s16le tmp.raw
	#ffmpeg -loglevel -8 -i /media/gondolin/audio/seedbox/2303/*Tunes\ 2*/$(shell printf '%02d' $$(( $$RANDOM % 17 + 1 )))*.flac -ac 2 -ar 44100 -f s16le tmp.raw

DirectSound: DirectSound-write.exe # DirectSound-ls.exe # bugged

KernelStreaming: KernelStreaming-write.exe

XAudio2: XAudio2-write.exe

MME: MME-write.exe MME-ls.exe

WASAPI: WASAPI-write.exe

OpenSL-ES: OpenSL-ES-write

AAudio: AAudio-write

MME-write.exe:	MME-write.c config.h
	$(MINGW_CPP) -o MME-write.exe MME-write.c -lwinmm

MME-ls.exe:	MME-ls.c config.h
	$(MINGW_CPP) -o MME-ls.exe MME-ls.c -lwinmm

DirectSound-write.exe: DirectSound-write.c
	$(MINGW_CPP) -o DirectSound-write.exe DirectSound-write.c -ldsound

DirectSound-ls.exe: DirectSound-ls.c
	$(MINGW_CPP) -o DirectSound-ls.exe DirectSound-ls.c -ldsound

WASAPI-write.exe: WASAPI-write.c
	$(MINGW_CPP) -o WASAPI-write.exe WASAPI-write.c -lole32 -luuid -lksuser -lavrt

XAudio2-write.exe: XAudio2-write.c
	$(MINGW_CPP) -o XAudio2-write.exe XAudio2-write.c -lole32 -lxaudio2_8

KernelStreaming-write.exe: KernelStreaming-write.c
	$(MINGW_CPP) -o KernelStreaming-write.exe KernelStreaming-write.c -lsetupapi -lksuser

Windows-binary-cat.exe: Windows-binary-cat.c
	$(MINGW_CPP) -o Windows-binary-cat.exe Windows-binary-cat.c

OpenSL-ES-write: OpenSL-ES-write.c
	clang -o OpenSL-ES-write OpenSL-ES-write.c -lOpenSLES

AAudio-write: AAudio-write.c args.c
	clang -o AAudio-write AAudio-write.c -laaudio --target=aarch64-unknown-linux-android26

.PHONY: install debug tcc waveout DSound-write.c.bak
