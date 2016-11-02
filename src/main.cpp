#include <iostream>
#include <fstream>
#include <thread>
#include <open62541.h>
#include "3rdparty/json.hpp"
#include "macros.h"
#include "opcua/client.h"

using json = nlohmann::json;
using namespace gateway;

static UA_StatusCode gateway_status;
static std::vector<OPCUA_Client *> gateway_clients;
static json gateway_settings;

static UA_SubscriptionSettings OPCUA_SubscriptionSettings =
{
	100.0,	// requestedPublishingInterval
	10000,	// requestedLifetimeCount
	1,		// requestedMaxKeepAliveCount
	10,		// maxNotificationsPerPublish
	true,	// publishingEnabled
	1		// priority
};

int main(int argc, char * argv[])
{
	// Read settings in JSON format
	std::ifstream settings("./res/settings.json", std::ifstream::binary);

	if (settings.is_open() == false)
	{
		ERR("Error: Cannot find settings.json from ./res/ folder. Please make sure it exists.\n", UA_DateTime_now());
		return -1;
	}

	settings >> gateway_settings;
	settings.close();

	// Configure stuff
	OPCUA_SubscriptionSettings.requestedPublishingInterval = gateway_settings["ua_client_config"]["sub_publish_interval"].get<UA_Double>();
	OPCUA_SubscriptionSettings.priority = gateway_settings["ua_client_config"]["sub_publish_priority"].get<UA_Byte>();

	// Initialize all clients
	try
	{
		size_t n_servers = gateway_settings["ua_servers_config"].size();
		for (size_t i = 0; i < n_servers; i++)
		{
			// Create client instance
			OPCUA_Client * client = new OPCUA_Client(
				gateway_settings["ua_servers_config"][i]["endpoint"].get<std::string>(),
				gateway_settings["ua_servers_config"][i]["username"].get<std::string>(),
				gateway_settings["ua_servers_config"][i]["password"].get<std::string>(),
				gateway_settings["ua_servers_config"][i]["id"].get<int32_t>()
			);

			// Subscribe to all target namespaces / nodes
			size_t n_subscriptions = gateway_settings["ua_servers_config"][i]["subscriptions"].size();
			for (size_t j = 0; j < n_subscriptions; j++)
			{
				client->subscribeToAll(
					gateway_settings["ua_servers_config"][i]["subscriptions"][j]["ns_index"].get<uint16_t>(),
					&gateway_settings["ua_servers_config"][i]["subscriptions"][j]["identifier"].get<std::string>()[0u]
				);
			}

			// Push the client into clients vector
			gateway_clients.push_back(client);
		}
	}
	catch (const std::exception & e)
	{
		ERR("%s\n", UA_DateTime_now(), e.what());
	}

	// Main loop, exit if an error occurs
	while (gateway_status == UA_STATUSCODE_GOOD)
	{
		// Iterate through all clients
		for (OPCUA_Client * c : gateway_clients)
		{
			c->update();

			gateway_status = c->getStatus();
		}

		// Sleep for a bit, just to keep the OS sane
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Cleanup resources
	for (OPCUA_Client * c : gateway_clients)
	{
		delete c;
	}

	system("pause");
	return 0;
}