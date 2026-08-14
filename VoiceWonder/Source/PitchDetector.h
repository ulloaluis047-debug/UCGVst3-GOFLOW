#pragma once
#include <JuceHeader.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <vector>

class StablePitchDetector {
public:
    void prepare(double newSampleRate) {
        sampleRate = newSampleRate;
        ring.assign(windowSize, 0.0f);
        frame.assign(windowSize, 0.0f);
        writeIndex = 0; samplesSinceAnalysis = 0; filled = 0; currentHz = 0.0f;
        history.fill(0.0f); historyWrite = 0;
    }

    void reset() {
        std::fill(ring.begin(), ring.end(), 0.0f);
        writeIndex = samplesSinceAnalysis = filled = 0; currentHz = 0.0f;
        history.fill(0.0f); historyWrite = 0;
    }

    float push(const float* samples, int count) {
        if (ring.empty()) return 0.0f;
        for (int i = 0; i < count; ++i) {
            ring[(size_t)writeIndex] = samples[i];
            writeIndex = (writeIndex + 1) % windowSize;
            filled = juce::jmin(windowSize, filled + 1);
        }
        samplesSinceAnalysis += count;
        if (filled == windowSize && samplesSinceAnalysis >= hopSize) {
            samplesSinceAnalysis %= hopSize;
            for (int i = 0; i < windowSize; ++i)
                frame[(size_t)i] = ring[(size_t)((writeIndex + i) % windowSize)];
            const float detected = detectFrame(frame.data(), windowSize, sampleRate);
            if (detected > 0.0f) {
                history[(size_t)historyWrite] = detected;
                historyWrite = (historyWrite + 1) % (int)history.size();
                auto sorted = history;
                std::sort(sorted.begin(), sorted.end());
                const float median = sorted[sorted.size() / 2];
                if (median > 0.0f) currentHz = currentHz > 0.0f ? 0.72f * currentHz + 0.28f * median : median;
            }
        }
        return currentHz;
    }

    float getHz() const noexcept { return currentHz; }
    static float hzToMidi(float hz) noexcept { return hz > 0.0f ? 69.0f + 12.0f * std::log2(hz / 440.0f) : -1.0f; }
    static float midiToHz(float midi) noexcept { return 440.0f * std::pow(2.0f, (midi - 69.0f) / 12.0f); }

    static float detectFrame(const float* x, int n, double sr) {
        if (x == nullptr || n < 256) return 0.0f;
        double mean = 0.0, energy = 0.0;
        for (int i = 0; i < n; ++i) mean += x[i];
        mean /= n;
        for (int i = 0; i < n; ++i) { const double v = x[i] - mean; energy += v * v; }
        if (std::sqrt(energy / n) < 0.0025) return 0.0f;
        const int minLag = juce::jmax(2, (int)(sr / 1000.0));
        const int maxLag = juce::jmin(n / 2 - 2, (int)(sr / 60.0));
        float bestScore = 0.0f; int bestLag = 0;
        for (int lag = minLag; lag <= maxLag; ++lag) {
            double corr = 0.0, e1 = 1.0e-12, e2 = 1.0e-12;
            for (int i = 0; i < n - lag; ++i) {
                const double a = x[i] - mean, b = x[i + lag] - mean;
                corr += a * b; e1 += a * a; e2 += b * b;
            }
            const float score = (float)(corr / std::sqrt(e1 * e2));
            if (score > bestScore) { bestScore = score; bestLag = lag; }
        }
        if (bestScore < 0.62f || bestLag <= 0) return 0.0f;
        return (float)(sr / bestLag);
    }

private:
    static constexpr int windowSize = 2048, hopSize = 384;
    double sampleRate = 44100.0;
    std::vector<float> ring, frame;
    int writeIndex = 0, samplesSinceAnalysis = 0, filled = 0, historyWrite = 0;
    std::array<float, 5> history{};
    float currentHz = 0.0f;
};

