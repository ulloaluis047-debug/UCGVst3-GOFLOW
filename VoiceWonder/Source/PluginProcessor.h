#pragma once
#include <JuceHeader.h>
#include "VoiceDSP.h"

class UCGVoiceWonderProcessor : public juce::AudioProcessor {
public:
    UCGVoiceWonderProcessor();
    void prepareToPlay(double,int) override; void releaseResources() override{};
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
    const juce::String getName() const override{return "UCG Voice Wonder";}
    bool acceptsMidi()const override{return false;} bool producesMidi()const override{return true;} bool isMidiEffect()const override{return false;}
    double getTailLengthSeconds()const override{return 0;}
    int getNumPrograms()override{return 1;} int getCurrentProgram()override{return 0;} void setCurrentProgram(int)override{};
    const juce::String getProgramName(int)override{return "Default";} void changeProgramName(int,const juce::String&)override{};
    void getStateInformation(juce::MemoryBlock&)override; void setStateInformation(const void*,int)override;
    juce::AudioProcessorValueTreeState apvts; VoiceProfile profile; void captureProfile(const juce::AudioBuffer<float>& b){profile=engine.analyse(b);}
private: static juce::AudioProcessorValueTreeState::ParameterLayout layout(); VoiceEngine engine; JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UCGVoiceWonderProcessor)
};

