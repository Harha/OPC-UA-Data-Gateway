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

	UA_SubscriptionSettings * OPCUA_SubscriptionSettings;

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

		// Get properties
		UA_DateTime datetime = value->sourceTimestamp;

		// Build the JSON object
		json opcua_variable;
		opcua_variable["nsIndex"] = sub->getNsIndex();
		opcua_variable["identifier"] = sub->getIdentifier();
		opcua_variable["serverId"] = sub->getServerId();
		opcua_variable["serverTimeStamp"] = UADateTimeToJSONDateTime(datetime);

		if (value->hasValue)
		{
			switch (value->value.type->typeIndex)
			{
			case UA_TYPES_STRING:
			{
				opcua_variable["value"] = (*(UA_String *)value->value.data).data;
			} break;
			case UA_TYPES_BYTESTRING:
			{
				opcua_variable["value"] = (*(UA_ByteString *)value->value.data).data;
			} break;
			case UA_TYPES_BOOLEAN:
			{
				opcua_variable["value"] = *(UA_Boolean *)value->value.data == UA_TRUE ? true : false;
			} break;
			case UA_TYPES_INT16:
			{
				opcua_variable["value"] = *(UA_Int16 *)value->value.data;
			} break;
			case UA_TYPES_INT32:
			{
				opcua_variable["value"] = *(UA_Int32 *)value->value.data;
			} break;
			case UA_TYPES_INT64:
			{
				opcua_variable["value"] = *(UA_Int64 *)value->value.data;
			} break;
			case UA_TYPES_UINT16:
			{
				opcua_variable["value"] = *(UA_UInt16 *)value->value.data;
			} break;
			case UA_TYPES_UINT32:
			{
				opcua_variable["value"] = *(UA_UInt32 *)value->value.data;
			} break;
			case UA_TYPES_UINT64:
			{
				opcua_variable["value"] = *(UA_UInt64 *)value->value.data;
			} break;
			case UA_TYPES_FLOAT:
			{
				opcua_variable["value"] = *(UA_Float *)value->value.data;
			} break;
			}
		}

		// POST the changed value to REST
		if (opcua_variable.find("value") != opcua_variable.end())
			sub->getClient()->getHttpClient()->sendJSON("/opcuavariables", HTTP_POST, opcua_variable);

		// Log the variable
		if (sub->getClient()->getHttpClient()->isVerbose())
			LOG("OPCUA_Variable: %s\n", UA_DateTime_now(), opcua_variable.dump().c_str());
	}

	OPCUA_Subscription::OPCUA_Subscription(
		OPCUA_Client * client,
		UA_NodeId * nodeId,
		int32_t serverId
	) :
		m_identifier((char *)nodeId->identifier.byteString.data),
		m_nsIndex(nodeId->namespaceIndex),
		m_client(client),
		m_status(UA_STATUSCODE_GOOD),
		m_id(0),
		m_nodeId(new UA_NodeId(*nodeId)),
		m_monitoredItemId(0),
		m_serverId(serverId)
	{
		LOG("OPCUA_Subscription init. Identifier: %s, ServerId: %d\n", UA_DateTime_now(), m_identifier.c_str(), m_serverId);

		m_status = UA_Client_Subscriptions_new(m_client->getClient(), *OPCUA_SubscriptionSettings, &m_id);

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the object.");

		m_status = UA_Client_Subscriptions_addMonitoredItem(m_client->getClient(), m_id, *m_nodeId, UA_ATTRIBUTEID_VALUE, &OPCUA_Callback_MonitoredItem, (void *) this, &m_monitoredItemId);

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the subscription link.");

		LOG("OPCUA_Subscription was created. Id: %d\n", UA_DateTime_now(), m_id);
	}

	OPCUA_Subscription::~OPCUA_Subscription()
	{
		if (m_client != NULL)
		{
			UA_Client_Subscriptions_remove(m_client->getClient(), m_id);

			LOG("OPCUA_Subscription was destroyed. Id: %d, ServerId: %d\n", UA_DateTime_now(), m_id, m_serverId);
		}
		else
		{
			WRN("OPCUA_Subscription was destroyed. Unattached instance, client was NULL.\n");
		}
	}

	OPCUA_Client * OPCUA_Subscription::getClient()
	{
		return m_client;
	}

	std::string OPCUA_Subscription::getIdentifier() const
	{
		return m_identifier;
	}

	uint16_t OPCUA_Subscription::getNsIndex() const
	{
		return m_nsIndex;
	}

	UA_StatusCode OPCUA_Subscription::getStatus() const
	{
		return m_status;
	}

	UA_NodeId * OPCUA_Subscription::getNodeId()
	{
		return m_nodeId;
	}

	uint32_t OPCUA_Subscription::getId() const
	{
		return m_id;
	}

	uint32_t OPCUA_Subscription::getMonitoredItemId() const
	{
		return m_monitoredItemId;
	}

	int32_t OPCUA_Subscription::getServerId() const
	{
		return m_serverId;
	}

}