cmake_minimum_required(VERSION 3.22)
project(BloodlineDrums VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The plugin needs JUCE; the core, its tests and the render tool do not.
option(BLOODLINE_DRUMS_PLUGIN "Build the JUCE plugin (fetches JUCE 9.0.5)" ON)

add_library(bloodline_core STATIC
  core/settings.cpp
  core/row.cpp
  core/voice.cpp
  core/instrument.cpp)
target_include_directories(bloodline_core PUBLIC core)
set_target_properties(bloodline_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(bloodline_core PRIVATE /W4)
else()
  target_compile_options(bloodline_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
foreach(name row voice instrument)
  add_executable(${name}_tests tests/${name}_tests.cpp)
  target_link_libraries(${name}_tests PRIVATE bloodline_core)
  add_test(NAME ${name} COMMAND ${name}_tests)
endforeach()

add_executable(bloodline_render tools/render.cpp)
target_link_libraries(bloodline_render PRIVATE bloodline_core)

if(BLOODLINE_DRUMS_PLUGIN)
  include(FetchContent)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)

  juce_add_plugin(BloodlineDrums
    COMPANY_NAME "Jimmy Nicholas"
    PLUGIN_MANUFACTURER_CODE Jnic
    PLUGIN_CODE Bldl
    PRODUCT_NAME "Bloodline Drums"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 AU Standalone)

  target_sources(BloodlineDrums PRIVATE plugin/processor.cpp plugin/editor.cpp)
  target_compile_definitions(BloodlineDrums PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(BloodlineDrums
    PRIVATE bloodline_core juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
endif()
