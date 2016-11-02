#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <string>
#include <cstdint>

struct UA_Client;
struct _UA_NodeId;
typedef _UA_NodeId UA_NodeId;

namespace gateway
{

	class OPCUA_Subscription
	{
	public:
		OPCUA_Subscription(
			UA_Client * client = NULL,
			uint32_t id = 0,
			uint32_t nodeId = 0,
			int32_t serverId = -1
		);
		~OPCUA_Subscription();
		UA_Client * getClient();
		uint32_t getId() const;
		uint32_t getNodeId();
		int32_t getServerId() const;
	private:
		UA_Client * m_client;
		uint32_t m_id;
		uint32_t m_nodeId;
		int32_t m_serverId;
	};

}

#endif // SUBSCRIPTION_H