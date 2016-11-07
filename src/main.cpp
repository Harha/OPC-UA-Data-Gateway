#include <iostream>
#include <fstream>
#include <thread>
#include <open62541.h>
#include "3rdparty/json.hpp"
#include "macros.h"
#include "util/strutils.h"
#include "opcua/opcua_client.h"
#include "opcua/opcua_subscription.h"
#include "http/http_client.h"

// For convenience
using json = nlohmann::json;
using namespace gateway;

// Gateway OPC UA data
static UA_StatusCode gateway_opcua_status;
static std::vector<OPCUA_Client *> gateway_opcua_clients;
static json gateway_settings;

// Gateway HTTP data
static HTTP_Client * gateway_http_client;
static json gateway_db_servers;

static UA_SubscriptionSettings gateway_subscription_settings =
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

	// Get OPC UA subscription properties
	gateway_subscription_settings.requestedPublishingInterval = gateway_settings["ua_client_config"]["sub_publish_interval"].get<UA_Double>();
	gateway_subscription_settings.priority = gateway_settings["ua_client_config"]["sub_publish_priority"].get<UA_Byte>();
	OPCUA_SubscriptionSettings = &gateway_subscription_settings;

	// Get REST service properties
	std::string rest_endpoint = gateway_settings["ua_rest_config"]["endpoint"].get<std::string>();
	std::string rest_username = gateway_settings["ua_rest_config"]["username"].get<std::string>();
	std::string rest_password = gateway_settings["ua_rest_config"]["password"].get<std::string>();

	// Initialize HTTP client
	gateway_http_client = new HTTP_Client(rest_endpoint, rest_username, rest_password);

	// Get list of servers in database
	gateway_db_servers = gateway_http_client->getJSON("/opcuaservers");

	// Initialize all clients
	try
	{
		size_t n_servers = gateway_settings["ua_servers_config"].size();
		for (size_t i = 0; i < n_servers; i++)
		{
			// Get server properties
			std::string endpoint = gateway_settings["ua_servers_config"][i]["endpoint"].get<std::string>();
			std::string username = gateway_settings["ua_servers_config"][i]["username"].get<std::string>();
			std::string password = gateway_settings["ua_servers_config"][i]["password"].get<std::string>();
			int32_t serverId = gateway_settings["ua_servers_config"][i]["serverId"].get<int32_t>();

			// Create client instance
			OPCUA_Client * client = new OPCUA_Client(endpoint, username, password, serverId, gateway_http_client);

			// Subscribe to all target namespaces / nodes
			size_t n_subscriptions = gateway_settings["ua_servers_config"][i]["subscriptions"].size();
			for (size_t j = 0; j < n_subscriptions; j++)
			{
				bool isFolder = gateway_settings["ua_servers_config"][i]["subscriptions"][j]["is_folder"].get<bool>();
				uint16_t nsIndex = gateway_settings["ua_servers_config"][i]["subscriptions"][j]["ns_index"].get<uint16_t>();
				std::string identifier = gateway_settings["ua_servers_config"][i]["subscriptions"][j]["identifier"].get<std::string>();

				if (isFolder == false)
				{
					// Single subscription
					client->subscribeToOne(
						nsIndex,
						&identifier[0u]
					);
				}
				else
				{
					// Subscribe to all childs of this node
					client->subscribeToAll(
						nsIndex,
						&identifier[0u]
					);
				}
			}

			// Does this server exist in database?
			bool exists_in_db = false;
			size_t n_db_servers = gateway_db_servers.size();
			for (size_t j = 0; j < n_db_servers; j++)
			{
				int32_t db_serverId = gateway_db_servers[j]["serverId"].get<int32_t>();

				if (serverId == db_serverId)
				{
					exists_in_db = true;
					break;
				}
			}

			// Insert / Update the server entry in database
			if (exists_in_db == false)
			{
				gateway_http_client->sendJSON("/opcuaservers", HTTP_POST, gateway_settings["ua_servers_config"][i]);
			}
			else
			{
				gateway_http_client->sendJSON("/opcuaservers", HTTP_PUT, gateway_settings["ua_servers_config"][i]);
			}

			// Push the client into clients vector
			gateway_opcua_clients.push_back(client);
		}
	}
	catch (const std::exception & e)
	{
		ERR("Excpetion: %s\n", UA_DateTime_now(), e.what());
	}

	// Main loop, exit if an error occurs
	while (gateway_opcua_status == UA_STATUSCODE_GOOD)
	{
		// Iterate through all clients
		for (OPCUA_Client * c : gateway_opcua_clients)
		{
			c->update();

			gateway_opcua_status = c->getStatus();
		}

		// Sleep for a bit, just to keep the OS sane
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Cleanup OPC UA resources
	for (OPCUA_Client * c : gateway_opcua_clients)
	{
		delete c;
	}

	// Cleanup HTTP resources
	delete gateway_http_client;

	return gateway_opcua_status;
}