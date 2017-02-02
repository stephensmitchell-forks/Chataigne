/*
  ==============================================================================

    ModuleEditor.cpp
    Created: 28 Dec 2016 1:37:59pm
    Author:  Ben

  ==============================================================================
*/

#include "ModuleEditor.h"
#include "AssetManager.h"

ModuleEditor::ModuleEditor(Module * _module, bool isRoot) :
	BaseItemEditor(_module,isRoot),
	module(_module)
{
	logIncomingUI = module->logIncomingData->createToggle();
	logOutgoingUI = module->logOutgoingData->createToggle();

	addAndMakeVisible(logIncomingUI);
	addAndMakeVisible(logOutgoingUI);

}

ModuleEditor::~ModuleEditor()
{
}

void ModuleEditor::resizedInternalHeader(Rectangle<int>& r)
{
	logOutgoingUI->setBounds(r.removeFromRight(60));
	r.removeFromRight(2);
	logIncomingUI->setBounds(r.removeFromRight(60));
	r.reduce(5, 0);

}

void ModuleEditor::controllableFeedbackAsyncUpdate(Controllable * c)
{
	BaseItemEditor::controllableFeedbackAsyncUpdate(c);
	if (c == module->logIncomingData)
	{

	} else if (c == module->logOutgoingData)
	{

	}
}