#include "subscription.h"
#include <open62541.h>
#include "../macros.h"

namespace gateway
{

	void OPCUA_Callback_NodeValueChanged(
		UA_UInt32 node_id,
		UA_DataValue * value,
		void * context
	)
	{
	}

	OPCUA_Subscription::OPCUA_Subscription(
		UA_Client * client,
		uint32_t id,
		uint32_t nodeId,
		int32_t serverId
	) :
		m_client(client),
		m_id(id),
		m_nodeId(nodeId),
		m_serverId(serverId)
	{
		UA_Client_Subscriptions_new(m_client, UA_SubscriptionSettings_standard, &m_id);

		if (m_id == false)
			ERR("OPCUA_Subscription something went wrong while creating the object.");

		UA_StatusCode status = UA_Client_Subscriptions_addMonitoredItem(m_client, m_id, UA_NODEID_STRING(0, "test"), UA_ATTRIBUTEID_VALUE, &OPCUA_Callback_NodeValueChanged, (void *) this, &m_nodeId);

		if (status == UA_STATUSCODE_GOOD)
			LOG("OPCUA_Subscription was created. Id: %d, ServerId: %d\n", UA_DateTime_now(), m_id, m_serverId);
		else
			ERR("OPCUA_Subscription something went wrong while creating the subscription link.");
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

	uint32_t OPCUA_Subscription::getId() const
	{
		return m_id;
	}

	uint32_t OPCUA_Subscription::getNodeId()
	{
		return m_nodeId;
	}

	int32_t OPCUA_Subscription::getServerId() const
	{
		return m_serverId;
	}

}