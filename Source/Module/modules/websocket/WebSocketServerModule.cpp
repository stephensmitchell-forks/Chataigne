/*
  ==============================================================================

	WebSocketServerModule.cpp
	Created: 20 Jun 2020 3:10:02pm
	Author:  bkupe

  ==============================================================================
*/

#include "WebSocketServerModule.h"
#include "ui/WebSocketServerModuleUI.h"

WebSocketServerModule::WebSocketServerModule(const String& name, int defaultRemotePort) :
	StreamingModule(name)
{
	localPort = moduleParams.addIntParameter("Local Port", "Port to bind to listen to incoming data", defaultRemotePort, 1, 65535);
	isConnected = moduleParams.addBoolParameter("Connected", "Is the socket sucessfully bound and listening", false);
	isConnected->setControllableFeedbackOnly(true);

	numClients = moduleParams.addIntParameter("Connected Clients", "Number of connected clients", 0);
	numClients->setControllableFeedbackOnly(true);

	connectionFeedbackRef = isConnected;

	setupServer();
}

WebSocketServerModule::~WebSocketServerModule()
{
}

void WebSocketServerModule::setupServer()
{
	if (server != nullptr)
	{
		server->stop();
		server.reset();
	}

	if (isCurrentlyLoadingData) return;

	isConnected->setValue(false);
	numClients->intValue();

	if (!enabled->intValue()) return;

	server.reset(new SimpleWebSocketServer());
	server->addWebSocketListener(this);
	server->start(localPort->intValue());

	isConnected->setValue(true);

	NLOG(niceName, "Server is running on port " << localPort->intValue());
}

bool WebSocketServerModule::isReadyToSend()
{
	return server != nullptr && isConnected->boolValue();
}

void WebSocketServerModule::sendMessageInternal(const String& message, var)
{
	server->send(message);
}

void WebSocketServerModule::sendBytesInternal(Array<uint8> data, var)
{
	server->send((const char*)data.getRawDataPointer(), data.size());
}

void WebSocketServerModule::connectionOpened(const String& connectionId)
{
	NLOG(niceName, "Connection opened from : " << connectionId);
	numClients->setValue(server->getNumActiveConnections());
}

void WebSocketServerModule::connectionClosed(const String& connectionId, int status, const String& reason)
{
	NLOG(niceName, "Connection closed from : " << connectionId);
	numClients->setValue(server->getNumActiveConnections());
}

void WebSocketServerModule::connectionError(const String& connectionId, const String& errorMessage)
{
	if (enabled->boolValue()) NLOGERROR(niceName, "Connection error from : " << connectionId << " : " << errorMessage);
	
	numClients->setValue(server->getNumActiveConnections());
}

void WebSocketServerModule::messageReceived(const String& connectionId, const String& message)
{
	StreamingType t = streamingType->getValueDataAsEnum<StreamingType>();
	switch (t)
	{
	case LINES:
	{
		StringArray sa;
		sa.addTokens(message, "\n", "\"");
		for (auto& s : sa) processDataLine(s);
	}
	break;

	case TYPE_JSON:
		processDataJSON(JSON::fromString(message));
		break;

	default:
		DBG("Not handled");
		break;
	}
}

void WebSocketServerModule::onContainerParameterChangedInternal(Parameter* p)
{
	StreamingModule::onContainerParameterChangedInternal(p);

	if (p == enabled)
	{
		if (!enabled->boolValue() && isConnected->boolValue())
		{
			NLOG(niceName, "Disabling module, closign server.");
		}
		setupServer();
	}
}

void WebSocketServerModule::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	StreamingModule::onControllableFeedbackUpdateInternal(cc, c);
}


void WebSocketServerModule::createControllablesFromJSONResult(var data, ControllableContainer* container)
{
	if (!data.isObject()) return;

	DynamicObject* dataObject = data.getDynamicObject();
	if (dataObject == nullptr) return;

	NamedValueSet props = dataObject->getProperties();

	for (auto& p : props)
	{
		if (p.value.isArray())
		{
			for (int i = 0; i < p.value.size(); ++i)
			{
				ControllableContainer* cc = container->getControllableContainerByName(String(i), true);
				if (cc == nullptr)
				{
					cc = new ControllableContainer(String(i));
					container->addChildControllableContainer(cc, true);
					cc->userCanAddControllables = true;
					cc->saveAndLoadRecursiveData = true;
					cc->saveAndLoadName = true;
				}

				createControllablesFromJSONResult(p.value[i], cc);
			}

		}
		else if (p.value.isObject())
		{
			ControllableContainer* cc = container->getControllableContainerByName(p.name.toString(), true);
			if (cc == nullptr)
			{
				cc = new ControllableContainer(p.name.toString());
				container->addChildControllableContainer(cc, true);
				cc->userCanAddControllables = true;
				cc->saveAndLoadRecursiveData = true;
				cc->saveAndLoadName = true;
			}

			createControllablesFromJSONResult(p.value, cc);
		}
		else
		{
			Controllable* newC = container->getControllableByName(p.name.toString(), true);
			if (newC == nullptr)
			{
				if (p.value.isBool()) newC = new BoolParameter(p.name.toString(), p.name.toString(), false);
				else if (p.value.isDouble()) newC = new FloatParameter(p.name.toString(), p.name.toString(), 0);
				else if (p.value.isInt()) newC = new IntParameter(p.name.toString(), p.name.toString(), 0);
				else if (p.value.isString() || p.value.isVoid()) newC = new StringParameter(p.name.toString(), p.name.toString(), "");
				else if (p.value.isArray())
				{
					if (p.value.size() == 1) newC = new FloatParameter(p.name.toString(), p.name.toString(), 0);
					else if (p.value.size() == 2) newC = new Point2DParameter(p.name.toString(), p.name.toString());
					else if (p.value.size() == 3) newC = new Point3DParameter(p.name.toString(), p.name.toString());
					else if (p.value.size() == 3) newC = new ColorParameter(p.name.toString(), p.name.toString());
				}


				if (newC != nullptr)
				{
					newC->isCustomizableByUser = true;
					newC->isRemovableByUser = true;
					newC->isSavable = true;
					newC->saveValueOnly = false;
					container->addControllable(newC);
				}
			}

			if (newC != nullptr)
			{
				if (newC->type == Controllable::TRIGGER && (int)p.value != 0) ((Trigger*)newC)->trigger();
				else
				{
					Parameter* param = dynamic_cast<Parameter*>(newC);
					if (param != nullptr) param->setValue(p.value.isVoid() ? "" : p.value, false, true);
				}
			}
		}
	}
}


ModuleUI* WebSocketServerModule::getModuleUI()
{
	return new WebSocketServerModuleUI(this);
}
