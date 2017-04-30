/*
  ==============================================================================

    ModuleRouter.h
    Created: 29 Apr 2017 10:35:18am
    Author:  Ben

  ==============================================================================
*/

#ifndef MODULEROUTER_H_INCLUDED
#define MODULEROUTER_H_INCLUDED

#include "BaseItem.h"
#include "Module.h"
#include "ModuleRouterValue.h"

class ModuleRouter :
	public BaseItem,
	public Inspectable::InspectableListener
{
public:
	ModuleRouter();
	~ModuleRouter();

	Module * sourceModule;
	Module * destModule;

	BaseManager<ModuleRouterValue> sourceValues;

	void setSourceModule(Module * m);
	void setDestModule(Module * m);

	void setSourceValueSelected(Controllable * sourceValue, bool selected);
	void clearSourceValues();

	void inspectableDestroyed(Inspectable * m) override;
	void onExternalParameterChanged(Parameter * p) override;
	void onExternalTriggerTriggered(Trigger * t) override;

	class RouterListener
	{
	public:
		virtual ~RouterListener() {}
		virtual void sourceModuleChanged(ModuleRouter *) {}
		virtual void destModuleChanged(ModuleRouter *) {}
	};

	ListenerList<RouterListener> routerListeners;
	void addRouterListener(RouterListener* newListener) { routerListeners.add(newListener); }
	void removeRouterListener(RouterListener* listener) { routerListeners.remove(listener); }
};

#endif  // MODULEROUTER_H_INCLUDED