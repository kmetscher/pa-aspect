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
## In progress
- Asynchronous API usage and recording capabilities for capturing output samples
- Implementation of the Fastest Fourier Transform in the West
## Planned
- Spectral visualization of discrete frequencies sorted into a handful of bins (determined on startup or in real time?)
- Real-time modification of frequency bands
- User selection of PulseAudio sinks (monitor specific applications/sinks instead of all sounds)
- User definition of visual appearance (amplitude gradients, bar width, etc)
## A note on style
The PulseAudio C API uses the snake_case convention. I am following the same convention within any C files that interface with the sound server for the sake of consistency. Elsewhere within the project, I will likely use camelCase out of habit.
## Other development notes
- Building on Ubuntu 20.04, Intel x86-64
- Non-standard includes: ncurses, pulse, fftw3 
- I'm tracking the compiled binary alongside source for analysis
## Links
- [FFTW](http://www.fftw.org/)
- [My website](https://kylemetscher.com)
- [My LinkedIn](https://linkedin.com/in/c0w80yd4n)
- [PulseAudio docs](https://freedesktop.org/software/pulseaudio/doxygen/index.html)
- [IPChicken](https://ipchicken.com)