#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetBank.h"

UCGVoiceWonderProcessor::UCGVoiceWonderProcessor()
  : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
    apvts(*this,nullptr,"UCGVoiceWonderState",createLayout()) {
    applyFactoryPreset(apvts,0);
}

juce::AudioProcessorValueTreeState::ParameterLayout UCGVoiceWonderProcessor::createLayout() {
    using F=juce::AudioParameterFloat; using B=juce::AudioParameterBool; using C=juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add=[&](const char*id,const char*name,float lo,float hi,float step,float def,const char*suffix=""){p.push_back(std::make_unique<F>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi,step),def,F::Attributes{}.withStringFromValueFunction([suffix](float v,int){return juce::String(v,2)+suffix;})));};
    add("mix","Mix",0,1,.001f,1); add("input","Input",-18,18,.01f,0," dB"); add("gate","Gate",0,1,.001f,.12f);
    add("tune","Tune Amount",0,1,.001f,.35f); add("tunespeed","Tune Speed",0,1,.001f,.35f);
    p.push_back(std::make_unique<C>(juce::ParameterID{"key",1},"Key",juce::StringArray{"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"},0));
    p.push_back(std::make_unique<C>(juce::ParameterID{"scale",1},"Scale",juce::StringArray{"Chromatic","Major","Minor"},0));
    add("pitch","Pitch",-12,12,.01f,0," st"); add("formant","Formant",-6,6,.01f,0," st");
    add("warmth","Warmth",-8,8,.01f,0," dB"); add("presence","Presence",-8,8,.01f,0," dB"); add("air","Air",-8,8,.01f,0," dB");
    add("deess","De-Esser",0,1,.001f,.25f); add("compress","Compression",0,1,.001f,.35f); add("drive","Drive",0,1,.001f,0);
    add("radio","Radio",0,1,.001f,0); add("vocoder","Vocoder",0,1,.001f,0); add("doubler","Doubler",0,1,.001f,0);
    add("width","Width",0,2,.001f,1); add("delay","Delay Mix",0,1,.001f,0); add("delaytime","Delay Time",50,750,1,280," ms");
    add("delayfeedback","Delay Feedback",0,.78f,.001f,.22f); add("reverb","Reverb",0,1,.001f,.08f); add("output","Output",-18,12,.01f,0," dB");
    p.push_back(std::make_unique<B>(juce::ParameterID{"midi",1},"Voice to MIDI",true));
    return {p.begin(),p.end()};
}

void UCGVoiceWonderProcessor::prepareToPlay(double sr,int block) {
    engine.prepare(sr,juce::jmax(64,block),getTotalNumOutputChannels());
    setLatencySamples(engine.getLatencySamples());
}

bool UCGVoiceWonderProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto out=l.getMainOutputChannelSet(); return l.getMainInputChannelSet()==out&&(out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo());
}

DspSettings UCGVoiceWonderProcessor::readSettings() const {
    auto v=[this](const char*id){return apvts.getRawParameterValue(id)->load();}; DspSettings s;
    s.mix=v("mix");s.inputDb=v("input");s.gate=v("gate");s.tune=v("tune");s.tuneSpeed=v("tunespeed");s.key=v("key");s.scale=v("scale");s.pitch=v("pitch");s.formant=v("formant");s.warmth=v("warmth");s.presence=v("presence");s.air=v("air");s.deEss=v("deess");s.compression=v("compress");s.drive=v("drive");s.radio=v("radio");s.vocoder=v("vocoder");s.doubler=v("doubler");s.width=v("width");s.delayMix=v("delay");s.delayMs=v("delaytime");s.delayFeedback=v("delayfeedback");s.reverb=v("reverb");s.outputDb=v("output");s.midiOut=v("midi")>.5f;return s;
}

void UCGVoiceWonderProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&m) {
    juce::ScopedNoDenormals noDenormals; for(int c=getTotalNumInputChannels();c<getTotalNumOutputChannels();c++)b.clear(c,0,b.getNumSamples());engine.process(b,m,readSettings());detectedPitch.store(engine.getDetectedPitchHz());
}

int UCGVoiceWonderProcessor::getNumPrograms(){return (int)getFactoryPresets().size();}
int UCGVoiceWonderProcessor::getCurrentProgram(){return currentProgram;}
void UCGVoiceWonderProcessor::setCurrentProgram(int i){applyPreset(i);}
const juce::String UCGVoiceWonderProcessor::getProgramName(int i){const auto&p=getFactoryPresets();return juce::isPositiveAndBelow(i,(int)p.size())?p[(size_t)i].name:juce::String();}
juce::StringArray UCGVoiceWonderProcessor::getPresetNames()const{juce::StringArray n;for(const auto&p:getFactoryPresets())n.add(p.name);return n;}
void UCGVoiceWonderProcessor::applyPreset(int i){if(!juce::isPositiveAndBelow(i,(int)getFactoryPresets().size()))return;applyFactoryPreset(apvts,i);currentProgram=i;}

void UCGVoiceWonderProcessor::applyReferenceProfile(const VocalReferenceProfile&p){if(!p.valid)return;setParameterPlain(apvts,"warmth",p.warmthDb);setParameterPlain(apvts,"presence",p.presenceDb);setParameterPlain(apvts,"air",p.airDb);setParameterPlain(apvts,"formant",p.formantSemitones);setParameterPlain(apvts,"compress",p.compression);setParameterPlain(apvts,"deess",p.deEss);setParameterPlain(apvts,"tune",.48f);setParameterPlain(apvts,"tunespeed",.42f);apvts.state.setProperty("presetName","MATCH: "+p.name,nullptr);apvts.state.setProperty("referenceProfile",juce::JSON::toString(p.toVar()),nullptr);const juce::ScopedLock l(profileLock);profileSummary=p.summary();}

bool UCGVoiceWonderProcessor::saveUserPreset(const juce::File&f)const{auto state=apvts.copyState();auto*o=new juce::DynamicObject();o->setProperty("format","UCG Voice Wonder preset");o->setProperty("version",1);o->setProperty("name",f.getFileNameWithoutExtension());o->setProperty("state",state.toXmlString());return f.replaceWithText(juce::JSON::toString(juce::var(o),true));}
bool UCGVoiceWonderProcessor::loadUserPreset(const juce::File&f){const auto parsed=juce::JSON::parse(f);auto*o=parsed.getDynamicObject();if(o==nullptr)return false;auto xml=juce::parseXML(o->getProperty("state").toString());if(xml==nullptr)return false;auto state=juce::ValueTree::fromXml(*xml);if(!state.isValid())return false;apvts.replaceState(state);return true;}

void UCGVoiceWonderProcessor::getStateInformation(juce::MemoryBlock&d){auto x=apvts.copyState().createXml();copyXmlToBinary(*x,d);}
void UCGVoiceWonderProcessor::setStateInformation(const void*d,int n){if(auto x=getXmlFromBinary(d,n))apvts.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessorEditor* UCGVoiceWonderProcessor::createEditor(){return new UCGVoiceWonderEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new UCGVoiceWonderProcessor();}
