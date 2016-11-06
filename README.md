Gateway Project
===============

This OPC UA client service connects to target endpoints, subscribes to manually configured namespaces and nodes and starts pushing the subscribed object values to a REST service.

Dependencies
------------
- open62541
- libcurl
- curlcpp

This repository includes dynamically linked Win32 versions of the libraries listed above.

- ./inc
- ./lib
- ./bin

Building
--------
TODO: CMakeLists.txt

Currently the repository includes a visual studio 2015 solution. In the future that will be replaced with a CMakeLists.txt file.

Configuration
-------------
- ./res/settings.json

- ua_client_config
-- Subscription settings can be changed here
- ua_rest_config
-- Target REST service settings can be changed here
- ua_servers_config
-- Target OPC UA endpoint settings can be changed here
-- List of OPC UA Server objects

Licenses
--------
- This software: LICENSE
- Open62541: LICENSE_open62541
- libcurl: LICENSE_libcurl
- curlcpp: LICENSE_curlcpp