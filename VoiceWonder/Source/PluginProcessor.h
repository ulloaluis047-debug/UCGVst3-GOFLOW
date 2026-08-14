#pragma once
#include <JuceHeader.h>
#include "VoiceDSP.h"
#include "ReferenceAnalyzer.h"

class UCGVoiceWonderProcessor final : public juce::AudioProcessor {
public:
    UCGVoiceWonderProcessor();
    void prepareToPlay(double,int) override; void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "UCG Voice Wonder"; }
    bool acceptsMidi() const override { return false; } bool producesMidi() const override { return true; } bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }
    int getNumPrograms() override; int getCurrentProgram() override; void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override; void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::StringArray getPresetNames() const;
    void applyPreset(int index);
    void applyReferenceProfile(const VocalReferenceProfile& profile);
    bool saveUserPreset(const juce::File& file) const;
    bool loadUserPreset(const juce::File& file);
    float getDetectedPitchHz() const noexcept { return detectedPitch.load(); }
    juce::String getCurrentProfileSummary() const { const juce::ScopedLock l(profileLock); return profileSummary; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    DspSettings readSettings() const;
    VoiceEngine engine;
    std::atomic<float> detectedPitch { 0.0f };
    int currentProgram = 0;
    mutable juce::CriticalSection profileLock;
    juce::String profileSummary { "Sin referencia analizada" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UCGVoiceWonderProcessor)
};

