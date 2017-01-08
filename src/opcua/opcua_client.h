#ifndef CLIENT_OPCUA_H
#define CLIENT_OPCUA_H

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
	class HTTP_Client;

	class OPCUA_Client
	{
	public:
		OPCUA_Client(
			const std::string & jsonConfig,
			const std::string & jsonDbServersConfig,
			const std::string & jsonDbSubscriptionsConfig,
			HTTP_Client * const httpClient
		);
		~OPCUA_Client();
		void update();
		void subscribeToAll(uint16_t nsIndex = 0, char * identifier = "");
		void subscribeToOne(uint16_t nsIndex = 0, char * identifier = "");
		std::string getJsonConfig() const;
		std::string getJsonDbServersConfig() const;
		std::string getJsonDbSubscriptionsConfig() const;
		UA_Client * getClient();
		UA_StatusCode & getStatus();
		HTTP_Client * getHttpClient();
		int32_t getServerId() const;
		std::string getEndpoint() const;
		std::string getUsername() const;
		std::string getPassword() const;
		double getSubPublishInterval() const;
		uint32_t getSubLifetimeCount() const;
		uint32_t getSubMaxKeepAliveCount() const;
		uint32_t getSubMaxNotificationsPerPublish() const;
		bool isSubPublishEnabled() const;
		uint8_t getSubPublishPriority() const;
		std::vector<OPCUA_Subscription *> & getSubscriptions();
	private:
		std::string m_jsonConfig;
		std::string m_jsonDbServersConfig;
		std::string m_jsonDbSubscriptionsConfig;
		UA_Client * m_client;
		UA_StatusCode m_status;
		HTTP_Client * m_httpClient;
		int32_t m_serverId;
		std::string m_endpoint;
		std::string m_username;
		std::string m_password;
		double m_subPublishInterval;
		uint32_t m_subLifetimeCount;
		uint32_t m_subMaxKeepAliveCount;
		uint32_t m_subMaxNotificationsPerPublish;
		bool m_subPublishEnabled;
		uint8_t m_subPublishPriority;
		std::vector<OPCUA_Subscription *> m_subscriptions;
	};

}

#endif // CLIENT_OPCUA_H