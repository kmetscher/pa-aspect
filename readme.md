# PulseAudio Aspect
```                     _   
                           | |  
   __ _ ___ _ __   ___  ___| |_ 
  / _` / __| '_ \ / _ \/ __| __|
 | (_| \__ \ |_) |  __/ (__| |_ 
  \__,_|___/ .__/ \___|\___|\__|
           | |                  
           |_|                  
```
## About
Aspect is a terminal-based spectral visualizer written in C. This version makes use of the PulseAudio sound server development API.
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
## Links
- [FFTW](http://www.fftw.org/)
- [My website](https://kylemetscher.com)
- [My LinkedIn](https://linkedin.com/in/c0w80yd4n)
- [IPChicken](https://ipchicken.com)