#pragma once
#include <JuceHeader.h>
#include <signalsmith-stretch/signalsmith-stretch.h>
#include "PitchDetector.h"
#include <array>
#include <cmath>
#include <vector>

struct DspSettings {
    float mix=1, inputDb=0, gate=.12f, tune=.35f, tuneSpeed=.35f, key=0, scale=0;
    float pitch=0, formant=0, warmth=0, presence=0, air=0, deEss=.25f, compression=.35f;
    float drive=0, radio=0, vocoder=0, doubler=0, width=1, delayMix=0, delayMs=280, delayFeedback=.22f;
    float reverb=.08f, outputDb=0; bool midiOut=true;
};

class ModulatedDoubler {
public:
    void prepare(double sr, int channels) {
        sampleRate=sr; delaySize=(int)(sr*.08)+4; data.assign((size_t)channels,std::vector<float>((size_t)delaySize,0)); write=0; phase=0;
    }
    void reset(){for(auto&c:data)std::fill(c.begin(),c.end(),0);write=0;phase=0;}
    void process(juce::AudioBuffer<float>& b,float amount) {
        if(amount<=.0001f||data.empty())return; const int n=b.getNumSamples(),ch=b.getNumChannels();
        for(int i=0;i<n;i++){
            phase+=2.0*juce::MathConstants<double>::pi*.31/sampleRate;if(phase>juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;
            for(int c=0;c<ch;c++){
                auto&d=data[(size_t)c];float* x=b.getWritePointer(c);d[(size_t)write]=x[i];
                const double base=.018*sampleRate, mod=.0045*sampleRate*std::sin(phase+(c?juce::MathConstants<double>::pi:0));
                double rp=write-(base+mod);while(rp<0)rp+=delaySize;int a=(int)rp,bb=(a+1)%delaySize;float f=(float)(rp-a);
                const float delayed=d[(size_t)a]+f*(d[(size_t)bb]-d[(size_t)a]);x[i]+=amount*.72f*delayed;
            } write=(write+1)%delaySize;
        }
    }
private:double sampleRate=44100,phase=0;int delaySize=1,write=0;std::vector<std::vector<float>>data;
};

class StereoDelay {
public:
    void prepare(double sr,int channels){sampleRate=sr;size=(int)(sr*2.1)+4;data.assign((size_t)channels,std::vector<float>((size_t)size,0));write=0;}
    void reset(){for(auto&c:data)std::fill(c.begin(),c.end(),0);write=0;}
    void process(juce::AudioBuffer<float>&b,float mix,float ms,float feedback){if(mix<=.0001f)return;const int delay=juce::jlimit(1,size-2,(int)(ms*.001*sampleRate));for(int i=0;i<b.getNumSamples();i++){for(int c=0;c<b.getNumChannels();c++){float*x=b.getWritePointer(c);auto&d=data[(size_t)c];int r=(write+size-delay)%size;float wet=d[(size_t)r];d[(size_t)write]=juce::jlimit(-2.f,2.f,x[i]+wet*feedback);x[i]+=wet*mix;}write=(write+1)%size;}}
private:double sampleRate=44100;int size=1,write=0;std::vector<std::vector<float>>data;
};

class SimpleVocoder {
public:
    void prepare(const juce::dsp::ProcessSpec&s){sampleRate=s.sampleRate;channels=(int)s.numChannels;maxBlock=(int)s.maximumBlockSize;carrier.setSize(channels,maxBlock);modBand.setSize(channels,maxBlock);carBand.setSize(channels,maxBlock);sum.setSize(channels,maxBlock);juce::dsp::ProcessSpec spec=s;for(int i=0;i<bands;i++){mod[(size_t)i].prepare(spec);car[(size_t)i].prepare(spec);mod[(size_t)i].setType(juce::dsp::StateVariableTPTFilterType::bandpass);car[(size_t)i].setType(juce::dsp::StateVariableTPTFilterType::bandpass);float f=centres[(size_t)i];mod[(size_t)i].setCutoffFrequency(f);car[(size_t)i].setCutoffFrequency(f);mod[(size_t)i].setResonance(.9f);car[(size_t)i].setResonance(.9f);}reset();}
    void reset(){for(auto&f:mod)f.reset();for(auto&f:car)f.reset();for(auto&e:env)e.fill(0);phase=0;}
    void process(juce::AudioBuffer<float>&b,float amount,float detectedHz){if(amount<=.0001f)return;const int n=b.getNumSamples();sum.clear();const float hz=juce::jlimit(55.f,440.f,detectedHz>0?detectedHz:110.f);for(int i=0;i<n;i++){phase+=hz/sampleRate;if(phase>=1)phase-=1;float saw=(float)(phase*2-1),pulse=phase<.5?1.f:-1.f;float v=.65f*saw+.35f*pulse;for(int c=0;c<channels;c++)carrier.setSample(c,i,v);}
        for(int k=0;k<bands;k++){for(int c=0;c<channels;c++){modBand.copyFrom(c,0,b,c,0,n);carBand.copyFrom(c,0,carrier,c,0,n);}juce::dsp::AudioBlock<float> mb(modBand),cb(carBand);auto ms=mb.getSubBlock(0,(size_t)n),cs=cb.getSubBlock(0,(size_t)n);juce::dsp::ProcessContextReplacing<float>mc(ms),cc(cs);mod[(size_t)k].process(mc);car[(size_t)k].process(cc);for(int c=0;c<channels;c++){auto*m=modBand.getReadPointer(c);auto*ca=carBand.getReadPointer(c);auto*o=sum.getWritePointer(c);float&e=env[(size_t)k][(size_t)c];for(int i=0;i<n;i++){const float target=std::abs(m[i]);e+=(target-e)*(target>e?.09f:.008f);o[i]+=ca[i]*e*2.4f;}}}
        for(int c=0;c<channels;c++){auto*x=b.getWritePointer(c);auto*v=sum.getReadPointer(c);for(int i=0;i<n;i++)x[i]+=amount*(v[i]-x[i]);}}
private:static constexpr int bands=8;const std::array<float,bands>centres{120,220,400,720,1250,2200,3800,6500};std::array<juce::dsp::StateVariableTPTFilter<float>,bands>mod,car;std::array<std::array<float,2>,bands>env{};juce::AudioBuffer<float>carrier,modBand,carBand,sum;double sampleRate=44100,phase=0;int channels=2,maxBlock=512;
};

class VoiceEngine {
public:
    void prepare(double sr,int maximumBlock,int numChannels){sampleRate=sr;maxBlock=maximumBlock;channels=juce::jlimit(1,2,numChannels);tracker.prepare(sr);stretch.presetDefault(channels,sr,true);stretch.setFormantBase(150.0/sr);latency=stretch.inputLatency()+stretch.outputLatency();pitchBuffer.setSize(channels,maxBlock);dryBuffer.setSize(channels,maxBlock);radioBuffer.setSize(channels,maxBlock);dryDelaySize=latency+maxBlock+8;dryDelay.assign((size_t)channels,std::vector<float>((size_t)dryDelaySize,0));dryWrite=0;juce::dsp::ProcessSpec spec{sr,(juce::uint32)maximumBlock,(juce::uint32)channels};lowShelf.prepare(spec);presenceEq.prepare(spec);airShelf.prepare(spec);compressor.prepare(spec);limiter.prepare(spec);radioHp.prepare(spec);radioLp.prepare(spec);radioHp.setType(juce::dsp::StateVariableTPTFilterType::highpass);radioLp.setType(juce::dsp::StateVariableTPTFilterType::lowpass);reverb.prepare(spec);doublerFx.prepare(sr,channels);delayFx.prepare(sr,channels);vocoderFx.prepare(spec);reset();}
    void reset(){tracker.reset();stretch.reset();lowShelf.reset();presenceEq.reset();airShelf.reset();compressor.reset();limiter.reset();radioHp.reset();radioLp.reset();reverb.reset();doublerFx.reset();delayFx.reset();vocoderFx.reset();for(auto&c:dryDelay)std::fill(c.begin(),c.end(),0);dryWrite=0;smoothedCorrection=0;gateEnv=deEssEnv=deEssLow=0;activeMidi=-1;midiHold=0;}
    int getLatencySamples()const noexcept{return latency;}float getDetectedPitchHz()const noexcept{return lastPitchHz;}

    void process(juce::AudioBuffer<float>&b,juce::MidiBuffer&m,const DspSettings&s){const int n=b.getNumSamples();if(n<=0)return;for(int c=0;c<channels;c++)dryBuffer.copyFrom(c,0,b,c,0,n);const float inGain=juce::Decibels::decibelsToGain(s.inputDb);for(int c=0;c<channels;c++)b.applyGain(c,0,n,inGain);
        lastPitchHz=tracker.push(b.getReadPointer(0),n);const float midi=StablePitchDetector::hzToMidi(lastPitchHz);float correction=0;if(midi>0){const float target=nearestAllowed(midi,(int)std::round(s.key),(int)std::round(s.scale));correction=(target-midi)*s.tune;handleMidi(m,target,n,s.midiOut);}else handleMidi(m,-1,n,s.midiOut);correction+=s.pitch;const float tau=.008f+(1.f-s.tuneSpeed)*.22f;const float alpha=1.f-std::exp(-(float)n/(float)(sampleRate*tau));smoothedCorrection+=alpha*(correction-smoothedCorrection);
        applyGateAndDeEss(b,s.gate,s.deEss);stretch.setTransposeSemitones(smoothedCorrection,juce::jmin(.45,8000.0/sampleRate));stretch.setFormantFactor(std::pow(2.0,s.formant/12.0),true);stretch.setFormantBase(juce::jlimit(75.f,350.f,lastPitchHz>0?lastPitchHz:150.f)/sampleRate);stretch.process(b.getArrayOfReadPointers(),n,pitchBuffer.getArrayOfWritePointers(),n);for(int c=0;c<channels;c++)b.copyFrom(c,0,pitchBuffer,c,0,n);
        updateEq(s);juce::dsp::AudioBlock<float>block(b);auto sub=block.getSubBlock(0,(size_t)n);juce::dsp::ProcessContextReplacing<float>ctx(sub);lowShelf.process(ctx);presenceEq.process(ctx);airShelf.process(ctx);compressor.setThreshold(-8.f-s.compression*24.f);compressor.setRatio(1.f+s.compression*9.f);compressor.setAttack(4.f+s.compression*10.f);compressor.setRelease(65.f+s.compression*120.f);compressor.process(ctx);
        for(int c=0;c<channels;c++){auto*x=b.getWritePointer(c);for(int i=0;i<n;i++)x[i]=std::tanh(x[i]*(1.f+s.drive*8.f))/(1.f+s.drive*.9f);}applyRadio(b,s.radio,n);vocoderFx.process(b,s.vocoder,lastPitchHz);doublerFx.process(b,s.doubler);applyWidth(b,s.width);delayFx.process(b,s.delayMix,s.delayMs,s.delayFeedback);juce::dsp::Reverb::Parameters rp;rp.roomSize=.32f+s.reverb*.55f;rp.damping=.45f;rp.wetLevel=s.reverb*.42f;rp.dryLevel=1.f;rp.width=1.f;reverb.setParameters(rp);reverb.process(ctx);
        mixWithLatencyCompensatedDry(b,s.mix,n);b.applyGain(juce::Decibels::decibelsToGain(s.outputDb));limiter.setThreshold(-.35f);limiter.setRelease(60.f);limiter.process(ctx);}
private:
    using IIR=juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,juce::dsp::IIR::Coefficients<float>>;
    static float nearestAllowed(float midi,int key,int scale){static constexpr int major[7]{0,2,4,5,7,9,11},minor[7]{0,2,3,5,7,8,10};if(scale==0)return std::round(midi);float best=std::round(midi),dist=99;const int*notes=scale==1?major:minor;const int center=(int)std::round(midi);for(int n=center-12;n<=center+12;n++){int pc=((n-key)%12+12)%12;bool ok=false;for(int i=0;i<7;i++)if(pc==notes[i])ok=true;if(ok&&std::abs(n-midi)<dist){best=(float)n;dist=std::abs(n-midi);}}return best;}
    void handleMidi(juce::MidiBuffer&m,float target,int block,bool enabled){const int note=target>=0?juce::jlimit(0,127,(int)std::round(target)):-1;if(!enabled){if(activeMidi>=0)m.addEvent(juce::MidiMessage::noteOff(1,activeMidi),0);activeMidi=-1;return;}if(note>=0){midiHold=0;if(note!=activeMidi){if(activeMidi>=0)m.addEvent(juce::MidiMessage::noteOff(1,activeMidi),0);m.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)105),0);activeMidi=note;}}else if(activeMidi>=0&&(midiHold+=block)>(int)(sampleRate*.12)){m.addEvent(juce::MidiMessage::noteOff(1,activeMidi),0);activeMidi=-1;midiHold=0;}}
    void applyGateAndDeEss(juce::AudioBuffer<float>&b,float gate,float deEss){const float threshold=juce::Decibels::decibelsToGain(-62.f+gate*34.f);const float att=std::exp(-1.f/(float)(sampleRate*.003)),rel=std::exp(-1.f/(float)(sampleRate*.09));for(int i=0;i<b.getNumSamples();i++){float mono=0;for(int c=0;c<channels;c++)mono+=std::abs(b.getSample(c,i))/channels;gateEnv=(mono>gateEnv?att:rel)*gateEnv+(1-(mono>gateEnv?att:rel))*mono;float gateGain=gateEnv>threshold?1.f:juce::jlimit(.05f,1.f,gateEnv/juce::jmax(threshold,1e-6f));deEssLow+=.08f*(mono-deEssLow);float high=std::abs(mono-deEssLow);deEssEnv+=((high>deEssEnv)?.11f:.012f)*(high-deEssEnv);float ds=1.f-juce::jlimit(0.f,.72f,deEss*deEssEnv*9.f);for(int c=0;c<channels;c++)b.setSample(c,i,b.getSample(c,i)*gateGain*ds);}}
    void updateEq(const DspSettings&s){lowShelf.state=juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate,180,.75f,juce::Decibels::decibelsToGain(s.warmth));presenceEq.state=juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,3200,.8f,juce::Decibels::decibelsToGain(s.presence));airShelf.state=juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate,8500,.72f,juce::Decibels::decibelsToGain(s.air));}
    void applyRadio(juce::AudioBuffer<float>&b,float amount,int n){if(amount<=.0001f)return;for(int c=0;c<channels;c++)radioBuffer.copyFrom(c,0,b,c,0,n);radioHp.setCutoffFrequency(140.f+amount*360.f);radioLp.setCutoffFrequency(18000.f-amount*14200.f);juce::dsp::AudioBlock<float>rb(radioBuffer);auto sub=rb.getSubBlock(0,(size_t)n);juce::dsp::ProcessContextReplacing<float>rc(sub);radioHp.process(rc);radioLp.process(rc);for(int c=0;c<channels;c++){auto*x=b.getWritePointer(c);auto*r=radioBuffer.getReadPointer(c);for(int i=0;i<n;i++){float v=std::tanh(r[i]*(1.f+amount*4.f));x[i]+=amount*(v-x[i]);}}}
    void applyWidth(juce::AudioBuffer<float>&b,float width){if(channels<2)return;auto*l=b.getWritePointer(0),*r=b.getWritePointer(1);for(int i=0;i<b.getNumSamples();i++){float mid=.5f*(l[i]+r[i]),side=.5f*(l[i]-r[i])*width;l[i]=mid+side;r[i]=mid-side;}}
    void mixWithLatencyCompensatedDry(juce::AudioBuffer<float>&b,float mix,int n){for(int i=0;i<n;i++){for(int c=0;c<channels;c++){auto&d=dryDelay[(size_t)c];int read=(dryWrite+dryDelaySize-latency)%dryDelaySize;float dry=d[(size_t)read];d[(size_t)dryWrite]=dryBuffer.getSample(c,i);float wet=b.getSample(c,i);b.setSample(c,i,dry+mix*(wet-dry));}dryWrite=(dryWrite+1)%dryDelaySize;}}
    double sampleRate=44100;int maxBlock=512,channels=2,latency=0,dryDelaySize=1,dryWrite=0,activeMidi=-1,midiHold=0;float lastPitchHz=0,smoothedCorrection=0,gateEnv=0,deEssEnv=0,deEssLow=0;StablePitchDetector tracker;signalsmith::stretch::SignalsmithStretch<float>stretch;juce::AudioBuffer<float>pitchBuffer,dryBuffer,radioBuffer;std::vector<std::vector<float>>dryDelay;IIR lowShelf,presenceEq,airShelf;juce::dsp::Compressor<float>compressor;juce::dsp::Limiter<float>limiter;juce::dsp::StateVariableTPTFilter<float>radioHp,radioLp;juce::dsp::Reverb reverb;ModulatedDoubler doublerFx;StereoDelay delayFx;SimpleVocoder vocoderFx;
};

