#include "opcua_subscription.h"
#include <open62541.h>
#include "../macros.h"

namespace gateway
{

	UA_SubscriptionSettings * OPCUA_SubscriptionSettings;

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

		if (value->hasValue)
		{
			switch (value->value.type->typeIndex)
			{
			case UA_TYPES_BOOLEAN:
			{
				LOG("ServerId: %d, Identifier: %16s, NsIndex: %d, Value: %s\n", datetime, sub->getServerId(), sub->getIdentifier().c_str(), sub->getNsIndex(), *(UA_Boolean *)value->value.data == UA_TRUE ? "true" : "false");
			}
			break;
			case UA_TYPES_INT16:
			case UA_TYPES_INT32:
			case UA_TYPES_INT64:
			{
				LOG("ServerId: %d, Identifier: %16s, NsIndex: %d, Value: %d\n", datetime, sub->getServerId(), sub->getIdentifier().c_str(), sub->getNsIndex(), *(UA_Int64 *)value->value.data);
			}
			break;
			case UA_TYPES_FLOAT:
			{
				LOG("ServerId: %d, Identifier: %16s, NsIndex: %d, Value: %.5f\n", datetime, sub->getServerId(), sub->getIdentifier().c_str(), sub->getNsIndex(), *(UA_Float *)value->value.data);
			}
			break;
			}
		}
	}

	OPCUA_Subscription::OPCUA_Subscription(
		UA_Client * client,
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

		m_status = UA_Client_Subscriptions_new(m_client, *OPCUA_SubscriptionSettings, &m_id);

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the object.");

		m_status = UA_Client_Subscriptions_addMonitoredItem(m_client, m_id, *m_nodeId, UA_ATTRIBUTEID_VALUE, &OPCUA_Callback_MonitoredItem, (void *) this, &m_monitoredItemId);

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the subscription link.");

		LOG("OPCUA_Subscription was created. Id: %d\n", UA_DateTime_now(), m_id);
	}

	OPCUA_Subscription::~OPCUA_Subscription()
	{
		if (m_client != NULL)
		{
			UA_Client_Subscriptions_remove(m_client, m_id);

			LOG("OPCUA_Subscription was destroyed. Id: %d, ServerId: %d\n", UA_DateTime_now(), m_id, m_serverId);
		}
		else
		{
			WRN("OPCUA_Subscription was destroyed. Unattached instance, client was NULL.");
		}
	}

	std::string OPCUA_Subscription::getIdentifier() const
	{
		return m_identifier;
	}

	uint16_t OPCUA_Subscription::getNsIndex() const
	{
		return m_nsIndex;
	}

	UA_Client * OPCUA_Subscription::getClient()
	{
		return m_client;
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