#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <string>
#include <cstdint>

struct UA_Client;
struct _UA_NodeId;
typedef _UA_NodeId UA_NodeId;
typedef uint32_t UA_StatusCode;
struct _UA_SubscriptionSettings;
typedef _UA_SubscriptionSettings UA_SubscriptionSettings;

namespace gateway
{

	extern UA_SubscriptionSettings * OPCUA_SubscriptionSettings;

	class OPCUA_Subscription
	{
	public:
		OPCUA_Subscription(
			UA_Client * client,
			UA_NodeId * nodeId,
			int32_t serverId
		);
		~OPCUA_Subscription();
		std::string getIdentifier() const;
		uint16_t getNsIndex() const;
		UA_Client * getClient();
		UA_StatusCode getStatus() const;
		UA_NodeId * getNodeId();
		uint32_t getId() const;
		uint32_t getMonitoredItemId() const;
		int32_t getServerId() const;
	private:
		std::string m_identifier;
		uint16_t m_nsIndex;
		UA_Client * m_client;
		UA_StatusCode m_status;
		UA_NodeId * m_nodeId;
		uint32_t m_id;
		uint32_t m_monitoredItemId;
		int32_t m_serverId;
	};

}

#endif // SUBSCRIPTION_H