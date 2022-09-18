# PulseAudio Aspect
`                            _   
                           | |  
   __ _ ___ _ __   ___  ___| |_ 
  / _` / __| '_ \ / _ \/ __| __|
 | (_| \__ \ |_) |  __/ (__| |_ 
  \__,_|___/ .__/ \___|\___|\__|
           | |                  
           |_|                  `
## About
Aspect is a terminal-based spectral visualizer written in C. This version makes use of the PulseAudio sound server development API.
## In progress
- Fast Fourier transform using modified Cooley-Tukey algorithm
- Fiddling with PA API to determine what the hell is actually coming down the pipe
- Drawing frequency band amplitudes with the graphics library
## Planned
- Spectral visualization of discrete frequencies sorted into a handful of bins (determined on startup or in real time?)
- Real-time modification of frequency bands
- User selection of PulseAudio sinks (monitor specific applications instead of all sounds)
- User definition of visual appearance (amplitude gradients, bar width, etc)
## Links
- [My website](https://kylemetscher.com)
- [My LinkedIn](https://linkedin.com/in/c0w80yd4n)
- [IPChicken](https://ipchicken.com)