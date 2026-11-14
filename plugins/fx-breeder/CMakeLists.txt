cmake_minimum_required(VERSION 3.22)
project(FxBreeder VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The plugin needs JUCE; the core, its tests and the render tool do not.
option(FX_BREEDER_PLUGIN "Build the JUCE plugin (fetches JUCE 9.0.5)" ON)

add_library(fx_core STATIC core/chain.cpp core/effects.cpp core/rack.cpp core/breeder.cpp core/loop.cpp)
target_include_directories(fx_core PUBLIC core)
set_target_properties(fx_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(fx_core PRIVATE /W4)
else()
  target_compile_options(fx_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
foreach(name chain rack)
  add_executable(${name}_tests tests/${name}_tests.cpp)
  target_link_libraries(${name}_tests PRIVATE fx_core)
  add_test(NAME ${name} COMMAND ${name}_tests)
endforeach()

add_executable(fx_render tools/render.cpp)
target_link_libraries(fx_render PRIVATE fx_core)

if(FX_BREEDER_PLUGIN)
  include(FetchContent)
  FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)

  juce_add_plugin(FxBreeder
    COMPANY_NAME "Jimmy Nicholas"
    PLUGIN_MANUFACTURER_CODE Jnic
    PLUGIN_CODE Fxbr
    PRODUCT_NAME "FX Breeder"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS VST3 AU Standalone)

  target_sources(FxBreeder PRIVATE plugin/processor.cpp plugin/editor.cpp)
  target_compile_definitions(FxBreeder PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(FxBreeder
    PRIVATE fx_core juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
endif()
