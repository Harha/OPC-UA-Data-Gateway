#include "opcua_client.h"
#include <iostream>
#include <fstream>
#include <open62541.h>
#include "../macros.h"
#include "opcua_subscription.h"
#include "../http/http_client.h"
#include "../3rdparty/json.hpp"

// For convenience
using json = nlohmann::json;

namespace gateway
{

	UA_StatusCode OPCUA_Callback_NodeIterator(
		UA_NodeId childId,
		UA_Boolean isInverse,
		UA_NodeId referenceTypeId,
		void * handle
	)
	{
		// Do not subscribe if NodeId type does not qualify or target is parent of referenceTypeId
		if (childId.identifierType != UA_NODEIDTYPE_STRING || isInverse)
			return UA_STATUSCODE_GOOD;

		// Get the client instance
		OPCUA_Client * client = (OPCUA_Client *)handle;

		// Create a new subscription instance
		OPCUA_Subscription * sub = new OPCUA_Subscription(client, &childId);
		client->getSubscriptions().push_back(sub);

		return client->getStatus();
	}

	OPCUA_Client::OPCUA_Client(
		const std::string & jsonConfig,
		const std::string & jsonDbServersConfig,
		const std::string & jsonDbSubscriptionsConfig,
		HTTP_Client * const httpClient
	) :
		m_jsonConfig(jsonConfig),
		m_jsonDbServersConfig(jsonDbServersConfig),
		m_jsonDbSubscriptionsConfig(jsonDbSubscriptionsConfig),
		m_client(NULL),
		m_status(UA_STATUSCODE_GOOD),
		m_httpClient(httpClient),
		m_serverId(0),
		m_endpoint("null"),
		m_username(""),
		m_password(""),
		m_subPublishInterval(10.0),
		m_subPublishPriority(1),
		m_subscriptions()
	{
		// Get config strings as JSON objects
		json jsonCfg = json::parse(m_jsonConfig);
		json jsonDbServersCfg = json::parse(m_jsonDbServersConfig);
		json jsonDbSubscriptionsCfg = json::parse(m_jsonDbSubscriptionsConfig);

		// Fetch client configuration
		m_serverId = jsonCfg["serverId"].get<int32_t>();
		m_endpoint = jsonCfg["endpoint"].get<std::string>();
		m_username = jsonCfg["username"].get<std::string>();
		m_password = jsonCfg["password"].get<std::string>();
		m_subPublishInterval = jsonCfg["subPublishInterval"].get<double>();
		m_subPublishPriority = jsonCfg["subPublishPriority"].get<uint8_t>();

		// Create UA_Client instance
		m_client = UA_Client_new(UA_ClientConfig_standard);

		// Attempt to connect the client based on given configuration
		if (m_username.empty())
			m_status = UA_Client_connect(m_client, m_endpoint.c_str());
		else
			m_status = UA_Client_connect_username(m_client, m_endpoint.c_str(), m_username.c_str(), m_password.c_str());

		// Throw if connection attempt failed
		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Client failed to connect to target endpoint.");

		LOG("OPCUA_Client serverId(%d) connected successfully to %s\n", UA_DateTime_now(), m_serverId, m_endpoint.c_str());

		// GET target serverId from REST
		json jsonDbServer = m_httpClient->getJSON("/opcuaservers/" + std::to_string(m_serverId));

		// POST or PUT server config to REST
		HTTP_Request_t http_req = (jsonDbServer.type() == json::value_t::array) ? HTTP_PUT : HTTP_POST;
		m_httpClient->sendJSON("/opcuaservers", http_req, jsonCfg);

		LOG("OPCUA_Client serverId(%d) %s to REST.\n", UA_DateTime_now(), m_serverId, (http_req == HTTP_POST) ? "HTTP_POST" : "HTTP_PUT");

		// Subscribe to all namespaces / nodes described in config
		size_t n_subscriptions = jsonCfg["subscriptions"].size();
		for (size_t i = 0; i < n_subscriptions; i++)
		{
			bool isFolder = jsonCfg["subscriptions"][i]["isFolder"].get<bool>();
			uint16_t nsIndex = jsonCfg["subscriptions"][i]["nsIndex"].get<uint16_t>();

			size_t n_identifiers = jsonCfg["subscriptions"][i]["identifiers"].size();
			for (size_t j = 0; j < n_identifiers; j++)
			{
				std::string identifier = jsonCfg["subscriptions"][i]["identifiers"][j].get<std::string>();
				if (isFolder == false)
					subscribeToOne(nsIndex, &identifier[0u]);
				else
					subscribeToAll(nsIndex, &identifier[0u]);
			}
		}

		LOG("OPCUA_Client serverId(%d) initialized successfully.\n", UA_DateTime_now(), m_serverId);

	}

	OPCUA_Client::~OPCUA_Client()
	{
		if (m_client != NULL)
		{
			for (OPCUA_Subscription * sub : m_subscriptions)
				delete sub;

			UA_Client_disconnect(m_client);
			UA_Client_delete(m_client);

			LOG("OPCUA_Client serverId(%d) was destroyed, endpoint: %s\n", UA_DateTime_now(), m_serverId, m_endpoint.c_str());
		}
		else
		{
			WRN("OPCUA_Client serverId(%d) was destroyed, m_client was NULL!\n", UA_DateTime_now(), m_serverId);
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
		OPCUA_Subscription * sub = new OPCUA_Subscription(this, &UA_NODEID_STRING(nsIndex, identifier));
		m_subscriptions.push_back(sub);

		LOG("OPCUA_Client serverId(%d) subscribeToOne %d: %s\n", UA_DateTime_now(), m_serverId, nsIndex, identifier);
	}

	std::string OPCUA_Client::getJsonConfig() const
	{
		return m_jsonConfig;
	}

	std::string OPCUA_Client::getJsonDbServersConfig() const
	{
		return m_jsonDbServersConfig;
	}

	std::string OPCUA_Client::getJsonDbSubscriptionsConfig() const
	{
		return m_jsonDbSubscriptionsConfig;
	}

	UA_Client * OPCUA_Client::getClient()
	{
		return m_client;
	}

	UA_StatusCode & OPCUA_Client::getStatus()
	{
		return m_status;
	}

	HTTP_Client * OPCUA_Client::getHttpClient()
	{
		return m_httpClient;
	}

	int32_t OPCUA_Client::getServerId() const
	{
		return m_serverId;
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

	double OPCUA_Client::getSubPublishInterval() const
	{
		return m_subPublishInterval;
	}

	uint8_t OPCUA_Client::getSubPublishPriority() const
	{
		return m_subPublishPriority;
	}

	std::vector<OPCUA_Subscription *> & OPCUA_Client::getSubscriptions()
	{
		return m_subscriptions;
	}

}