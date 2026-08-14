cmake_minimum_required(VERSION 3.22)
project(UCGVoiceWonder VERSION 0.1.0)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(UCGVoiceWonder
  COMPANY_NAME "UCG Corp Finance"
  BUNDLE_ID com.ucg.voicewonder
  PLUGIN_MANUFACTURER_CODE Ucg1
  PLUGIN_CODE Vw01
  FORMATS VST3 Standalone
  PRODUCT_NAME "UCG Voice Wonder"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(UCGVoiceWonder PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_features(UCGVoiceWonder PRIVATE cxx_std_20)
target_compile_definitions(UCGVoiceWonder PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(UCGVoiceWonder PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

