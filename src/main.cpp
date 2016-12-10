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

// Gateway data
static json gateway_settings;

// Gateway OPC UA data
static UA_StatusCode gateway_opcua_status;
static std::vector<OPCUA_Client *> gateway_opcua_clients;

// Gateway HTTP data
static HTTP_Client * gateway_http_client;

// Gateway DB data
static json gateway_db_servers;
static json gateway_db_subscriptions;

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

	// Get REST service config
	std::string rest_endpoint = gateway_settings["ua_rest_config"]["endpoint"].get<std::string>();
	std::string rest_username = gateway_settings["ua_rest_config"]["username"].get<std::string>();
	std::string rest_password = gateway_settings["ua_rest_config"]["password"].get<std::string>();
	std::string rest_output = gateway_settings["ua_rest_config"]["output"].get<std::string>();
	bool rest_verbose = gateway_settings["ua_rest_config"]["verbose"].get<bool>();

	// Initialize HTTP client
	gateway_http_client = new HTTP_Client(gateway_settings["ua_rest_config"].dump());

	// Get list of servers and subscriptions from db
	gateway_db_servers = gateway_http_client->getJSON("/opcuaservers");
	gateway_db_subscriptions = gateway_http_client->getJSON("/opcuasubscriptions");

	try
	{
		// Initialize all clients
		size_t n_clients = gateway_settings["ua_client_config"].size();
		for (size_t i = 0; i < n_clients; i++)
		{
			// Create client instance
			OPCUA_Client * client = new OPCUA_Client(
				gateway_settings["ua_client_config"][i].dump(),
				gateway_db_servers.dump(),
				gateway_db_subscriptions.dump(),
				gateway_http_client
			);

			// Push the client into clients vector
			gateway_opcua_clients.push_back(client);
		}
	}
	catch (const std::exception & e)
	{
		ERR("Exception: %s\n", UA_DateTime_now(), e.what());
	}

	// Runtime service config properties
	bool quit_on_error = gateway_settings["ua_service_config"]["quit_on_error"].get<bool>();

	// Main loop, exit if an error occurs
	while (true)
	{
		// Iterate through all clients
		for (OPCUA_Client * c : gateway_opcua_clients)
		{
			c->update();
			gateway_opcua_status = c->getStatus();

			if (gateway_opcua_status != UA_STATUSCODE_GOOD && quit_on_error)
				break;
		}

		if (gateway_opcua_status != UA_STATUSCODE_GOOD && quit_on_error || gateway_opcua_clients.size() == 0 && quit_on_error)
			break;

		// Sleep for a bit, just to keep the OS sane
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Cleanup OPC UA clients
	for (OPCUA_Client * c : gateway_opcua_clients)
	{
		delete c;
	}

	// Cleanup HTTP client
	delete gateway_http_client;

	return gateway_opcua_status;
}