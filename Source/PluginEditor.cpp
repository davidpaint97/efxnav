#include "PluginProcessor.h"
#include "PluginEditor.h"

EfxNavAudioProcessorEditor::EfxNavAudioProcessorEditor (EfxNavAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 400);
    
    // Configurar Sliders
    auto setupSlider = [this](juce::Slider& slider, const juce::String& name) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(slider);
    };

    setupSlider(sizeSlider, "Size");
    setupSlider(depthSlider, "Depth");
    setupSlider(timeSlider, "Time");
    setupSlider(mixSlider, "Mix");

    // Conectar Sliders al APVTS
    sizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "space_size", sizeSlider);
    depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "space_depth", depthSlider);
    timeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "space_time", timeSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "space_mix", mixSlider);
}

EfxNavAudioProcessorEditor::~EfxNavAudioProcessorEditor()
{
}

void EfxNavAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff1e1e1e)); // Fondo oscuro estilo rack

    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("EFX NAV - Space Module (Lexicon 480L Style)", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);

    // Dibujar etiquetas
    g.setFont (14.0f);
    auto drawLabel = [&](juce::Slider& slider, const juce::String& text) {
        auto bounds = slider.getBounds().withY(slider.getY() - 20).withHeight(20);
        g.drawFittedText(text, bounds, juce::Justification::centred, 1);
    };

    drawLabel(sizeSlider, "SIZE");
    drawLabel(depthSlider, "DEPTH");
    drawLabel(timeSlider, "TIME");
    drawLabel(mixSlider, "MIX");
}

void EfxNavAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop(80).reduced(20);
    
    int sliderWidth = area.getWidth() / 4;
    int sliderHeight = 120;
    
    sizeSlider.setBounds(area.removeFromLeft(sliderWidth).withSizeKeepingCentre(100, sliderHeight));
    depthSlider.setBounds(area.removeFromLeft(sliderWidth).withSizeKeepingCentre(100, sliderHeight));
    timeSlider.setBounds(area.removeFromLeft(sliderWidth).withSizeKeepingCentre(100, sliderHeight));
    mixSlider.setBounds(area.removeFromLeft(sliderWidth).withSizeKeepingCentre(100, sliderHeight));
}
