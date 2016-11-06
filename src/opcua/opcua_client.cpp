#include "opcua_client.h"
#include <iostream>
#include <fstream>
#include <open62541.h>
#include "../macros.h"
#include "opcua_subscription.h"

namespace gateway
{

	UA_StatusCode OPCUA_Callback_NodeIterator(
		UA_NodeId childId,
		UA_Boolean isInverse,
		UA_NodeId referenceTypeId,
		void * handle
	)
	{
		// Do not subscribe if NodeId type does not qualify or target is parent of refrenceTypeId
		if (childId.identifierType != UA_NODEIDTYPE_STRING || isInverse)
			return UA_STATUSCODE_GOOD;

		// Get client instance
		OPCUA_Client * client = (OPCUA_Client *)handle;

		// Create subscription instance
		OPCUA_Subscription * sub = new OPCUA_Subscription(client->getClient(), &childId, client->getServerId());

		// Insert the subscription to client's vector
		client->getSubscriptions().push_back(sub);

		return sub->getStatus();
	}

	OPCUA_Client::OPCUA_Client(
		const std::string & endpoint,
		const std::string & username,
		const std::string & password,
		int32_t serverId
	) :
		m_endpoint(endpoint),
		m_username(username),
		m_password(password),
		m_client(NULL),
		m_status(UA_STATUSCODE_GOOD),
		m_serverId(serverId),
		m_subscriptions()
	{
		LOG("OPCUA_Client serverId(%d) initializing... endpoint URL: %s\n", UA_DateTime_now(), m_serverId, m_endpoint.c_str());

		// Create UA_Client instance
		m_client = UA_Client_new(UA_ClientConfig_standard);

		// Attempt to connect to the target endpoint
		if (username.empty())
			m_status = UA_Client_connect(m_client, m_endpoint.c_str());
		else
			m_status = UA_Client_connect_username(m_client, m_endpoint.c_str(), m_username.c_str(), m_password.c_str());

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Client failed to connect to target endpoint.");

		LOG("OPCUA_Client connected successfully to target endpoint.\n");
	}

	OPCUA_Client::~OPCUA_Client()
	{
		if (m_client != NULL)
		{
			UA_Client_disconnect(m_client);
			UA_Client_delete(m_client);

			LOG("OPCUA_Client serverId(%d) was destroyed. Endpoint URL: %s", UA_DateTime_now(), m_serverId, m_endpoint.c_str());
		}
	}

	void OPCUA_Client::update()
	{
		UA_Client_Subscriptions_manuallySendPublishRequest(m_client);
	}

	void OPCUA_Client::subscribeToAll(uint16_t nsIndex, char * identifier)
	{
		m_status = UA_Client_forEachChildNodeCall(m_client, UA_NODEID_STRING(nsIndex, identifier), &OPCUA_Callback_NodeIterator, (void *) this);

		LOG("OPCUA_Client serverId(%d) subscribeToAll %d: %s\n", UA_DateTime_now(), m_serverId, nsIndex, identifier);
	}

	void OPCUA_Client::subscribeToOne(uint16_t nsIndex, char * identifier)
	{
		OPCUA_Subscription * sub = new OPCUA_Subscription(m_client, &UA_NODEID_STRING(nsIndex, identifier), m_serverId);
		m_subscriptions.push_back(sub);

		LOG("OPCUA_Client serverId(%d) subscribeToOne %d: %s\n", UA_DateTime_now(), m_serverId, nsIndex, identifier);
	}

	std::string OPCUA_Client::getEndpoint() const
	{
		return m_endpoint;
	}

	std::string OPCUA_Client::getUsername() const
	{
		return m_username;
	}

	std::string OPCUA_Client::getPassword() const
	{
		return m_password;
	}

	UA_Client * OPCUA_Client::getClient()
	{
		return m_client;
	}

	UA_StatusCode OPCUA_Client::getStatus() const
	{
		return m_status;
	}

	int32_t OPCUA_Client::getServerId() const
	{
		return m_serverId;
	}

	std::vector<OPCUA_Subscription *> & OPCUA_Client::getSubscriptions()
	{
		return m_subscriptions;
	}

}