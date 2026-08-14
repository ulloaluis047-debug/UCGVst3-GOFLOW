#include "ReferenceAnalyzer.h"
#include "PitchDetector.h"
#include <algorithm>
#include <numeric>

juce::var VocalReferenceProfile::toVar() const {
    auto* o = new juce::DynamicObject();
    o->setProperty("name", name); o->setProperty("valid", valid);
    o->setProperty("meanPitchHz", meanPitchHz); o->setProperty("pitchLowHz", pitchLowHz); o->setProperty("pitchHighHz", pitchHighHz);
    o->setProperty("rmsDb", rmsDb); o->setProperty("crestDb", crestDb); o->setProperty("spectralCentroidHz", spectralCentroidHz);
    o->setProperty("warmthDb", warmthDb); o->setProperty("presenceDb", presenceDb); o->setProperty("airDb", airDb);
    o->setProperty("formantSemitones", formantSemitones); o->setProperty("compression", compression); o->setProperty("deEss", deEss);
    return juce::var(o);
}

VocalReferenceProfile VocalReferenceProfile::fromVar(const juce::var& value) {
    VocalReferenceProfile p; auto* o = value.getDynamicObject(); if (o == nullptr) return p;
    auto number = [o](const char* id, float fallback) { const auto v = o->getProperty(id); return v.isVoid() ? fallback : (float)v; };
    p.name = o->getProperty("name").toString(); p.valid = (bool)o->getProperty("valid");
    p.meanPitchHz = number("meanPitchHz", 0); p.pitchLowHz = number("pitchLowHz", 0); p.pitchHighHz = number("pitchHighHz", 0);
    p.rmsDb = number("rmsDb", -60); p.crestDb = number("crestDb", 0); p.spectralCentroidHz = number("spectralCentroidHz", 0);
    p.warmthDb = number("warmthDb", 0); p.presenceDb = number("presenceDb", 0); p.airDb = number("airDb", 0);
    p.formantSemitones = number("formantSemitones", 0); p.compression = number("compression", .35f); p.deEss = number("deEss", .25f);
    return p;
}

juce::String VocalReferenceProfile::summary() const {
    if (!valid) return "No se pudo detectar una voz utilizable";
    return name + " | tono " + juce::String(meanPitchHz, 1) + " Hz | brillo " + juce::String(spectralCentroidHz, 0)
        + " Hz | dinámica " + juce::String(crestDb, 1) + " dB";
}

VocalReferenceProfile ReferenceAnalyzer::analyse(const juce::File& audioFile) {
    VocalReferenceProfile p; p.name = audioFile.getFileNameWithoutExtension();
    juce::AudioFormatManager formats; formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(audioFile));
    if (reader == nullptr || reader->lengthInSamples < 4096) return p;

    const auto maxSamples = (juce::int64)(reader->sampleRate * 120.0);
    const int count = (int)juce::jmin(reader->lengthInSamples, maxSamples);
    juce::AudioBuffer<float> source((int)juce::jmin((unsigned int)2, reader->numChannels), count);
    if (!reader->read(&source, 0, count, 0, true, true)) return p;
    std::vector<float> mono((size_t)count, 0.0f);
    for (int c = 0; c < source.getNumChannels(); ++c)
        for (int i = 0; i < count; ++i) mono[(size_t)i] += source.getSample(c, i) / source.getNumChannels();

    double sumSq = 0.0; float peak = 0.0f;
    for (float x : mono) { sumSq += x * x; peak = juce::jmax(peak, std::abs(x)); }
    const float rms = std::sqrt((float)(sumSq / juce::jmax(1, count)));
    p.rmsDb = juce::Decibels::gainToDecibels(rms, -100.0f);
    p.crestDb = juce::Decibels::gainToDecibels(peak / juce::jmax(rms, 1.0e-6f), 0.0f);

    constexpr int order = 11, fftSize = 1 << order, hop = 1024;
    juce::dsp::FFT fft(order); juce::dsp::WindowingFunction<float> window(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
    std::vector<float> fftData((size_t)fftSize * 2, 0.0f), pitchFrame(fftSize, 0.0f), pitches;
    double weightedHz = 0.0, spectralSum = 0.0, low = 0.0, mid = 0.0, presence = 0.0, air = 0.0;
    int frames = 0;
    for (int start = 0; start + fftSize < count; start += hop) {
        float frameRms = 0.0f;
        for (int i = 0; i < fftSize; ++i) { const float x = mono[(size_t)(start + i)]; fftData[(size_t)i] = x; pitchFrame[(size_t)i] = x; frameRms += x*x; }
        frameRms = std::sqrt(frameRms / fftSize); if (frameRms < 0.004f) continue;
        const float hz = StablePitchDetector::detectFrame(pitchFrame.data(), fftSize, reader->sampleRate);
        if (hz >= 60.0f && hz <= 1000.0f) pitches.push_back(hz);
        window.multiplyWithWindowingTable(fftData.data(), fftSize); fft.performFrequencyOnlyForwardTransform(fftData.data());
        for (int b = 1; b < fftSize / 2; ++b) {
            const double freq = b * reader->sampleRate / fftSize; const double mag = fftData[(size_t)b];
            weightedHz += freq * mag; spectralSum += mag;
            if (freq < 250) low += mag; else if (freq < 1200) mid += mag; else if (freq < 5000) presence += mag; else air += mag;
        }
        ++frames;
    }
    if (pitches.size() < 3 || frames == 0 || spectralSum <= 0.0) return p;
    std::sort(pitches.begin(), pitches.end());
    auto quantile = [&pitches](double q) { return pitches[(size_t)juce::jlimit(0, (int)pitches.size()-1, (int)std::round(q*(pitches.size()-1)))]; };
    p.pitchLowHz = quantile(.1); p.meanPitchHz = quantile(.5); p.pitchHighHz = quantile(.9);
    p.spectralCentroidHz = (float)(weightedHz / spectralSum);

    const double total = low + mid + presence + air + 1.0e-12;
    const float lowRatio = (float)(low / total), midRatio = (float)(mid / total), presenceRatio = (float)(presence / total), airRatio = (float)(air / total);
    p.warmthDb = juce::jlimit(-8.0f, 8.0f, 36.0f * (lowRatio - 0.12f));
    p.presenceDb = juce::jlimit(-8.0f, 8.0f, 24.0f * (presenceRatio - 0.42f));
    p.airDb = juce::jlimit(-8.0f, 8.0f, 40.0f * (airRatio - 0.10f));
    p.formantSemitones = juce::jlimit(-4.0f, 4.0f, 12.0f * std::log2(juce::jlimit(0.75f, 1.35f, (midRatio + presenceRatio) / 0.62f)));
    p.compression = juce::jlimit(0.15f, 0.85f, 0.72f - p.crestDb / 30.0f);
    p.deEss = juce::jlimit(0.05f, 0.85f, airRatio * 3.2f);
    p.valid = true;
    return p;
}

