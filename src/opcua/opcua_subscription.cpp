#include "opcua_subscription.h"
#include <open62541.h>
#include "../macros.h"
#include "opcua_client.h"
#include "../http/http_client.h"
#include "../3rdparty/json.hpp"

// For convenience
using json = nlohmann::json;

namespace gateway
{

	// UA_DateTime -> JSON ISO 8601 DateTime conversion
	std::string UADateTimeToJSONDateTime(UA_DateTime datetime = UA_DateTime_now())
	{
		// Convert datetime to struct
		UA_DateTimeStruct datetime_struct = UA_DateTime_toStruct(datetime);

		// Print to char buffer
		char buffer[255];
		snprintf(buffer, 255, "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ",
			datetime_struct.year, datetime_struct.month, datetime_struct.day,
			datetime_struct.hour, datetime_struct.min, datetime_struct.sec,
			datetime_struct.milliSec
		);

		return std::string(buffer);
	}

	void OPCUA_Callback_MonitoredItem(
		UA_UInt32 monId,
		UA_DataValue * value,
		void * context
	)
	{
		// Get the subscription instance
		OPCUA_Subscription * sub = (OPCUA_Subscription *)context;

		// Build the JSON object
		json jsonThis;
		jsonThis["identifier"] = sub->getIdentifier();
		jsonThis["nsIndex"] = sub->getNsIndex();
		jsonThis["serverId"] = sub->getClient()->getServerId();
		jsonThis["serverTimeStamp"] = UADateTimeToJSONDateTime(value->sourceTimestamp);

		if (value->hasValue)
		{
			switch (value->value.type->typeIndex)
			{
			case UA_TYPES_STRING:
			{
				jsonThis["value"] = (*(UA_String *)value->value.data).data;
				jsonThis["type"] = "string";
			} break;
			case UA_TYPES_BYTESTRING:
			{
				jsonThis["value"] = (*(UA_ByteString *)value->value.data).data;
				jsonThis["type"] = "string";
			} break;
			case UA_TYPES_LOCALIZEDTEXT:
			{
				jsonThis["value"] = (*(UA_LocalizedText *)value->value.data).text.data;
				jsonThis["type"] = "string";
			} break;
			case UA_TYPES_DATETIME:
			{
				jsonThis["value"] = UADateTimeToJSONDateTime(*(UA_DateTime *)value->value.data);
				jsonThis["type"] = "datetime";
			} break;
			case UA_TYPES_BOOLEAN:
			{
				jsonThis["value"] = *(UA_Boolean *)value->value.data == UA_TRUE ? true : false;
				jsonThis["type"] = "bool";
			} break;
			case UA_TYPES_STATUSCODE:
			{
				jsonThis["value"] = *(UA_UInt32 *)value->value.data;
				jsonThis["type"] = "uint32_t";
			} break;
			case UA_TYPES_SBYTE:
			{
				jsonThis["value"] = *(UA_SByte *)value->value.data;
				jsonThis["type"] = "int8_t";
			} break;
			case UA_TYPES_INT16:
			{
				jsonThis["value"] = *(UA_Int16 *)value->value.data;
				jsonThis["type"] = "int16_t";
			} break;
			case UA_TYPES_INT32:
			{
				jsonThis["value"] = *(UA_Int32 *)value->value.data;
				jsonThis["type"] = "int32_t";
			} break;
			case UA_TYPES_INT64:
			{
				jsonThis["value"] = *(UA_Int64 *)value->value.data;
				jsonThis["type"] = "int64_t";
			} break;
			case UA_TYPES_BYTE:
			{
				jsonThis["value"] = *(UA_Byte *)value->value.data;
				jsonThis["type"] = "uint8_t";
			} break;
			case UA_TYPES_UINT16:
			{
				jsonThis["value"] = *(UA_UInt16 *)value->value.data;
				jsonThis["type"] = "uint16_t";
			} break;
			case UA_TYPES_UINT32:
			{
				jsonThis["value"] = *(UA_UInt32 *)value->value.data;
				jsonThis["type"] = "uint32_t";
			} break;
			case UA_TYPES_UINT64:
			{
				jsonThis["value"] = *(UA_UInt64 *)value->value.data;
				jsonThis["type"] = "uint64_t";
			} break;
			case UA_TYPES_FLOAT:
			{
				jsonThis["value"] = *(UA_Float *)value->value.data;
				jsonThis["type"] = "float";
			} break;
			case UA_TYPES_DOUBLE:
			{
				jsonThis["value"] = *(UA_Double *)value->value.data;
				jsonThis["type"] = "double";
			} break;
			}
		}

		// POST the variable to REST
		if (jsonThis.find("value") != jsonThis.end())
			sub->getClient()->getHttpClient()->sendJSON("/opcuavariables", HTTP_POST, jsonThis);

		// Log the variable in verbose mode
		if (sub->getClient()->getHttpClient()->isVerbose())
			LOG("OPCUA_Variable: %s\n", UA_DateTime_now(), jsonThis.dump().c_str());
	}

	OPCUA_Subscription::OPCUA_Subscription(
		OPCUA_Client * const client,
		UA_NodeId * const nodeId
	) :
		m_client(client),
		m_nodeId(new UA_NodeId(*nodeId)),
		m_identifier((char *)nodeId->identifier.byteString.data),
		m_nsIndex(nodeId->namespaceIndex),
		m_id(0),
		m_monitoredItemId(0)
	{
		// Get config strings as JSON objects
		json jsonCfg = json::parse(m_client->getJsonConfig());
		json jsonDbServersCfg = json::parse(m_client->getJsonDbServersConfig());
		json jsonDbSubscriptionsCfg = json::parse(m_client->getJsonDbSubscriptionsConfig());

		// Fetch subscription configuration
		UA_SubscriptionSettings configuration =
		{
			m_client->getSubPublishInterval(),	// requestedPublishingInterval
			10000,								// requestedLifetimeCount
			1,									// requestedMaxKeepAliveCount
			10,									// maxNotificationsPerPublish
			true,								// publishingEnabled
			m_client->getSubPublishPriority()	// priority
		};

		// Create UA_Subscription instance
		m_client->getStatus() = UA_Client_Subscriptions_new(m_client->getClient(), configuration, &m_id);

		// Throw if creation failed
		if (m_client->getStatus() != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the UA_Subscription instance.");

		// Create the subscription request
		m_client->getStatus() = UA_Client_Subscriptions_addMonitoredItem(m_client->getClient(), m_id, *m_nodeId, UA_ATTRIBUTEID_VALUE, &OPCUA_Callback_MonitoredItem, (void *) this, &m_monitoredItemId);

		// Throw if subscription failed
		if (m_client->getStatus() != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the subscription link.");

		LOG("OPCUA_Subscription serverId(%d) was linked successfully, id: %d\n", UA_DateTime_now(), m_client->getServerId(), m_identifier.c_str(), m_id);

		// Create a JSON instance
		json jsonThis;
		jsonThis["identifier"] = m_identifier;
		jsonThis["nsIndex"] = m_nsIndex;
		jsonThis["type"] = "NOT_IMPLEMENTED";
		jsonThis["serverId"] = m_client->getServerId();

		// GET target subscription from REST
		json jsonDbSubscription = m_client->getHttpClient()->getJSON("/opcuasubscriptions/" + std::to_string(m_nsIndex) + "/?identifier=" + m_identifier + "&serverId=" + std::to_string(m_client->getServerId()));

		// POST or PUT target subscription to REST
		HTTP_Request_t http_req = (jsonDbSubscription.empty() == false) ? HTTP_PUT : HTTP_POST;
		m_client->getHttpClient()->sendJSON("/opcuasubscriptions", http_req, jsonThis);

		LOG("OPCUA_Subscription serverId(%d) was initialized successfully, identifier: %s\n", UA_DateTime_now(), m_client->getServerId(), m_identifier.c_str());

	}

	OPCUA_Subscription::~OPCUA_Subscription()
	{
		if (m_client != NULL)
		{
			UA_Client_Subscriptions_remove(m_client->getClient(), m_id);

			LOG("OPCUA_Subscription id(%d) serverId(%d) was destroyed.\n", UA_DateTime_now(), m_id, m_client->getServerId());
		}
		else
		{
			WRN("OPCUA_Subscription id(%d) serverId(%d) was destroyed, m_client was NULL!\n", UA_DateTime_now(), m_id, m_client->getServerId());
		}
	}

	OPCUA_Client * OPCUA_Subscription::getClient()
	{
		return m_client;
	}

	UA_NodeId * OPCUA_Subscription::getNodeId()
	{
		return m_nodeId;
	}

	std::string OPCUA_Subscription::getIdentifier() const
	{
		return m_identifier;
	}

	uint16_t OPCUA_Subscription::getNsIndex() const
	{
		return m_nsIndex;
	}

	uint32_t OPCUA_Subscription::getId() const
	{
		return m_id;
	}

	uint32_t OPCUA_Subscription::getMonitoredItemId() const
	{
		return m_monitoredItemId;
	}

}