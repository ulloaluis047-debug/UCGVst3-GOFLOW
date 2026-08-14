#pragma once
#include <JuceHeader.h>
#include <initializer_list>
#include <utility>
#include <vector>

struct FactoryPreset {
    juce::String name;
    std::vector<std::pair<juce::String, float>> values;
};

inline const std::vector<FactoryPreset>& getFactoryPresets() {
    static const std::vector<FactoryPreset> presets {
        { "01 Clean Studio", {{"gate",.18f},{"tune",.30f},{"tunespeed",.35f},{"warmth",1.2f},{"presence",1.5f},{"air",1.8f},{"deess",.32f},{"compress",.42f},{"drive",.06f},{"doubler",.05f},{"reverb",.10f}} },
        { "02 Reggaeton Lead", {{"gate",.24f},{"tune",.78f},{"tunespeed",.78f},{"warmth",1.8f},{"presence",2.8f},{"air",3.4f},{"deess",.48f},{"compress",.62f},{"drive",.14f},{"doubler",.16f},{"width",1.12f},{"delay",.09f},{"delaytime",250.f},{"reverb",.13f}} },
        { "03 Romantic Air", {{"gate",.15f},{"tune",.56f},{"tunespeed",.48f},{"warmth",2.5f},{"presence",1.0f},{"air",5.2f},{"deess",.58f},{"compress",.48f},{"drive",.05f},{"doubler",.22f},{"width",1.28f},{"delay",.13f},{"delaytime",375.f},{"reverb",.25f}} },
        { "04 Maleanteo Dark", {{"gate",.28f},{"tune",.68f},{"tunespeed",.65f},{"pitch",-2.f},{"formant",-1.7f},{"warmth",5.0f},{"presence",-1.5f},{"air",-2.f},{"deess",.20f},{"compress",.70f},{"drive",.28f},{"doubler",.12f},{"reverb",.08f}} },
        { "05 Pop Radio Ready", {{"gate",.22f},{"tune",.70f},{"tunespeed",.68f},{"warmth",.5f},{"presence",4.0f},{"air",5.5f},{"deess",.66f},{"compress",.68f},{"drive",.10f},{"doubler",.19f},{"width",1.18f},{"delay",.07f},{"reverb",.15f}} },
        { "06 Hard Tune", {{"gate",.20f},{"tune",1.f},{"tunespeed",1.f},{"warmth",1.f},{"presence",2.5f},{"air",2.5f},{"deess",.45f},{"compress",.58f},{"drive",.12f},{"reverb",.08f}} },
        { "07 Natural Tune", {{"gate",.12f},{"tune",.38f},{"tunespeed",.22f},{"warmth",1.2f},{"presence",1.0f},{"air",1.5f},{"deess",.30f},{"compress",.36f},{"drive",.03f},{"reverb",.10f}} },
        { "08 Deep Voice", {{"tune",.40f},{"tunespeed",.35f},{"pitch",-4.f},{"formant",-2.8f},{"warmth",5.5f},{"presence",-2.f},{"air",-3.f},{"compress",.56f},{"drive",.20f},{"doubler",.10f}} },
        { "09 High Character", {{"tune",.48f},{"tunespeed",.42f},{"pitch",3.f},{"formant",2.2f},{"warmth",-2.f},{"presence",3.2f},{"air",4.5f},{"deess",.62f},{"compress",.52f},{"doubler",.15f}} },
        { "10 Robot Vocoder", {{"tune",.92f},{"tunespeed",.90f},{"vocoder",.88f},{"radio",.18f},{"compress",.62f},{"drive",.18f},{"doubler",.08f},{"reverb",.12f}} },
        { "11 Radio Intro", {{"tune",.22f},{"radio",.94f},{"presence",3.f},{"compress",.66f},{"drive",.26f},{"delay",.10f},{"delaytime",190.f},{"reverb",.08f}} },
        { "12 Telephone", {{"radio",1.f},{"presence",5.f},{"air",-6.f},{"compress",.72f},{"drive",.10f},{"mix",1.f}} },
        { "13 Warm Vintage", {{"tune",.22f},{"warmth",6.f},{"presence",-1.f},{"air",-2.5f},{"deess",.18f},{"compress",.50f},{"drive",.24f},{"doubler",.08f},{"reverb",.17f}} },
        { "14 Wide Doubles", {{"tune",.46f},{"tunespeed",.40f},{"warmth",1.f},{"presence",2.f},{"air",2.f},{"compress",.52f},{"doubler",.72f},{"width",1.65f},{"delay",.06f},{"reverb",.14f}} },
        { "15 Whisper Shine", {{"gate",.08f},{"tune",.18f},{"warmth",-2.f},{"presence",2.8f},{"air",7.f},{"deess",.78f},{"compress",.72f},{"doubler",.18f},{"reverb",.30f}} },
        { "16 Grit Adlib", {{"gate",.30f},{"tune",.62f},{"tunespeed",.64f},{"formant",-1.f},{"warmth",3.f},{"presence",3.5f},{"air",1.f},{"compress",.76f},{"drive",.64f},{"doubler",.36f},{"width",1.38f},{"delay",.17f},{"reverb",.18f}} },
        { "17 Spacious Hook", {{"tune",.58f},{"tunespeed",.52f},{"warmth",1.5f},{"presence",2.f},{"air",4.f},{"deess",.52f},{"compress",.54f},{"doubler",.30f},{"width",1.42f},{"delay",.22f},{"delaytime",420.f},{"delayfeedback",.34f},{"reverb",.38f}} },
        { "18 Live Low Latency", {{"gate",.16f},{"tune",.18f},{"tunespeed",.18f},{"warmth",.8f},{"presence",1.4f},{"air",1.f},{"deess",.28f},{"compress",.38f},{"drive",.04f},{"doubler",0.f},{"delay",0.f},{"reverb",.06f}} }
    };
    return presets;
}

inline void setParameterPlain(juce::AudioProcessorValueTreeState& state, const juce::String& id, float plainValue) {
    if (auto* parameter = state.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(plainValue));
}

inline void applyFactoryPreset(juce::AudioProcessorValueTreeState& state, int index) {
    const auto& presets = getFactoryPresets(); if (!juce::isPositiveAndBelow(index, (int)presets.size())) return;
    const std::vector<std::pair<juce::String,float>> defaults {
        {"mix",1.f},{"input",0.f},{"gate",.12f},{"tune",.35f},{"tunespeed",.35f},{"key",0.f},{"scale",0.f},
        {"pitch",0.f},{"formant",0.f},{"warmth",0.f},{"presence",0.f},{"air",0.f},{"deess",.25f},{"compress",.35f},
        {"drive",0.f},{"radio",0.f},{"vocoder",0.f},{"doubler",0.f},{"width",1.f},{"delay",0.f},{"delaytime",280.f},
        {"delayfeedback",.22f},{"reverb",.08f},{"output",0.f}
    };
    for (const auto& [id,value] : defaults) setParameterPlain(state,id,value);
    for (const auto& [id,value] : presets[(size_t)index].values) setParameterPlain(state,id,value);
    state.state.setProperty("presetName", presets[(size_t)index].name, nullptr);
}

