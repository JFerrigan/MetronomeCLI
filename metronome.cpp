// Minimal command-line metronome for macOS.
// Build: clang++ -std=c++17 -O2 metronome.cpp -framework AudioToolbox -o metronome
// Usage: ./metronome [bpm] [beats_per_bar] [subdivision]
// Example: ./metronome 80 4 2

#include <AudioToolbox/AudioToolbox.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {
constexpr double kSampleRate = 44100.0;
constexpr double kPi = 3.14159265358979323846;
constexpr unsigned kFramesPerBuffer = 512;
constexpr unsigned kBufferCount = 4;
constexpr double kClickSeconds = 0.028;

volatile std::sig_atomic_t gStopRequested = 0;

void onSignal(int) {
    gStopRequested = 1;
}

struct Metronome {
    int bpm = 80;
    int beatsPerBar = 4;
    int subdivision = 1;
    double samplesPerTick = 0.0;
    std::uint64_t nextFrame = 0;  // Accessed only from Audio Queue callback thread.
    std::atomic<std::uint64_t> latestTick{0};
    std::atomic<bool> audioError{false};
};

void fillBuffer(void* userData, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    auto& m = *static_cast<Metronome*>(userData);
    auto* output = static_cast<std::int16_t*>(buffer->mAudioData);
    std::uint64_t lastTick = 0;

    for (unsigned i = 0; i < kFramesPerBuffer; ++i) {
        const double frame = static_cast<double>(m.nextFrame++);
        const auto tick = static_cast<std::uint64_t>(frame / m.samplesPerTick);
        const double phase = frame - static_cast<double>(tick) * m.samplesPerTick;
        lastTick = tick;

        double sample = 0.0;
        if (phase >= 0.0 && phase < kClickSeconds * kSampleRate) {
            const bool isBeat = (tick % m.subdivision) == 0;
            const bool isDownbeat = isBeat &&
                ((tick / m.subdivision) % m.beatsPerBar) == 0;
            const double pitch = isDownbeat ? 1700.0 : (isBeat ? 1250.0 : 850.0);
            const double volume = isDownbeat ? 0.86 : (isBeat ? 0.64 : 0.36);
            const double t = phase / kSampleRate;
            // Short, decaying synthesized click with no sound file or dependencies.
            sample = volume * std::exp(-115.0 * t) *
                     std::sin(2.0 * kPi * pitch * t);
        }
        output[i] = static_cast<std::int16_t>(sample * 32767.0);
    }

    m.latestTick.store(lastTick, std::memory_order_relaxed);
    buffer->mAudioDataByteSize = kFramesPerBuffer * sizeof(std::int16_t);
    if (AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr) != noErr) {
        m.audioError.store(true, std::memory_order_relaxed);
    }
}

bool parseInt(const char* text, int& out) {
    try {
        const std::string value(text);
        std::size_t used = 0;
        int parsed = std::stoi(value, &used);
        if (used != value.size()) return false;
        out = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

void usage(const char* executable) {
    std::cout << "Usage: " << executable << " [BPM] [beats/bar] [subdivision]\n"
              << "  BPM:         30-300 (default 80)\n"
              << "  beats/bar:   1-12  (default 4)\n"
              << "  subdivision: 1, 2, or 4 clicks/beat (default 1)\n\n"
              << "Examples:\n"
              << "  " << executable << " 60       # Quarter notes at 60 BPM\n"
              << "  " << executable << " 80 4 2   # Eighth notes at 80 BPM\n"
              << "  " << executable << " 100 4 4  # Sixteenth notes at 100 BPM\n\n"
              << "Press Ctrl+C to stop.\n";
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "--help" ||
                     std::string(argv[1]) == "-h")) {
        usage(argv[0]);
        return 0;
    }
    if (argc > 4) {
        usage(argv[0]);
        return 1;
    }

    Metronome m;
    if ((argc > 1 && !parseInt(argv[1], m.bpm)) ||
        (argc > 2 && !parseInt(argv[2], m.beatsPerBar)) ||
        (argc > 3 && !parseInt(argv[3], m.subdivision)) ||
        m.bpm < 30 || m.bpm > 300 ||
        m.beatsPerBar < 1 || m.beatsPerBar > 12 ||
        (m.subdivision != 1 && m.subdivision != 2 && m.subdivision != 4)) {
        std::cerr << "Invalid arguments.\n\n";
        usage(argv[0]);
        return 1;
    }

    m.samplesPerTick = kSampleRate * 60.0 / (m.bpm * m.subdivision);

    AudioStreamBasicDescription format{};
    format.mSampleRate = kSampleRate;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger |
                          kLinearPCMFormatFlagIsPacked;
    format.mBytesPerPacket = sizeof(std::int16_t);
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = sizeof(std::int16_t);
    format.mChannelsPerFrame = 1;
    format.mBitsPerChannel = 16;

    AudioQueueRef queue = nullptr;
    OSStatus status = AudioQueueNewOutput(&format, fillBuffer, &m, nullptr,
                                         nullptr, 0, &queue);
    if (status != noErr) {
        std::cerr << "Could not open the audio output (Core Audio error "
                  << status << ").\n";
        return 1;
    }

    for (unsigned i = 0; i < kBufferCount; ++i) {
        AudioQueueBufferRef buffer = nullptr;
        status = AudioQueueAllocateBuffer(queue, kFramesPerBuffer * sizeof(std::int16_t),
                                          &buffer);
        if (status != noErr) break;
        fillBuffer(&m, queue, buffer);
        if (m.audioError.load()) {
            status = -1;
            break;
        }
    }
    if (status == noErr) status = AudioQueueStart(queue, nullptr);
    if (status != noErr) {
        std::cerr << "Could not start audio (Core Audio error " << status << ").\n";
        AudioQueueDispose(queue, true);
        return 1;
    }

    std::signal(SIGINT, onSignal);
    std::cout << m.bpm << " BPM | " << m.beatsPerBar << "/4 | "
              << m.subdivision << " click(s)/beat | Ctrl+C to stop\n";

    std::uint64_t previousTick = UINT64_MAX;
    while (!gStopRequested && !m.audioError.load()) {
        const auto tick = m.latestTick.load(std::memory_order_relaxed);
        if (tick != previousTick) {
            const auto beat = (tick / m.subdivision) % m.beatsPerBar + 1;
            const auto part = tick % m.subdivision + 1;
            std::cout << "\rBeat " << beat << "/" << m.beatsPerBar;
            if (m.subdivision > 1) {
                std::cout << "  subdivision " << part << "/" << m.subdivision;
            }
            std::cout << "     " << std::flush;
            previousTick = tick;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    AudioQueueStop(queue, true);
    AudioQueueDispose(queue, true);
    if (m.audioError.load()) {
        std::cerr << "\nAudio output error.\n";
        return 1;
    }
    std::cout << "\nStopped.\n";
    return 0;
}
