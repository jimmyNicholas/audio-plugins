# PROTOTYPE (throwaway spike): Bloodline Synth 2, the sounds and the loops that play them in one plugin.
cmake_minimum_required(VERSION 3.22)
project(BloodlineSynth2 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The plugin needs JUCE; the core and its tests do not.
option(BLOODLINE_SYNTH2_PLUGIN "Build the JUCE plugin (fetches JUCE 9.0.5)" ON)

# Bloodline Synth's patch, engine and riff, as they are, without moving them into plugins/shared.
set(SYNTH_CORE ${CMAKE_CURRENT_SOURCE_DIR}/../bloodline-synth/core)
add_library(bs2_core STATIC core/sounds.cpp core/loops.cpp core/studio.cpp
  ${SYNTH_CORE}/patch.cpp ${SYNTH_CORE}/engine.cpp ${SYNTH_CORE}/riff.cpp)
target_include_directories(bs2_core PUBLIC core ${SYNTH_CORE})
set_target_properties(bs2_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(bs2_core PRIVATE /W4)
else()
  target_compile_options(bs2_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(bs2_tests tests/bs2_tests.cpp)
target_link_libraries(bs2_tests PRIVATE bs2_core)
add_test(NAME bs2 COMMAND bs2_tests)

if(BLOODLINE_SYNTH2_PLUGIN)
  include(FetchContent)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)

  juce_add_plugin(BloodlineSynth2
    COMPANY_NAME "Jimmy Nicholas"
    PLUGIN_MANUFACTURER_CODE Jnic
    PLUGIN_CODE Syn2
    PRODUCT_NAME "Bloodline Synth 2"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 AU Standalone)

  target_sources(BloodlineSynth2 PRIVATE plugin/processor.cpp plugin/editor.cpp)
  target_compile_definitions(BloodlineSynth2 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(BloodlineSynth2
    PRIVATE bs2_core juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
endif()
