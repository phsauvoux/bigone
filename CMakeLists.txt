cmake_minimum_required(VERSION 3.22)

project(PhSynthOne VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.10
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(PhSynthOne
  COMPANY_NAME "Ph"
  BUNDLE_ID "com.ph.phsynthone"
  PLUGIN_MANUFACTURER_CODE PhSv
  PLUGIN_CODE PsO1
  FORMATS VST3 Standalone
  PRODUCT_NAME "PhSynth One"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(PhSynthOne)

target_sources(PhSynthOne PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
)

target_compile_definitions(PhSynthOne PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PhSynthOne PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  juce::juce_gui_extra
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)
