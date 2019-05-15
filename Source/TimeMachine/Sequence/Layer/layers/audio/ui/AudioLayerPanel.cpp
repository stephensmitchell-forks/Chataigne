/*
  ==============================================================================

    AudioLayerPanel.cpp
    Created: 20 Nov 2016 3:08:49pm
    Author:  Ben Kuper

  ==============================================================================
*/

#include "AudioLayerPanel.h"

AudioLayerPanel::AudioLayerPanel(AudioLayer * layer) :
	SequenceLayerPanel(layer),
	audioLayer(layer)
{
	
	moduleChooser.setTextWhenNoChoicesAvailable("No audio module");
	moduleChooser.setTextWhenNothingSelected("Choose an audio module");
	
	moduleChooser.filterModuleFunc = &AudioLayerPanel::isAudioModule;
	moduleChooser.buildModuleBox();

	moduleChooser.addChooserListener(this);
	moduleChooser.setModuleSelected(audioLayer->audioModule,true);

	addAndMakeVisible(&moduleChooser);

	audioLayer->addAudioLayerListener(this);

	enveloppeUI.reset(audioLayer->enveloppe->createSlider());
	addAndMakeVisible(enveloppeUI.get());

}

AudioLayerPanel::~AudioLayerPanel()
{
	if (!inspectable.wasObjectDeleted()) audioLayer->removeAudioLayerListener(this);
}


void AudioLayerPanel::resizedInternalContent(Rectangle<int>& r)
{
	SequenceLayerPanel::resizedInternalContent(r);  
	Rectangle<int> gr = r.reduced(2).removeFromTop(16);

	moduleChooser.setBounds(gr.removeFromLeft(80));
	gr.removeFromLeft(5);
	enveloppeUI->setBounds(gr);
}

void AudioLayerPanel::targetAudioModuleChanged(AudioLayer *)
{
	moduleChooser.setModuleSelected(audioLayer->audioModule,true);
}

void AudioLayerPanel::selectedModuleChanged(ModuleChooserUI *, Module * m)
{
	audioLayer->setAudioModule(dynamic_cast<AudioModule *>(m));
}

void AudioLayerPanel::moduleListChanged(ModuleChooserUI *)
{
	moduleChooser.setModuleSelected(audioLayer->audioModule, true);

}
