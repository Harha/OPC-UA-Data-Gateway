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
	class OPCUA_Client;

	class OPCUA_Subscription
	{
	public:
		OPCUA_Subscription(
			OPCUA_Client * const client,
			UA_NodeId * const nodeId
		);
		~OPCUA_Subscription();
		OPCUA_Client * getClient();
		UA_NodeId * getNodeId();
		std::string getIdentifier() const;
		uint16_t getNsIndex() const;
		uint32_t getId() const;
		uint32_t getMonitoredItemId() const;
	private:
		OPCUA_Client * m_client;
		UA_NodeId * m_nodeId;
		std::string m_identifier;
		uint16_t m_nsIndex;
		uint32_t m_id;
		uint32_t m_monitoredItemId;
	};

}

#endif // SUBSCRIPTION_H