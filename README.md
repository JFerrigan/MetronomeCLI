# mac_metronome

A small command-line metronome for macOS, written in C++17. It synthesizes clicks with Apple's native AudioToolbox framework, with no external libraries or sound files.

## Features

- Adjustable tempo from 30 to 300 BPM.
- From 1 to 12 beats per bar.
- Choose 1, 2, or 4 clicks per beat.
- A higher-pitched accent on the first beat of each bar.
- Quieter clicks for subdivisions.
- Live beat and subdivision display in the terminal.

## Requirements

- macOS.
- Apple's Command Line Tools for Xcode, including `clang++`.

If you need the command-line tools, install them with:

```sh
xcode-select --install
```

## Build

From the project directory, run:

```sh
./build.sh
```

This creates the `metronome` executable in the project directory. You can also compile it directly:

```sh
clang++ -std=c++17 -O2 metronome.cpp -framework AudioToolbox -o metronome
```

## Usage

```sh
./metronome [BPM] [beats_per_bar] [subdivision]
```

All arguments are optional and positional. Omitted arguments use their defaults.

| Argument | Accepted values | Default |
| --- | --- | --- |
| `BPM` | 30–300 | 80 |
| `beats_per_bar` | 1–12 | 4 |
| `subdivision` | 1, 2, or 4 clicks per beat | 1 |

### Examples

```sh
# Start with the defaults: 80 BPM, 4 beats per bar, 1 click per beat
./metronome

# Quarter notes at 60 BPM
./metronome 60

# Eighth notes at 80 BPM in 4/4
./metronome 80 4 2

# Sixteenth notes at 100 BPM in 4/4
./metronome 100 4 4

# Three beats per bar at 120 BPM
./metronome 120 3
```

Press **Ctrl+C** to stop playback. Show the built-in help with:

```sh
./metronome --help
```

## Troubleshooting

- **`clang++` is unavailable:** install the Command Line Tools with `xcode-select --install`.
- **No sound:** check your Mac's selected audio output device and volume.
- **Invalid arguments:** use whole numbers within the ranges listed above. Subdivision must be `1`, `2`, or `4`.
