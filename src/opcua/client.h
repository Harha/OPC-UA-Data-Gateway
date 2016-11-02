#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <cstdint>
#include <vector>

struct UA_Client;
struct _UA_NodeId;
typedef _UA_NodeId UA_NodeId;
typedef uint32_t UA_StatusCode;

namespace gateway
{

	class OPCUA_Subscription;

	class OPCUA_Client
	{
	public:
		OPCUA_Client(
			const std::string & endpoint = "opc.tcp://localhost:48050",
			const std::string & username = "",
			const std::string & password = "",
			int32_t serverId = -1
		);
		~OPCUA_Client();
		void update();
		void subscribeToAll(uint16_t nsIndex = 0, char * identifier = "");
		void subscribeToOne(uint16_t nsIndex = 0, char * identifier = "");
		std::string getEndpoint() const;
		std::string getUsername() const;
		std::string getPassword() const;
		UA_Client * getClient();
		UA_StatusCode getStatus() const;
		int32_t getServerId() const;
		std::vector<OPCUA_Subscription *> & getSubscriptions();
	private:
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
		UA_Client * m_client;
		UA_StatusCode m_status;
		int32_t m_serverId;
		std::vector<OPCUA_Subscription *> m_subscriptions;
	};

}

#endif // CLIENT_H