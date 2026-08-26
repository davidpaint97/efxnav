cmake_minimum_required(VERSION 3.21)

# Define the project name and version
project(EfxNavPlugin VERSION 1.0.0)

# Fetch JUCE from GitHub
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0 # Puedes cambiar esto por la versión que necesites
)
FetchContent_MakeAvailable(juce)

# Configurar el plugin de audio
juce_add_plugin(EfxNavPlugin
    # Detalles de la empresa y plugin
    COMPANY_NAME "EfxNav"
    PLUGIN_MANUFACTURER_CODE "EfxN"
    PLUGIN_CODE "EfxN"
    FORMATS VST3 AU
    PRODUCT_NAME "EfxNav Plugin"
)

# Añadir archivos fuente
target_sources(EfxNavPlugin
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Modules/SpaceReverb.cpp
        Source/Modules/SpaceReverb.h
)

# Definiciones de compilación
target_compile_definitions(EfxNavPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Vincular los módulos de JUCE necesarios
target_link_libraries(EfxNavPlugin
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
