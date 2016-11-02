#include "subscription.h"
#include <open62541.h>
#include "../macros.h"

namespace gateway
{

	void OPCUA_Callback_MonitoredItem(
		UA_UInt32 monId,
		UA_DataValue * value,
		void * context
	)
	{
	}

	OPCUA_Subscription::OPCUA_Subscription(
		UA_Client * client,
		UA_NodeId * nodeId,
		int32_t serverId
	) :
		m_client(client),
		m_status(UA_STATUSCODE_GOOD),
		m_id(0),
		m_nodeId(nodeId),
		m_monitoredItemId(0),
		m_serverId(serverId)
	{
		LOG("OPCUA_Subscription serverId(%d) initializing...\n", UA_DateTime_now(), m_serverId);

		m_status = UA_Client_Subscriptions_new(m_client, UA_SubscriptionSettings_standard, &m_id);

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the object.");

		m_status = UA_Client_Subscriptions_addMonitoredItem(m_client, m_id, *m_nodeId, UA_ATTRIBUTEID_VALUE, &OPCUA_Callback_MonitoredItem, (void *) this, &m_monitoredItemId);

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Subscription something went wrong while creating the subscription link.");

		LOG("OPCUA_Subscription was created. Id: %d, ServerId: %d\n", UA_DateTime_now(), m_id, m_serverId);
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