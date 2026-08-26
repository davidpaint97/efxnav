#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class EfxNavAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EfxNavAudioProcessorEditor (EfxNavAudioProcessor&);
    ~EfxNavAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    EfxNavAudioProcessor& audioProcessor;
    
    // Sliders
    juce::Slider sizeSlider;
    juce::Slider depthSlider;
    juce::Slider timeSlider;
    juce::Slider mixSlider;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> timeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EfxNavAudioProcessorEditor)
};
