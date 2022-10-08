# PulseAudio Aspect
```                         _   
                           | |  
   __ _ ___ _ __   ___  ___| |_ 
  / _` / __| '_ \ / _ \/ __| __|
 | (_| \__ \ |_) |  __/ (__| |_ 
  \__,_|___/ .__/ \___|\___|\__|
           | |                  
           |_|                  
```
## About
Aspect is a terminal-based spectral visualizer written in C. It applies the Discrete Fourier Transform (DFT) to a series of audio samples captured from a playback sink to visualize frequency ranges in near-real time. The DFT is calculated using a Fast Fourier Transform (FFT) algorithm implemented with the FFTW library. 

This version makes use of the PulseAudio sound server development API and is intended for use on those POSIX machines running PulseAudio (hence pa-aspect). I intend to build versions of the visualizer that work with other sound servers/implementations in the future, such as with PipeWire and ALSA.

pa-aspect is in a FUNCTIONAL, but not COMPLETE state. You can build and run it on your machine and watch some pound signs dance to the beat of your music. I intend to build this application into an aesthetically compelling and performant state. While pa-aspect is still in development, I also cannot make any promises about its memory and thread safety. State and threads are managed -- mostly -- in the robust PulseAudio development API callback system and the ncurses library, but the possibility of leakage or races can't be ignored through my own or external fault.
## Installation from source
- Use your favorite C compiler, like GCC
- Compile time dependencies: complex, fftw3, math, ncurses, pulse; check your distribution's package repositories or the links below
- Runtime dependencies: suitable terminal emulator environment, a working PulseAudio server with at least one sink monitor
- Link with: fftw3, m, menu, ncurses, pulse
- Run the binary from your working directory
- Play some tunes!
## Recent changes
- User prompted at startup to select a sink monitor for analysis
- Licensing and copyleft
## In progress
- Determining how to reduce client-side latency in retrieving samples. The server only reads data out when the read callback recognizes there's data to be read, and a DFT requires a non-negligible amount of samples in its input buffer to be both accurate and also spit out remotely interesting data. Presently the delay in acquiring enough samples for a compelling frequency bin set (>64) is taking too long
- "Graphical" representation of frequency amplitudes with color and ASCII art in curses windows
## Planned
- User definition of visual appearance (amplitude gradients, bar width, etc)
## Possibilities
- Wrapping in an external shell application to make drawing easier; possibly "smooth" rendering out to make the delay in buffer retrieval less obvious?
- Asynchronous math/display operations to allow the buffer to keep filling while the overhead of the DFT (negligible) and curses (significant) blocks. Make display operations non-blocking?
## A note on style
The PulseAudio C API uses the snake_case convention. I am following the same convention within any C files that interface with the sound server for the sake of consistency. Elsewhere within the project, I will likely use camelCase out of habit.
## Other development notes
- Building on Ubuntu 20.04, Intel x86-64
- Non-standard includes: ncurses, pulse, fftw3 
- I'm tracking the compiled binary alongside source for analysis
## License
PulseAudio Aspect is licensed under the terms of the GNU GPL version 3 or later.
## Links
- [FFTW](http://www.fftw.org/)
- [My website](https://kylemetscher.com)
- [My LinkedIn](https://linkedin.com/in/c0w80yd4n)
- [PulseAudio docs](https://freedesktop.org/software/pulseaudio/doxygen/index.html)
- [IPChicken](https://ipchicken.com)