#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

struct VoiceProfile {
    float brightness = 0.5f, presence = 0.5f, warmth = 0.5f, dynamics = 0.5f;
};

class PitchTracker {
public:
    void prepare(double sr) { sampleRate = sr; }
    float detect(const float* x, int n) const {
        if (n < 128) return 0.0f;
        const int minLag = juce::jmax(1, (int)(sampleRate / 1000.0));
        const int maxLag = juce::jmin(n / 2, (int)(sampleRate / 65.0));
        float best = 0.0f; int bestLag = 0;
        double energy = 1.0e-9;
        for (int i=0;i<n;i++) energy += x[i]*x[i];
        for (int lag=minLag; lag<=maxLag; ++lag) {
            double c=0.0, e=1.0e-9;
            for (int i=0;i<n-lag;i++) { c += x[i]*x[i+lag]; e += x[i+lag]*x[i+lag]; }
            const float score = (float)(c / std::sqrt(energy*e));
            if (score > best) { best=score; bestLag=lag; }
        }
        return best > 0.55f && bestLag ? (float)(sampleRate / bestLag) : 0.0f;
    }
    static int frequencyToMidi(float hz) { return hz > 0 ? juce::roundToInt(69.0f + 12.0f*std::log2(hz/440.0f)) : -1; }
private: double sampleRate = 44100.0;
};

class VoiceEngine {
public:
    void prepare(const juce::dsp::ProcessSpec& s) {
        sampleRate=s.sampleRate; tracker.prepare(sampleRate);
        hp.prepare(s); lp.prepare(s); presence.prepare(s); compressor.prepare(s);
        hp.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        lp.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        presence.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        delay.assign((size_t)sampleRate*2, 0.0f);
    }
    void reset(){ hp.reset(); lp.reset(); presence.reset(); compressor.reset(); std::fill(delay.begin(),delay.end(),0.0f); write=0; activeMidi=-1; }
    void process(juce::AudioBuffer<float>& b, juce::MidiBuffer& midi, float mix, float pitchSemis,
                 float correction, float radio, float vocoder, float drive, float eqLow, float eqHigh, bool midiOut) {
        const int n=b.getNumSamples();
        if(n==0) return;
        float hz=tracker.detect(b.getReadPointer(0),n); const int note=PitchTracker::frequencyToMidi(hz);
        if(midiOut && note!=activeMidi){ if(activeMidi>=0)midi.addEvent(juce::MidiMessage::noteOff(1,activeMidi),0); if(note>=0)midi.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)100),0); activeMidi=note; }
        juce::AudioBuffer<float> dry; dry.makeCopyOf(b);
        const float target = note>=0 ? 440.0f*std::pow(2.0f,(note-69)/12.0f) : hz;
        float ratio = std::pow(2.0f,pitchSemis/12.0f);
        if(hz>0 && target>0) ratio *= std::pow(target/hz, correction);
        naivePitch(b,ratio);
        juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block);
        hp.setCutoffFrequency(40.0f + radio*260.0f); lp.setCutoffFrequency(19000.0f-radio*15000.0f); presence.setCutoffFrequency(2400.0f);
        hp.process(ctx); lp.process(ctx);
        presence.setResonance(0.7f); presence.process(ctx);
        compressor.setThreshold(-18.0f); compressor.setRatio(1.0f+vocoder*7.0f); compressor.setAttack(4.0f); compressor.setRelease(80.0f); compressor.process(ctx);
        for(int c=0;c<b.getNumChannels();++c){auto* p=b.getWritePointer(c); const auto* d=dry.getReadPointer(c); for(int i=0;i<n;i++){ float shaped=std::tanh(p[i]*(1.0f+drive*7.0f)); shaped += eqLow*0.15f*d[i] + eqHigh*0.08f*(d[i]-(i?d[i-1]:0)); p[i]=d[i]+mix*(shaped-d[i]); }}
    }
    VoiceProfile analyse(const juce::AudioBuffer<float>& b) const {
        VoiceProfile p; if(!b.getNumSamples())return p; const float* x=b.getReadPointer(0); int n=b.getNumSamples(); double rms=0,z=0,slow=0,low=0;
        for(int i=0;i<n;i++){rms+=x[i]*x[i]; if(i&&((x[i]>=0)!=(x[i-1]>=0)))z++; slow=0.995*slow+0.005*x[i];low+=slow*slow;}
        rms=std::sqrt(rms/n); p.brightness=juce::jlimit(0.f,1.f,(float)(z/n*18)); p.warmth=juce::jlimit(0.f,1.f,(float)(low/(rms*rms*n+1e-9))); p.dynamics=juce::jlimit(0.f,1.f,(float)(rms*5)); p.presence=(p.brightness+p.dynamics)*0.5f; return p;
    }
private:
    void naivePitch(juce::AudioBuffer<float>& b,float ratio){ if(std::abs(ratio-1.f)<0.002f)return; const int n=b.getNumSamples(); juce::AudioBuffer<float> t; t.makeCopyOf(b); for(int c=0;c<b.getNumChannels();c++){auto*o=b.getWritePointer(c);auto*i=t.getReadPointer(c);for(int s=0;s<n;s++){float pos=std::fmod(s*ratio,(float)juce::jmax(1,n-1));int a=(int)pos;float f=pos-a;o[s]=i[a]+f*(i[juce::jmin(a+1,n-1)]-i[a]);}}}
    double sampleRate=44100; PitchTracker tracker; juce::dsp::StateVariableTPTFilter<float> hp,lp,presence; juce::dsp::Compressor<float> compressor; std::vector<float> delay; size_t write=0; int activeMidi=-1;
};

