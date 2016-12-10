OPC UA Data Gateway
===================

This is an OPC UA client service that connects to target endpoints, subscribes to manually configured namespaces and nodes and starts pushing the subscribed object values to a REST service.

Related projects
----------------
Backend project: [OPC UA Data REST](https://github.com/Harha/OPC-UA-Data-REST)

Frontend project: [OPC UA Data Visualizer](https://github.com/Harha/OPC-UA-Data-Visualizer)

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

Currently the repository includes a visual studio 2015 project. In the future that will be replaced with a CMakeLists.txt file.

Configuration
-------------
- ./res/settings.json

- ua_rest_config		(HTTP Client configuration)
- ua_client_config		(Array of OPC UA Client objects)

Licenses
--------
- This software: LICENSE
- Open62541: LICENSE_open62541
- libcurl: LICENSE_libcurl
- curlcpp: LICENSE_curlcpp