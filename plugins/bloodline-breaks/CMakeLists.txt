# PROTOTYPE (throwaway spike): Bloodline Breaks.
cmake_minimum_required(VERSION 3.22)
project(BloodlineBreaks VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The plugin needs JUCE; the core and its tests do not.
option(BLOODLINE_BREAKS_PLUGIN "Build the JUCE plugin (fetches JUCE 9.0.5)" ON)

# The built-in kit borrows Bloodline Drums' voice as it is, without moving it into plugins/shared.
set(DRUMS_CORE ${CMAKE_CURRENT_SOURCE_DIR}/../bloodline-drums/core)
add_library(breaks_core STATIC core/pattern.cpp core/breeder.cpp core/player.cpp
  ${DRUMS_CORE}/settings.cpp ${DRUMS_CORE}/voice.cpp)
target_include_directories(breaks_core PUBLIC core ${DRUMS_CORE})
set_target_properties(breaks_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(breaks_core PRIVATE /W4)
else()
  target_compile_options(breaks_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_executable(breaks_tests tests/breaks_tests.cpp)
target_link_libraries(breaks_tests PRIVATE breaks_core)
add_test(NAME breaks COMMAND breaks_tests)

if(BLOODLINE_BREAKS_PLUGIN)
  include(FetchContent)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)

  juce_add_plugin(BloodlineBreaks
    COMPANY_NAME "Jimmy Nicholas"
    PLUGIN_MANUFACTURER_CODE Jnic
    PLUGIN_CODE Brks
    PRODUCT_NAME "Bloodline Breaks"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 AU Standalone)

  target_sources(BloodlineBreaks PRIVATE plugin/processor.cpp plugin/editor.cpp)
  target_compile_definitions(BloodlineBreaks PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(BloodlineBreaks
    PRIVATE breaks_core juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
endif()
