cmake_minimum_required(VERSION 3.22)
project(SynthBreeder VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The plugin needs JUCE; the core, its tests and the render tool do not.
option(SYNTH_BREEDER_PLUGIN "Build the JUCE plugin (fetches JUCE 9.0.5)" ON)

add_library(synth_core STATIC core/patch.cpp core/engine.cpp core/riff.cpp core/breeder.cpp)
target_include_directories(synth_core PUBLIC core)
set_target_properties(synth_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(synth_core PRIVATE /W4)
else()
  target_compile_options(synth_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
foreach(name litter engine)
  add_executable(${name}_tests tests/${name}_tests.cpp)
  target_link_libraries(${name}_tests PRIVATE synth_core)
  add_test(NAME ${name} COMMAND ${name}_tests)
endforeach()

add_executable(synth_render tools/render.cpp)
target_link_libraries(synth_render PRIVATE synth_core)

if(SYNTH_BREEDER_PLUGIN)
  include(FetchContent)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)

  juce_add_plugin(SynthBreeder
    COMPANY_NAME "Jimmy Nicholas"
    PLUGIN_MANUFACTURER_CODE Jnic
    PLUGIN_CODE Synb
    PRODUCT_NAME "Synth Breeder"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 AU Standalone)

  target_sources(SynthBreeder PRIVATE plugin/processor.cpp plugin/editor.cpp)
  target_compile_definitions(SynthBreeder PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(SynthBreeder
    PRIVATE synth_core juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
endif()
