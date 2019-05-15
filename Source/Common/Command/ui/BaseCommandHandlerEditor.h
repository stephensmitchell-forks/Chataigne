/*
  ==============================================================================

    BaseCommandHandlerEditor.h
    Created: 19 Jan 2017 6:41:22pm
    Author:  Ben

  ==============================================================================
*/

#ifndef BASECOMMANDHANDLEREDITOR_H_INCLUDED
#define BASECOMMANDHANDLEREDITOR_H_INCLUDED


#include "Common/Processor/Action/Consequence/Consequence.h"
#include "CommandChooserUI.h"

class BaseCommandHandlerEditor :
	public BaseItemEditor,
	public CommandChooserUI::Listener,
	public BaseCommandHandler::AsyncListener
{
public:
	BaseCommandHandlerEditor(BaseCommandHandler *, bool isRoot);
	virtual ~BaseCommandHandlerEditor();

	BaseCommandHandler * handler;

	CommandChooserUI chooser;
	std::unique_ptr<TriggerButtonUI> triggerBT;

	void resizedInternalHeaderItemInternal(Rectangle<int> &r) override;

	void updateChooserLabel();

	void definitionChosen(CommandDefinition * d) override;
	void newMessage(const BaseCommandHandler::CommandHandlerEvent &e) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseCommandHandlerEditor)
};


#endif  // BASECOMMANDHANDLEREDITOR_H_INCLUDED
