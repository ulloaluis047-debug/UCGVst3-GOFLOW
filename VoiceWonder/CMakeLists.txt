cmake_minimum_required(VERSION 3.24)
project(UCGVoiceWonder VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.4
  GIT_SHALLOW TRUE)
FetchContent_Declare(signalsmith_stretch
  GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
  GIT_TAG 57b93f4e9206a089a45387eaa39bdc9f310d3308
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE signalsmith_stretch)

juce_add_plugin(UCGVoiceWonder
  COMPANY_NAME "UCG Corp Finance"
  COMPANY_WEBSITE "https://github.com/ulloaluis047-debug/UCGVst3-GOFLOW"
  COMPANY_EMAIL "ulloaluis047@gmail.com"
  BUNDLE_ID com.ucg.voicewonder
  PLUGIN_MANUFACTURER_CODE Ucg1
  PLUGIN_CODE Vw10
  FORMATS VST3 Standalone
  PRODUCT_NAME "UCG Voice Wonder"
  DESCRIPTION "Production vocal processor, reference matcher and voice-to-MIDI effect"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT TRUE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(UCGVoiceWonder)

set(VOICEWONDER_SOURCES
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/ReferenceAnalyzer.cpp)
target_sources(UCGVoiceWonder PRIVATE ${VOICEWONDER_SOURCES})
target_include_directories(UCGVoiceWonder PRIVATE Source)
target_compile_features(UCGVoiceWonder PRIVATE cxx_std_20)
target_compile_definitions(UCGVoiceWonder PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_DISPLAY_SPLASH_SCREEN=0
  JUCE_REPORT_APP_USAGE=0)
target_link_libraries(UCGVoiceWonder PRIVATE
  signalsmith-stretch
  juce::juce_audio_utils
  juce::juce_audio_formats
  juce::juce_dsp
  PUBLIC
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags)

juce_add_console_app(VoiceWonderSmoke PRODUCT_NAME "Voice Wonder DSP Smoke Test")
juce_generate_juce_header(VoiceWonderSmoke)
target_sources(VoiceWonderSmoke PRIVATE Tests/SmokeTest.cpp)
target_include_directories(VoiceWonderSmoke PRIVATE Source)
target_compile_features(VoiceWonderSmoke PRIVATE cxx_std_20)
target_compile_definitions(VoiceWonderSmoke PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(VoiceWonderSmoke PRIVATE
  signalsmith-stretch
  juce::juce_audio_formats
  juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags)
