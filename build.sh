#!/bin/sh
set -eu

cd "$(dirname "$0")"
clang++ -std=c++17 -O2 metronome.cpp -framework AudioToolbox -o metronome
