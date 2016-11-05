#include <iostream>
#include <fstream>
#include <thread>
#include <open62541.h>
#include "3rdparty/json.hpp"
#include "3rdparty/happyhttp.h"
#include "macros.h"
#include "util/strutils.h"
#include "opcua/client.h"
#include "opcua/subscription.h"

// Socketing on windows
#ifdef _WIN32 
#include <winsock2.h>
#endif // _WIN32 

using json = nlohmann::json;
using namespace gateway;

static UA_StatusCode gateway_status;
static std::vector<OPCUA_Client *> gateway_clients;
static json gateway_settings;
static json gateway_database_servers;

static UA_SubscriptionSettings gateway_subscription_settings =
{
	// requestedPublishingInterval
	100.0,
	// requestedLifetimeCount
	10000,
	// requestedMaxKeepAliveCount
	1,
	// maxNotificationsPerPublish
	10,
	// publishingEnabled
	true,
	// priority
	1
};

// Callback: HTTP Request begin
void HTTPOnBegin(const happyhttp::Response * r, void * userdata)
{
	LOG("HTTP Request begin, Response status: %d\n", UA_DateTime_now(), r->getstatus());
}

// Callback: HTTP Request on data
void HTTPOnData(const happyhttp::Response * r, void * userdata, const unsigned char * data, int n)
{
	LOG("HTTP Request data, Response status: %d, n: %d\n", UA_DateTime_now(), r->getstatus(), n);

	// Get request type
	char * req_type = static_cast<char *>(userdata);

	switch (cstr2int(req_type))
	{
	case cstr2int("GET /opcuaservers"):
	{
		// Get data as string and resize it to the content length
		std::string data_string(data, data + n);

		// Parse the JSON data and store it to memory
		gateway_database_servers = json::parse(data_string);

		LOG("HTTP Request GET /opcuaservers, json: %s\n", UA_DateTime_now(), gateway_database_servers.dump());
	} break;
	}
}

// Callback: HTTP Request on complete
void HTTPOnComplete(const happyhttp::Response * r, void * userdata)
{
	LOG("HTTP Request complete, Response status: %d\n", UA_DateTime_now(), r->getstatus());
}


int main(int argc, char * argv[])
{
	// Initialize socketing on windows
#ifdef _WIN32 
	WSAData wsaData;
	int code = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (code != 0)
	{
		ERR("WSAStartup failed. Error code: %d\n", UA_DateTime_now(), code);
		return 0;
	}
#endif // _WIN32 

	// Read settings in JSON format
	std::ifstream settings("./res/settings.json", std::ifstream::binary);

	if (settings.is_open() == false)
	{
		ERR("Error: Cannot find settings.json from ./res/ folder. Please make sure it exists.\n", UA_DateTime_now());
		return -1;
	}

	settings >> gateway_settings;
	settings.close();

	// Configure subscription settings
	gateway_subscription_settings.requestedPublishingInterval = gateway_settings["ua_client_config"]["sub_publish_interval"].get<UA_Double>();
	gateway_subscription_settings.priority = gateway_settings["ua_client_config"]["sub_publish_priority"].get<UA_Byte>();

	OPCUA_SubscriptionSettings = &gateway_subscription_settings;

	// Get REST service properties
	std::string rest_url = gateway_settings["ua_rest_config"]["url"].get<std::string>();
	int32_t rest_port = gateway_settings["ua_rest_config"]["port"].get<int32_t>();

	// Initialize HTTP connection to REST
	happyhttp::Connection conn(rest_url.c_str(), rest_port);

	// Request list of OPC UA servers in database
	try
	{
		conn.setcallbacks(HTTPOnBegin, HTTPOnData, HTTPOnComplete, static_cast<void *>("GET /opcuaservers"));
		conn.request("GET", "/opcuaservers");
		while (conn.outstanding())
		{
			conn.pump();
		}
		conn.close();
	}
	catch (const happyhttp::Wobbly & w)
	{
		ERR("%s\n", UA_DateTime_now(), w.what());
	}

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
			int32_t serverId = gateway_settings["ua_servers_config"][i]["id"].get<int32_t>();

			// Create client instance
			OPCUA_Client * client = new OPCUA_Client(endpoint, username, password, serverId);

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

	// Close socketing on windows
#ifdef _WIN32 
	WSACleanup();
#endif // _WIN32 

	return gateway_status;
}