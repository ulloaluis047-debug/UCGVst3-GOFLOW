#pragma once
#include <JuceHeader.h>

struct VocalReferenceProfile {
    juce::String name { "Reference Match" };
    bool valid = false;
    float meanPitchHz = 0.0f;
    float pitchLowHz = 0.0f;
    float pitchHighHz = 0.0f;
    float rmsDb = -60.0f;
    float crestDb = 0.0f;
    float spectralCentroidHz = 0.0f;
    float warmthDb = 0.0f;
    float presenceDb = 0.0f;
    float airDb = 0.0f;
    float formantSemitones = 0.0f;
    float compression = 0.35f;
    float deEss = 0.25f;

    juce::var toVar() const;
    static VocalReferenceProfile fromVar(const juce::var& value);
    juce::String summary() const;
};

class ReferenceAnalyzer {
public:
    static VocalReferenceProfile analyse(const juce::File& audioFile);
};

