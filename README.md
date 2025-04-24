# 2b

*by Benjamin Cook*\
<http://baryon.it>

<sup>pronounced "tube"</sup>

## Abstract

This is a suite of auxiliary programs for various audio apps of mine.

- [2n](https://github.com/Canar/2n).
- [prolaudi](https://github.com/Canar/prolaudi).

These programs read raw PCM from stdin and write to various audio devices. 

These are modeled after eg. `pacat`, `aplay`, `pw-cat`.

These programs may be of interest to users looking for a compact\
example as a tutorial for implementation of the various APIs. But\
note that I've removed all "unnecessary" code I can get away with.

Nothing here is well-tested at all, but everything "worked" at one point.

## Platforms

### Windows

The following Windows audio APIs have an implementation.

- MME / waveOut
- DirectSound
- WASAPI
- XAudio

Windows programs are built with MinGW.\
XAudio has a playback issue.\
There is a non-functional attempt at Kernel Streaming.\
Windows code was tested in Wine and MSYS2-MINGW64 on\
QEMU KVM Windows 11.

Windows platform defaults to 16-bit samples.

#### Windows `cat`

2b includes a minimal implementation of binary cat.

It's only really of interest if you're curious how to\
set up Windows systems to pipe binary, ie. not treat\
`0x1a` as EOF.

### Termux

Both Android audio APIs have an implementation under Termux.

- OpenSL ES
- AAudio

AAudio requires Android 9 aka O aka Oreo aka API 26.

Android programs has a primitive getopt implementation that is\
probably portable to other shims with little effort.

## Usage

To generate a file `tmp.raw` for testing using ffmpeg:

`ffmpeg -i audio.mp3 -ac 2 -ar 44100 -f s16le tmp.raw`

`tmp.raw` can then be piped into your choice of device.

There is a Makefile rule for `tmp.raw` that is configured\
to select a random track from an album on my dev box.

Termux: `cat tmp.raw | ./OpenSLES-write`

Windows `type tmp.raw | DirectSound-write.exe`

## Features and Configuration

These programs are very lightly tested.

Configuration is handled by editing the code.

## Release Log

0.0 - Initial release. `2024-07-15`\
Added some experimental outputs, readers, etc. `2025-04-10`\
hacked around on 2b's aaudio stuff until I figured out how to enable the aaudio pulseaudio sink in Termux lol `2025-04-14`\
AAudio still has merit because Termux's AAudio Pulseaudio sink isn't reliable. Added option handling to AAudio format. `2025-04-19`\
Refined OpenSL-ES code somewhat. `2025-04-23`
reamde, AAudio cleanup `2025-04-24`
