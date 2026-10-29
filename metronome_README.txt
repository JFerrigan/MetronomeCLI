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
Ctrl+C stops playback.

If clang++ isn't available, run: xcode-select --install
If you don't hear sound, check the Mac's selected output device and volume.
