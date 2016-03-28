.. include:: replace.txt

HTTP Traffic Generator
------------

HTTP traffic generator is one traffic model to generate web-like traffic and have different options to tune and generate HTTP traffic.

This model was developed by 
`the ResiliNets research group <http://www.ittc.ku.edu/resilinets>`_
at the University of Kansas.  

Overview
*********************
 
 The implementation of HTTP traffic generator composes mainly four parts:
 
 1. Client
 2. Server
 3. Controller
 4. Distribution Generation (Notification to Statistics Research, Bell Labs, Lucent Technologies and The University of North Carolina at Chapel Hill)
 
 The major classes include: 
 
 Class http::HttpClient implements the functions needed to initiate the transport connections to the HTTP server and the random variables generation functions

 Class http::HttpServer implements the functions to receive the connection from HTTP clients and establish the connection, receive HTTP packets received by the HTTP client

 Class http::HttpDistribution implements functions to take use of the distribution functions generated parameters and use them in the 

 Class http::HttpRandomVariableBase defines the major distribution functions to generate the parameters used in the HTTP traffic model

 Class http::HttpController centrally controls all the sending events of both client and server.  It also saves the ADU containers for both of them.

 When the traffic generator starts, the http client initialize connections to http server, after the connection has established,
 the client calls the distribution generation module and generates empirical parameters involved with http operation. For example,
 request size, response size, page request gap time, object request gap time server delay time, total objects per page, total pages per web session. All those
 parameters will be saved in runtime variable model, which can be accessed by both the http client and server. Based on the parameters
 saved in runtime variable, the client and server will generate http traffic which can represent real-world http traffic.

 This traffic generator is capable of generating both HTTP 1.0 traffic and HTTP 1.1 traffic.  When HTTP 1.0 mode is used, each http request
 will wait until the previous http transaction is over, while HTTP 1.1 will do pipelining, which means it sends multiple http requests 
 without waiting for previous responses; however, parallel connections is not yet supported.
 
 ADU defines the size and useful parameters in simulation.  It can represent both request and response size ADUs.  The ADUs are saved in 
 ADU container and both the client and server have one copy of it.  The request ADU and response ADU are saved in sequence inside the ADU
 container.
 

Working Modes
*****************

 There are two working modes: internet-like and user-defined. When internet-like mode is used, it generates the variables based
 on predefined distribution functions. While the user-defined mode will need the users to pass in variables for the
 model to generate HTTP traffic.

 This module does not use the built in tracing module of ns-3, it generated its own files to record the parameters generated from
 distribution functions as well as the simulation results.  The reason for this choice is that some of the metrics we use cannot be represented
 by the ns-3 built in tracing module.  For example, the Web Page Delivery Delay needs to identify the time it takes a whole HTTP object 
 to deliver to the destination.  The build tracing module in ns-3 can only monitor the time it takes for the single transport segment to reach
 the destination.  This is the reason we choose to use the text-based tracing of the module.  Furthermore, the tracing module is carefully tested 
 and validated. 

 There are two metrics to record the results:
 1. Object Delivery Ratio (ODR)
 2. Web Page Latency

 Notes:
 The time recorded as parameters are all in milliseconds

HTTP Instructions
*****************

To have a node running HTTP, the easiest way would be to use the HttpHelper class in your simulation script. For instance::

  HttpHelper http;
  http.Install (http, adhocNodes);

The example scripts inside ``src/http/examples/`` demonstrate the use of HTTP based nodes
in different scenarios. The helper source can be found inside ``src/http/helper/http-helper.{h,cc}``

Examples
********

The example can be found in ``src/http/examples/``:

* http.cc use HTTP as traffic generator.  This example uses basic settings to make it simple to use.  Advanced settings have been commented out.


Testing
**********

This model has been tested as follows:

* Unit tests have been written to verify the internals of HTTP. This can be found in ``src/http/test/http-test-suite.cc``. These tests verify whether the methods inside DSR module which deal with packet buffer, headers work correctly.
* Random variable validation cases have been conducted to fit the proposed distribution function for each http parameter
* For each http parameter 
  
References
**********

[1] BibTeX for the http performance paper: 
@inproceedings{cheng:2011:PCO,
	Address = {Las Vegas, NV},
	Author = {Yufei Cheng and Egemen K. \c{C}etinkaya and James P.G. Sterbenz},
	Booktitle = {Proceedings of the International Telemetering Conference ({ITC})},
	Month = {October},
	Title = {Performance Comparison of Routing Protocols for Transactional Traffic over Aeronautical Networks},
	Year = {2011}}

