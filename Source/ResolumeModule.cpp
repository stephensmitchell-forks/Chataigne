/*
  ==============================================================================

    ResolumeOutput.cpp
    Created: 29 Oct 2016 7:21:01pm
    Author:  bkupe

  ==============================================================================
*/

#include "ResolumeModule.h"
#include "OSCCommand.h"
#include "CommandFactory.h"

ResolumeModule::ResolumeModule() :
	OSCModule("Resolume")
{
	
	defManager.add(CommandDefinition::createDef(this, "Composition", "Stop Composition", &OSCCommand::create, CommandContext::ACTION)->addParam("address", "/composition/stop"));
	
}