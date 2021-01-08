/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SvfAudioProcessor::SvfAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ), tree(*this, nullptr)
#endif
{
    NormalisableRange<float> cuttoffRange (20.0f, 20000.f);
    NormalisableRange<float> resRange (1.0f, 5.0f);
    NormalisableRange<float> filterMenuRange (0, 2);
    
    tree.createAndAddParameter("cutoff", "Cutoff", "cutoff", cuttoffRange, 600.0f, nullptr, nullptr);
    tree.createAndAddParameter("resonance", "Resonance", "resonance", resRange, 1.0f, nullptr, nullptr);
    tree.createAndAddParameter("filterMenu", "FilterMenu", "filterMenu", filterMenuRange, 0, nullptr, nullptr);
}

SvfAudioProcessor::~SvfAudioProcessor()
{
}

//==============================================================================
const String SvfAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SvfAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SvfAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SvfAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SvfAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SvfAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SvfAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SvfAudioProcessor::setCurrentProgram (int index)
{
}

const String SvfAudioProcessor::getProgramName (int index)
{
    return {};
}

void SvfAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SvfAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    lastSampleRate = sampleRate;
    
    dsp::ProcessSpec spec;
    spec.sampleRate = lastSampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getMainBusNumOutputChannels();
    
    stateVariableFilter.reset();
    updateFilter();
    stateVariableFilter.prepare(spec);
}

void SvfAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void SvfAudioProcessor::updateFilter()
{
    int menuChoice = *tree.getRawParameterValue("filterMenu");
    int cutOff = *tree.getRawParameterValue("cutoff");
    int resonance = *tree.getRawParameterValue("resonance");
    
    if (menuChoice == 0)
    {
        stateVariableFilter.state->type = dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
        
        stateVariableFilter.state->setCutOffFrequency(lastSampleRate, cutOff, resonance);
    }
    else if (menuChoice == 1)
    {
        stateVariableFilter.state->type = dsp::StateVariableFilter::Parameters<float>::Type::bandPass;
        
        stateVariableFilter.state->setCutOffFrequency(lastSampleRate, cutOff, resonance);
    }
    else if (menuChoice == 2)
    {
        stateVariableFilter.state->type = dsp::StateVariableFilter::Parameters<float>::Type::highPass;
        
        stateVariableFilter.state->setCutOffFrequency(lastSampleRate, cutOff, resonance);
    }
    
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SvfAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SvfAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    dsp::AudioBlock<float> block (buffer);
    
    stateVariableFilter.process(dsp::ProcessContextReplacing<float> (block));
    
    updateFilter();
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    /*for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        // ..do something to the data...
    }*/
}

//==============================================================================
bool SvfAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SvfAudioProcessor::createEditor()
{
    return new SvfAudioProcessorEditor (*this);
}

//==============================================================================
void SvfAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SvfAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SvfAudioProcessor();
}
