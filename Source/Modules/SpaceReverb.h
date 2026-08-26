#pragma once
#include <JuceHeader.h>
#include <vector>

class SpaceReverb
{
public:
    SpaceReverb();
    ~SpaceReverb();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void process(const juce::dsp::ProcessContextReplacing<float>& context);
    void reset();

    void setParameters(float size, float depth, float time, float mix);

private:
    // --- DSP Components ---
    
    class DelayLine {
    public:
        void prepare(double sampleRate, double maxDelayMs);
        void reset();
        float read(float delayMs) const;
        void write(float sample);
        float processAllPass(float in, float delayMs, float coeff);
    private:
        std::vector<float> buffer;
        int writePos = 0;
        double fs = 44100.0;
    };

    class OnePoleLPF {
    public:
        void setCutoff(float cutoffHz, double sampleRate);
        float process(float in);
        void reset();
    private:
        float z1 = 0.0f;
        float a0 = 1.0f;
        float b1 = 0.0f;
    };

    class LFO {
    public:
        void prepare(double sampleRate);
        void setFrequency(float freqHz);
        float process();
    private:
        float phase = 0.0f;
        float phaseInc = 0.0f;
        double fs = 44100.0;
    };

    // --- Reverb Network ---
    
    // Input Diffusion
    DelayLine preDelay;
    DelayLine ap1, ap2, ap3, ap4;
    
    // Tank Left
    DelayLine tankAp1;
    DelayLine tankDelay1;
    OnePoleLPF lpf1;
    DelayLine tankAp2;
    DelayLine tankDelay2;
    
    // Tank Right
    DelayLine tankAp3;
    DelayLine tankDelay3;
    OnePoleLPF lpf2;
    DelayLine tankAp4;
    DelayLine tankDelay4;

    LFO lfo1, lfo2;

    double sampleRate = 44100.0;

    // State
    float feedbackLeft = 0.0f;
    float feedbackRight = 0.0f;

    // Params
    float pSize = 1.0f;
    float pDecay = 0.5f;
    float pMix = 0.5f;
};
