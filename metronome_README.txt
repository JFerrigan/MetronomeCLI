MACOS COMMAND-LINE METRONOME
===========================

Zero external libraries. Uses Apple's native AudioToolbox to synthesize clicks.

Compile:
  clang++ -std=c++17 -O2 metronome.cpp -framework AudioToolbox -o metronome

Run:
  ./metronome 60
  ./metronome 80 4 2
  ./metronome 100 4 4

Arguments:
  1. Tempo: 30-300 BPM (default: 80)
  2. Beats per bar: 1-12 (default: 4)
  3. Subdivisions: 1, 2, or 4 clicks per beat (default: 1)

The downbeat of each bar sounds higher than the other beats.
Subdivisions are played more quietly than the main beats.
During playback in a terminal:
  Space: tap at least twice to set tempo (averages up to 4 intervals).
  Up/Down: increase/decrease tempo by 1 BPM.
  Right/Left: increase/decrease tempo by 10 BPM.
  Ctrl+C: stop playback.
Tempo stays within 30-300 BPM. A pause over 2.5 seconds or an arrow
adjustment resets the tap sequence. The live display shows the current BPM.

If clang++ isn't available, run: xcode-select --install
If you don't hear sound, check the Mac's selected output device and volume.
