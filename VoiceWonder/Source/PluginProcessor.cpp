#include "PluginProcessor.h"
#include "PluginEditor.h"
UCGVoiceWonderProcessor::UCGVoiceWonderProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),apvts(*this,nullptr,"PARAMS",layout()){}
juce::AudioProcessorValueTreeState::ParameterLayout UCGVoiceWonderProcessor::layout(){
    using P=juce::AudioParameterFloat; std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto add=[&](const char* id,const char* name,float lo,float hi,float def){p.push_back(std::make_unique<P>(id,name,juce::NormalisableRange<float>(lo,hi,0.001f),def));};
    add("mix","Mix",0,1,1);add("pitch","Pitch",-12,12,0);add("correct","Correction",0,1,.65f);add("radio","Radio",0,1,0);add("vocoder","Vocoder",0,1,0);add("drive","Drive",0,1,0);add("low","Warmth",-1,1,0);add("high","Air",-1,1,0);p.push_back(std::make_unique<juce::AudioParameterBool>("midi","Voice to MIDI",true)); return {p.begin(),p.end()};}
void UCGVoiceWonderProcessor::prepareToPlay(double sr,int bs){engine.prepare({sr,(juce::uint32)bs,(juce::uint32)getTotalNumOutputChannels()});}
bool UCGVoiceWonderProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainInputChannelSet()==l.getMainOutputChannelSet()&&(l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());}
void UCGVoiceWonderProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& m){juce::ScopedNoDenormals n; auto f=[&](const char*s){return apvts.getRawParameterValue(s)->load();}; engine.process(b,m,f("mix"),f("pitch"),f("correct"),f("radio"),f("vocoder"),f("drive"),f("low"),f("high"),f("midi")>.5f);}
juce::AudioProcessorEditor* UCGVoiceWonderProcessor::createEditor(){return new UCGVoiceWonderEditor(*this);}
void UCGVoiceWonderProcessor::getStateInformation(juce::MemoryBlock& d){auto x=apvts.copyState().createXml();copyXmlToBinary(*x,d);} void UCGVoiceWonderProcessor::setStateInformation(const void*d,int n){if(auto x=getXmlFromBinary(d,n))apvts.replaceState(juce::ValueTree::fromXml(*x));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new UCGVoiceWonderProcessor();}
