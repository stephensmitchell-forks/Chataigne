/*
  ==============================================================================

    TimeTriggerManagerUI.cpp
    Created: 10 Dec 2016 12:23:33pm
    Author:  Ben

  ==============================================================================
*/

#include "TimeTriggerManagerUI.h"
#include "TriggerLayerTimeline.h"
#include "../../../../Cue/ui/TimeCueManagerUI.h"
#include "StateMachine/StateManager.h"
#include "Common/Processor/Action/ui/ActionUI.h"
#include "Module/ui/ModuleUI.h"
#include "Common/Command/Template/CommandTemplate.h"

TimeTriggerManagerUI::TimeTriggerManagerUI(TriggerLayerTimeline * _timeline, TimeTriggerManager * manager) :
	BaseManagerUI("Triggers", manager, false),
	timeline(_timeline)
{
	addItemText = "Add Trigger";
	animateItemOnAdd = false;
	transparentBG = true;

	acceptedDropTypes.add("Action");
	acceptedDropTypes.add("CommandTemplate");
	acceptedDropTypes.add("Module");

	addItemBT->setVisible(false);

	noItemText = "To add triggers, double click here";

	manager->selectionManager->addSelectionListener(this);

	resizeOnChildBoundsChanged = false;
	addExistingItems();
}

TimeTriggerManagerUI::~TimeTriggerManagerUI()
{
	manager->selectionManager->removeSelectionListener(this);
	if (InspectableSelector::getInstanceWithoutCreating()) InspectableSelector::getInstance()->removeSelectorListener(this);
}

void TimeTriggerManagerUI::paint(Graphics & g)
{
	BaseManagerUI::paint(g);
	if (isDraggingOver)
	{
		g.setColour(BLUE_COLOR);
		Line<int> line(getMouseXYRelative().withY(0), getMouseXYRelative().withY(getHeight()));
		g.drawLine(line.toFloat(), 2);
	}
}

void TimeTriggerManagerUI::resized()
{
	updateContent();
	if (transformer != nullptr) transformer->updateBoundsFromKeys();
}

void TimeTriggerManagerUI::updateContent()
{
	for (auto &ttui : itemsUI)
	{
		placeTimeTriggerUI(ttui);
		if(ttui->item->isSelected) ttui->toFront(true);
		else ttui->toBack();
	}

	
}

void TimeTriggerManagerUI::placeTimeTriggerUI(TimeTriggerUI * ttui)
{
	int tx = timeline->getXForTime(ttui->item->time->floatValue());
	
	float ttuiWidthTime = timeline->getTimeForX(ttui->getWidth(),false);
	//DBG(ttuiWidthTime << "/" << timeline->item->sequence->totalTime->floatValue());

	float viewEnd = timeline->item->sequence->viewEndTime->floatValue();
	float totalTime = timeline->item->sequence->totalTime->floatValue();
	
	float ttuiTime = totalTime - ttuiWidthTime;

	if (viewEnd >= ttuiTime && ttuiTime < totalTime)
	{
		float rel = jmap<float>(viewEnd, ttuiTime, totalTime, 0, 1);
		int minTx = getWidth() - ttui->getWidth() * rel;
		int safeTx = jmin(tx, minTx); 
		ttui->flagXOffset = tx - safeTx;
		tx = safeTx;
	} else
	{
		ttui->flagXOffset = 0;
	}

	ttui->setBounds(tx, 0, ttui->getWidth(), getHeight());
}

void TimeTriggerManagerUI::mouseDown(const MouseEvent & e)
{
	BaseManagerUI::mouseDown(e);

	if (e.eventComponent == this)
	{
		if (e.mods.isLeftButtonDown())
		{
			if (e.mods.isAltDown())
			{
				float time = timeline->getTimeForX(getMouseXYRelative().x);
				manager->addTriggerAt(time, getMouseXYRelative().y*1.f / getHeight());
			} else
			{
				Array<Component *> selectables;
				Array<Inspectable *> inspectables;
				for (auto &i : itemsUI) if (i->isVisible())
				{
					selectables.add(i);
					inspectables.add(i->inspectable);
				}

				if (transformer != nullptr)
				{
					removeChildComponent(transformer.get());
					transformer = nullptr;
				}

				if (InspectableSelector::getInstance())
				{
					InspectableSelector::getInstance()->startSelection(this, selectables, inspectables, manager->selectionManager, !e.mods.isCommandDown());
					InspectableSelector::getInstance()->addSelectorListener(this);
				}
			}
		}
	}
}

void TimeTriggerManagerUI::mouseDoubleClick(const MouseEvent & e)
{
	float time = timeline->getTimeForX(getMouseXYRelative().x);
	manager->addTriggerAt(time, getMouseXYRelative().y*1.f / getHeight());
}



void TimeTriggerManagerUI::addItemFromMenu(bool isFromAddButton, Point<int> mouseDownPos)
{
	if (isFromAddButton) return;

	float time = timeline->getTimeForX(mouseDownPos.x);
	manager->addTriggerAt(time, mouseDownPos.y*1.f / getHeight());
}
void TimeTriggerManagerUI::addItemUIInternal(TimeTriggerUI * ttui)
{
	ttui->addTriggerUIListener(this);
}

void TimeTriggerManagerUI::removeItemUIInternal(TimeTriggerUI * ttui)
{
	if (transformer != nullptr)
	{
		removeChildComponent(transformer.get());
		transformer = nullptr;
	}

	ttui->removeTriggerUIListener(this);
}

void TimeTriggerManagerUI::timeTriggerDragged(TimeTriggerUI * ttui, const MouseEvent & e)
{
	float targetTime = ttui->timeAtMouseDown + timeline->getTimeForX(e.getOffsetFromDragStart().x, false);
	if (e.mods.isShiftDown()) targetTime = timeline->item->sequence->cueManager->getNearestCueForTime(targetTime);
	ttui->item->time->setValue(targetTime);
	repaint();
	
}

void TimeTriggerManagerUI::timeTriggerTimeChanged(TimeTriggerUI * ttui)
{
	placeTimeTriggerUI(ttui);
}

void TimeTriggerManagerUI::itemDropped(const SourceDetails & details)
{
	isDraggingOver = false;
	
	String dataType = details.description.getProperty("dataType", "");
	CommandDefinition * def = nullptr;

	if (dataType == "Action") def = StateManager::getInstance()->module.getCommandDefinitionFor("Action", "Trigger Action");
	else if (dataType == "Module")
	{
		ModuleUI * mui = dynamic_cast<ModuleUI *>(details.sourceComponent.get());
		if (mui != nullptr)
		{
			PopupMenu p = mui->item->getCommandMenu(0, CommandContext::ACTION);
			int result = p.show();
			if (result > 0) def= mui->item->getCommandDefinitionForItemID(result - 1);
		}
	}
	else if (dataType == "CommandTemplate")
	{
		BaseItemUI<CommandTemplate> * tui = dynamic_cast<BaseItemUI<CommandTemplate> *>(details.sourceComponent.get());
		if (tui != nullptr)
		{
			CommandTemplateManager * ctm = dynamic_cast<CommandTemplateManager *>(tui->item->parentContainer.get());
			if(ctm != nullptr) def = ctm->defManager.getCommandDefinitionFor(ctm->menuName, tui->item->niceName);
		}
	}

	BaseCommand * command = nullptr;

	TimeTrigger * t = nullptr;

	if (def != nullptr)
	{
		float time = timeline->getTimeForX(getMouseXYRelative().x);
		t = manager->addTriggerAt(time, getMouseXYRelative().y * 1.f / getHeight());
		Consequence * c = t->csmOn->addItem();
		c->setCommand(def);
		command = c->command.get();
	}

	if (command != nullptr)
	{
		if (dataType == "Action")
		{
			StateCommand * sc = dynamic_cast<StateCommand *>(command);
			ActionUI * bui = dynamic_cast<ActionUI *>(details.sourceComponent.get());
			if (bui != nullptr)
			{
				sc->target->setValueFromTarget(bui->item);
				t->setNiceName(bui->item->niceName);
			}
		}
	}
}

void TimeTriggerManagerUI::selectionEnded(Array<Component*> selectedComponents)
{
	if(InspectableSelector::getInstanceWithoutCreating()) InspectableSelector::getInstance()->removeSelectorListener(this);
	if (selectedComponents.size() == 0) timeline->item->selectThis();
}

void TimeTriggerManagerUI::inspectablesSelectionChanged()
{
	if (transformer != nullptr)
	{
		removeChildComponent(transformer.get());
		transformer = nullptr;
	}

	Array<TimeTriggerUI *> uiSelection;
	if (manager->selectionManager->currentInspectables.size() >= 2)
	{

	}

	Array<TimeTrigger *> triggers = manager->selectionManager->getInspectablesAs<TimeTrigger>();
	for (auto &t : triggers)
	{
		TimeTriggerUI * kui = getUIForItem(t);
		if (kui == nullptr) return;

		uiSelection.add(kui);
	}

	if (uiSelection.size() >= 2)
	{
		transformer.reset(new TimeTriggerMultiTransformer(this, uiSelection));
		addAndMakeVisible(transformer.get());
		transformer->grabKeyboardFocus(); // so no specific key has the focus for deleting
	}
}
