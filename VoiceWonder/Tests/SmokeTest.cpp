#include <JuceHeader.h>
#include "VoiceDSP.h"
#include <cmath>
#include <iostream>

int main() {
    constexpr double sampleRate=48000.0; constexpr int blockSize=512,channels=2,blocks=700;
    VoiceEngine engine; engine.prepare(sampleRate,blockSize,channels); DspSettings s;
    s.tune=.72f;s.tuneSpeed=.65f;s.formant=-.7f;s.warmth=2.f;s.presence=2.2f;s.air=2.5f;s.deEss=.45f;s.compression=.58f;s.drive=.12f;s.doubler=.18f;s.width=1.2f;s.delayMix=.06f;s.reverb=.12f;
    juce::AudioBuffer<float> audio(channels,blockSize); juce::MidiBuffer midi; double phase=0,energy=0;float previous=0,maxJump=0,peak=0;long long finiteCount=0,activeCount=0;
    for(int b=0;b<blocks;b++){audio.clear();const double f=105.0+95.0*(.5+.5*std::sin(b*.017));for(int i=0;i<blockSize;i++){phase+=2.0*juce::MathConstants<double>::pi*f/sampleRate;if(phase>juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;float env=.28f*(.75f+.25f*std::sin((b*blockSize+i)*2.0*juce::MathConstants<double>::pi*4.7/sampleRate));float x=env*((float)std::sin(phase)+.42f*(float)std::sin(phase*2)+.2f*(float)std::sin(phase*3));audio.setSample(0,i,x);audio.setSample(1,i,x*.98f);}midi.clear();engine.process(audio,midi,s);for(int c=0;c<channels;c++)for(int i=0;i<blockSize;i++){float x=audio.getSample(c,i);if(!std::isfinite(x)){std::cerr<<"FAIL non-finite sample\n";return 2;}peak=juce::jmax(peak,std::abs(x));maxJump=juce::jmax(maxJump,std::abs(x-previous));previous=x;energy+=x*x;finiteCount++;if(std::abs(x)>1e-5f)activeCount++;}}
    const double rms=std::sqrt(energy/finiteCount),activity=(double)activeCount/finiteCount;
    std::cout<<"latency="<<engine.getLatencySamples()<<" rms="<<rms<<" peak="<<peak<<" maxJump="<<maxJump<<" activity="<<activity<<" pitch="<<engine.getDetectedPitchHz()<<"\n";
    if(rms<.01||peak>1.1f||maxJump>1.5f||activity<.70){std::cerr<<"FAIL stability thresholds\n";return 3;}
    std::cout<<"PASS continuous vocal DSP smoke test\n";return 0;
}

