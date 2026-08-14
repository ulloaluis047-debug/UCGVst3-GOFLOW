#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class UCGVoiceWonderEditor:public juce::AudioProcessorEditor{
public: explicit UCGVoiceWonderEditor(UCGVoiceWonderProcessor&); void paint(juce::Graphics&)override;void resized()override;
private: UCGVoiceWonderProcessor&p; std::array<juce::Slider,8> knobs; juce::ToggleButton midi{"VOICE → MIDI"}; juce::Label title; std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sa; std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> ba; JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UCGVoiceWonderEditor)
};

