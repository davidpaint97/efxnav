#include "SpaceReverb.h"
#include <cmath>
#include <algorithm>

// --- DelayLine Implementation ---
void SpaceReverb::DelayLine::prepare(double sampleRate, double maxDelayMs) {
    fs = sampleRate;
    int maxSamples = static_cast<int>(sampleRate * maxDelayMs / 1000.0) + 10;
    buffer.assign(maxSamples, 0.0f);
    writePos = 0;
}

void SpaceReverb::DelayLine::reset() {
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    writePos = 0;
}

void SpaceReverb::DelayLine::write(float sample) {
    buffer[writePos] = sample;
    writePos = (writePos + 1) % buffer.size();
}

float SpaceReverb::DelayLine::read(float delayMs) const {
    float delaySamples = delayMs * static_cast<float>(fs / 1000.0);
    float readPos = static_cast<float>(writePos) - delaySamples;
    if (readPos < 0.0f) readPos += static_cast<float>(buffer.size());
    
    int index1 = static_cast<int>(readPos);
    int index2 = (index1 + 1) % buffer.size();
    float frac = readPos - static_cast<float>(index1);
    
    return buffer[index1] * (1.0f - frac) + buffer[index2] * frac;
}

float SpaceReverb::DelayLine::processAllPass(float in, float delayMs, float coeff) {
    float delayed = read(delayMs);
    float w = in + coeff * delayed;
    write(w);
    return delayed - coeff * w;
}

// --- OnePoleLPF Implementation ---
void SpaceReverb::OnePoleLPF::setCutoff(float cutoffHz, double sampleRate) {
    float wc = juce::MathConstants<float>::twoPi * cutoffHz / static_cast<float>(sampleRate);
    b1 = std::exp(-wc);
    a0 = 1.0f - b1;
}

float SpaceReverb::OnePoleLPF::process(float in) {
    z1 = in * a0 + z1 * b1;
    return z1;
}

void SpaceReverb::OnePoleLPF::reset() {
    z1 = 0.0f;
}

// --- LFO Implementation ---
void SpaceReverb::LFO::prepare(double sampleRate) {
    fs = sampleRate;
}

void SpaceReverb::LFO::setFrequency(float freqHz) {
    phaseInc = juce::MathConstants<float>::twoPi * freqHz / static_cast<float>(fs);
}

float SpaceReverb::LFO::process() {
    float out = std::sin(phase);
    phase += phaseInc;
    if (phase >= juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;
    return out;
}

// --- SpaceReverb Implementation ---
SpaceReverb::SpaceReverb() {}
SpaceReverb::~SpaceReverb() {}

void SpaceReverb::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    
    // Allocate max 2000ms for safety
    preDelay.prepare(sampleRate, 200.0);
    ap1.prepare(sampleRate, 50.0);
    ap2.prepare(sampleRate, 50.0);
    ap3.prepare(sampleRate, 100.0);
    ap4.prepare(sampleRate, 100.0);
    
    tankAp1.prepare(sampleRate, 200.0);
    tankDelay1.prepare(sampleRate, 500.0);
    tankAp2.prepare(sampleRate, 200.0);
    tankDelay2.prepare(sampleRate, 500.0);
    
    tankAp3.prepare(sampleRate, 200.0);
    tankDelay3.prepare(sampleRate, 500.0);
    tankAp4.prepare(sampleRate, 200.0);
    tankDelay4.prepare(sampleRate, 500.0);

    lfo1.prepare(sampleRate);
    lfo2.prepare(sampleRate);
    lfo1.setFrequency(0.6f);
    lfo2.setFrequency(0.83f);

    reset();
}

void SpaceReverb::reset()
{
    preDelay.reset();
    ap1.reset(); ap2.reset(); ap3.reset(); ap4.reset();
    tankAp1.reset(); tankDelay1.reset(); tankAp2.reset(); tankDelay2.reset();
    tankAp3.reset(); tankDelay3.reset(); tankAp4.reset(); tankDelay4.reset();
    lpf1.reset(); lpf2.reset();
    feedbackLeft = 0.0f;
    feedbackRight = 0.0f;
}

void SpaceReverb::setParameters(float size, float depth, float time, float mix)
{
    // size (0-1): scales the room size -> scales delay times
    pSize = 0.5f + size * 1.5f; // multiplier from 0.5x to 2.0x

    // depth (0-1): controls damping cutoff (10000 Hz down to 1000 Hz)
    float cutoff = 10000.0f - (depth * 9000.0f);
    lpf1.setCutoff(cutoff, sampleRate);
    lpf2.setCutoff(cutoff, sampleRate);

    // time (0-1): controls decay feedback coefficient (0.3 to 0.98)
    pDecay = 0.3f + time * 0.68f;

    pMix = mix;
}

void SpaceReverb::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    int numChannels = static_cast<int>(inputBlock.getNumChannels());
    int numSamples = static_cast<int>(inputBlock.getNumSamples());

    // Delay times in ms (inspired by Dattorro, scaled by pSize)
    float t_ap1 = 4.771f * pSize;
    float t_ap2 = 3.595f * pSize;
    float t_ap3 = 12.73f * pSize;
    float t_ap4 = 9.307f * pSize;
    
    float t_tankAp1 = 22.58f * pSize;
    float t_tankD1  = 149.6f * pSize;
    float t_tankAp2 = 60.48f * pSize;
    float t_tankD2  = 125.0f * pSize;

    float t_tankAp3 = 30.51f * pSize;
    float t_tankD3  = 141.69f * pSize;
    float t_tankAp4 = 89.24f * pSize;
    float t_tankD4  = 106.28f * pSize;

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = inputBlock.getChannelPointer(0)[i];
        float inR = numChannels > 1 ? inputBlock.getChannelPointer(1)[i] : inL;
        
        // Mono mix for input
        float monoIn = (inL + inR) * 0.5f;

        // Pre-delay
        preDelay.write(monoIn);
        float diffused = preDelay.read(40.0f); // 40ms predelay
        
        // Input Diffusion (4 APFs in series)
        diffused = ap1.processAllPass(diffused, t_ap1, 0.75f);
        diffused = ap2.processAllPass(diffused, t_ap2, 0.75f);
        diffused = ap3.processAllPass(diffused, t_ap3, 0.625f);
        diffused = ap4.processAllPass(diffused, t_ap4, 0.625f);

        // Figure-8 Tank
        // Left side modulation (LFOs return -1 to +1, we modulate by +/- 1.0 ms)
        float modLeft = lfo1.process() * 1.0f;
        float modRight = lfo2.process() * 1.0f;

        // Left Tank Branch
        float leftNode = diffused + feedbackRight * pDecay;
        float apLeft = tankAp1.processAllPass(leftNode, t_tankAp1 + modLeft, 0.7f);
        tankDelay1.write(apLeft);
        float d1Left = tankDelay1.read(t_tankD1);
        d1Left = lpf1.process(d1Left);
        float ap2Left = tankAp2.processAllPass(d1Left, t_tankAp2, 0.5f);
        tankDelay2.write(ap2Left);
        feedbackLeft = tankDelay2.read(t_tankD2); // crosses to right

        // Right Tank Branch
        float rightNode = diffused + feedbackLeft * pDecay;
        float apRight = tankAp3.processAllPass(rightNode, t_tankAp3 + modRight, 0.7f);
        tankDelay3.write(apRight);
        float d1Right = tankDelay3.read(t_tankD3);
        d1Right = lpf2.process(d1Right);
        float ap2Right = tankAp4.processAllPass(d1Right, t_tankAp4, 0.5f);
        tankDelay4.write(ap2Right);
        feedbackRight = tankDelay4.read(t_tankD4); // crosses to left

        // Taps (Simplified reading positions for output)
        float outL = tankDelay1.read(t_tankD1 * 0.5f) - tankAp2.read(t_tankAp2 * 0.5f) + tankDelay2.read(t_tankD2 * 0.5f);
        float outR = tankDelay3.read(t_tankD3 * 0.5f) - tankAp4.read(t_tankAp4 * 0.5f) + tankDelay4.read(t_tankD4 * 0.5f);

        // Mix
        if (numChannels > 0)
            outputBlock.getChannelPointer(0)[i] = inL * (1.0f - pMix) + outL * pMix;
        if (numChannels > 1)
            outputBlock.getChannelPointer(1)[i] = inR * (1.0f - pMix) + outR * pMix;
    }
}
