/*
  ==============================================================================

    GenericOSCQueryModule.h
    Created: 28 Feb 2019 10:33:17pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "Module/Module.h"


class GenericOSCQueryValueContainer :
	public ControllableContainer
{
public:
	GenericOSCQueryValueContainer(const String &name);
	~GenericOSCQueryValueContainer();

	BoolParameter* enableListen;

	InspectableEditor* getEditor(bool isRoot) override;
};

class GenericOSCQueryModule;
class OSCQueryOutput :
	public EnablingControllableContainer
{
public:
	OSCQueryOutput(GenericOSCQueryModule * module);
	~OSCQueryOutput();

	GenericOSCQueryModule * module;
	InspectableEditor * getEditor(bool isRoot) override;
};

class GenericOSCQueryModule :
	public Module,
	public SimpleWebSocketClient::Listener,
	public Thread
{
public:
	GenericOSCQueryModule(const String &name = "OSCQuery",int defaultRemotePort = 5678);
	virtual ~GenericOSCQueryModule();

	const Identifier dataStructureEventId = "dataStructureEvent";

	Trigger * syncTrigger;
	BoolParameter* keepValuesOnSync;
	StringParameter* serverName;
	Trigger* listenAllTrigger;

	std::unique_ptr<OSCQueryOutput> sendCC;
	BoolParameter * useLocal;
	StringParameter * remoteHost;
	IntParameter * remotePort;
	IntParameter* remoteOSCPort;
	

	OSCSender sender;
	std::unique_ptr<SimpleWebSocketClient> wsClient;
	bool hasListenExtension;
	var treeData; //to keep on save


	void setupWSClient();

	void sendOSCMessage(OSCMessage m);
	void sendOSCForControllable(Controllable * c);
	
	//Script
	static var sendOSCFromScript(const var::NativeFunctionArgs &args);
	
	static OSCArgument varToArgument(const var &v);

	virtual void syncData();
	virtual void createTreeFromData(var data);
	virtual void fillContainerFromData(ControllableContainer * cc, var data);
	virtual Controllable * createControllableFromData(StringRef name, var data);

	void updateListenToContainer(GenericOSCQueryValueContainer* gcc);

	virtual void onControllableFeedbackUpdateInternal(ControllableContainer * cc, Controllable * c) override;

	void connectionOpened() override;
	void connectionClosed(int status, const String& reason) override;
	void connectionError(const String& errorMessage) override;
	
	void dataReceived(const MemoryBlock& data) override;
	void messageReceived(const String& message) override;


	var getJSONData() override;
	void loadJSONDataInternal(var data) override;
	void afterLoadJSONDataInternal() override;

	// Inherited via Thread
	virtual void run() override;
	virtual void requestHostInfo();
	virtual void requestStructure();

	//Routing
	class OSCQueryRouteParams :
		public RouteParams,
		public Inspectable::InspectableListener
	{
	public:
		OSCQueryRouteParams(GenericOSCQueryModule* outModule, Module* sourceModule, Controllable* c);
		~OSCQueryRouteParams();

		TargetParameter* target;
		WeakReference<Controllable> cRef;

		void setControllable(Controllable* c);

		void onContainerParameterChanged(Parameter* p) override;

		void inspectableDestroyed(Inspectable * i) override;
	};

	virtual RouteParams* createRouteParamsForSourceValue(Module* sourceModule, Controllable* c, int /*index*/) override { return new OSCQueryRouteParams(this, sourceModule, c); }
	virtual void handleRoutedModuleValue(Controllable* c, RouteParams* p) override;
	

	static GenericOSCQueryModule* create() { return new GenericOSCQueryModule(); }
	virtual String getDefaultTypeString() const override { return "OSCQuery"; }

};