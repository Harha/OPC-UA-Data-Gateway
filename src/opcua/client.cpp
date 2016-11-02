#include "client.h"
#include <iostream>
#include <fstream>
#include <open62541.h>
#include "../macros.h"

namespace gateway
{

	OPCUA_Client::OPCUA_Client(
		const std::string & endpoint,
		const std::string & username,
		const std::string & password
	) :
		m_endpoint(endpoint),
		m_username(username),
		m_password(password),
		m_client(NULL),
		m_status(UA_STATUSCODE_GOOD),
		m_subscriptions()
	{
		// Create UA_Client instance
		m_client = UA_Client_new(UA_ClientConfig_standard);

		LOG("OPCUA_Client initilaizing... endpoint URL: %s\n", UA_DateTime_now(), m_endpoint.c_str());

		// Attempt to connect to the target endpoint
		if (username.empty())
			m_status = UA_Client_connect(m_client, m_endpoint.c_str());
		else
			m_status = UA_Client_connect_username(m_client, m_endpoint.c_str(), m_username.c_str(), m_password.c_str());

		if (m_status != UA_STATUSCODE_GOOD)
			throw std::exception("OPCUA_Client: Failed to connect to target endpoint.");

		LOG("OPCUA_Client connected successfully to target endpoint.\n");
	}

	OPCUA_Client::~OPCUA_Client()
	{
		if (m_client != NULL)
		{
			UA_Client_delete(m_client);

			LOG("OPCUA_Client was destroyed. Endpoint URL: %s", UA_DateTime_now(), m_endpoint.c_str());
		}
	}

	void OPCUA_Client::update()
	{

	}

	std::string OPCUA_Client::getEndpoint() const
	{
		return m_endpoint;
	}

	std::string OPCUA_Client::getUsername() const
	{
		return m_username;
	}

	std::string OPCUA_Client::getPassword() const
	{
		return m_password;
	}

	UA_StatusCode OPCUA_Client::getStatus() const
	{
		return m_status;
	}

}