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
			const std::string & password = ""
		);
		~OPCUA_Client();
		void update();
		std::string getEndpoint() const;
		std::string getUsername() const;
		std::string getPassword() const;
		UA_StatusCode getStatus() const;
	private:
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
		UA_Client * m_client;
		UA_StatusCode m_status;
		std::vector<OPCUA_Subscription *> m_subscriptions;
	};

}

#endif // CLIENT_H