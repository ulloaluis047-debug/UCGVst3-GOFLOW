#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class UCGVoiceWonderEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit UCGVoiceWonderEditor(UCGVoiceWonderProcessor&);
    ~UCGVoiceWonderEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
    struct Knob { juce::Slider slider; juce::Label label; std::unique_ptr<SliderAttachment> attachment; };
    void addKnob(const juce::String& id,const juce::String& name);
    void chooseReference(); void savePreset(); void loadPreset(); void timerCallback() override;
    UCGVoiceWonderProcessor& processor;
    std::vector<std::unique_ptr<Knob>> knobs;
    juce::Label title,subtitle,status,pitchReadout,presetLabel,keyLabel,scaleLabel;
    juce::ComboBox presetBox,keyBox,scaleBox;
    juce::TextButton analyseButton{"ANALYSE VOICE FILE"},saveButton{"SAVE PRESET"},loadButton{"LOAD PRESET"};
    juce::ToggleButton midiButton{"VOICE TO MIDI"};
    std::unique_ptr<ComboAttachment> keyAttachment,scaleAttachment;
    std::unique_ptr<ButtonAttachment> midiAttachment;
    std::unique_ptr<juce::FileChooser> fileChooser;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UCGVoiceWonderEditor)
};

