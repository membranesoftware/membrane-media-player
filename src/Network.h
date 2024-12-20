/*
* Membrane Software Reference Source License
* Version 2024 Sep 18
* This license is a legal agreement between you and Membrane Software
*
* This license agreement governs use of the accompanying source code. If you use the source code, you accept this license. If you do not accept the license, do not use the source code.
*
* DEFINITIONS
* "compilation" means to compile the code from source code to machine code.
* “non-commercial distribution” means distribution of the code or any compilation of the code, or of any other application or program containing the code or any compilation of the code, where such distribution is not intended for or directed towards commercial advantage or monetary compensation.
* "review" means to access, analyse, test and otherwise review the code as a reference
* "you" means the licensee of rights set out in this license.
*
* GRANT OF RIGHTS
* Subject to the terms of this license, we grant you a non-transferable, non-exclusive, worldwide, royalty-free license to access and use the source code solely for the purposes of review, compilation and non-commercial distribution.
*
* LIMITATIONS
* This license does not grant you any rights to use Membrane Software's name, logo, or trademarks.
*
* If you issue proceedings in any jurisdiction against Membrane Software because you consider Membrane Software has infringed copyright or any patent right in respect of the code (including any joinder or counterclaim), your license to the code is automatically terminated.
*
* This source code is provided by the copyright holders and contributors "as is" and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the copyright holder or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this source code, even if advised of the possibility of such damage.
*
* NO IMPLIED RIGHTS
* All rights not expressly granted by Membrane Software to you in this License Agreement are hereby reserved by Membrane Software and its suppliers. There are no implied rights in this License Agreement.
*
* GOVERNING LAW
* This EULA agreement, and any dispute arising out of or in connection with this EULA agreement, shall be governed by and construed in accordance with the laws of Washington State, United States of America.
*
* QUESTIONS OR ADDITIONAL INFORMATION
* If you have questions regarding this License Agreement, please contact Membrane Software by sending an email to support@membranesoftware.com.
*/
// Class that handles network interfaces and sockets
#ifndef NETWORK_H
#define NETWORK_H

#include "curl/curl.h"

class Buffer;
class SharedBuffer;
class StringList;

class Network {
public:
	Network ();
	~Network ();
	static Network *instance;

	// Initialize static instance data
	static void createInstance ();

	// Clear static instance data
	static void freeInstance ();

	static constexpr const int defaultMaxRequestThreads = 3;
	static constexpr const char *localhostAddress = "127.0.0.1";

	static constexpr const int HttpOkCode = 200;
	static constexpr const int HttpUnauthorizedCode = 401;
	static constexpr const char *HttpAcceptHeader = "Accept";
	static constexpr const char *HttpContentTypeHeader = "Content-Type";
	static constexpr const char *HttpHostHeader = "Host";
	static constexpr const char *ApplicationJsonContentType = "application/json";
	static constexpr const char *WwwFormUrlencodedContentType = "application/x-www-form-urlencoded";

	typedef void (*DatagramCallback) (void *callbackData, const char *messageData, int messageLength, const char *sourceAddress, int sourcePort);
	typedef void (*HttpRequestCallback) (void *callbackData, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);

	struct DatagramCallbackContext {
		Network::DatagramCallback callback;
		void *callbackData;
		DatagramCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		DatagramCallbackContext (Network::DatagramCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};
	struct HttpRequestCallbackContext {
		Network::HttpRequestCallback callback;
		void *callbackData;
		HttpRequestCallbackContext ():
			callback (NULL),
			callbackData (NULL) { }
		HttpRequestCallbackContext (Network::HttpRequestCallback callback, void *callbackData):
			callback (callback),
			callbackData (callbackData) { }
	};

	// Read-write data members
	int maxRequestThreads;
	StdString httpUserAgent;
	bool allowUnverifiedHttps;
	bool enableDatagramSocket;
	Network::DatagramCallbackContext datagramCallback;

	// Read-only data members
	bool isStarted;
	bool isStopped;
	int datagramPort;

	// Initialize networking functionality and acquire resources as needed. Returns a Result value.
	OpResult start ();

	// Stop the networking engine and release acquired resources
	void stop ();

	// Return a boolean value indicating if the networking engine has completed its stop operation and its child threads have terminated
	bool isStopComplete ();

	// Join the networking engine's child threads, blocking until the operation completes
	void waitThreads ();

	// Return a string containing the address of the primary network interface, or an empty string if no such address was found
	StdString getPrimaryInterfaceAddress ();

	// Send a datagram packet to a remote host using data from the provided buffer. This class becomes responsible for freeing messageData when it's no longer needed.
	void sendDatagram (const StdString &targetHostname, int targetPort, Buffer *messageData);

	// Send a datagram packet to all known broadcast addresses using data from the provided buffer. This class becomes responsible for freeing messageData when it's no longer needed.
	void sendBroadcastDatagram (int targetPort, Buffer *messageData);

	// Send an HTTP GET request and invoke the provided callback when complete. If headerList is provided, apply each pair of contained items as the name and value of a request header.
	void sendHttpGet (const StdString &targetUrl, Network::HttpRequestCallbackContext callback, const StringList &headerList = StringList ());

	// Send an HTTP POST request and invoke the provided callback when complete. If headerList is provided, apply each pair of contained items as the name and value of a request header.
	void sendHttpPost (const StdString &targetUrl, const StdString &postData, Network::HttpRequestCallbackContext callback, const StringList &headerList = StringList ());

private:
	// Run a thread that sends datagrams submitted by outside callers
	static int runDatagramSendThread (void *itPtr);

	// Run a thread that receives messages from datagramSocket
	static int runDatagramReceiveThread (void *itPtr);

	// Run a thread that sends HTTP requests submitted by outside callers
	static int runHttpRequestThread (void *itPtr);

	struct Interface {
		int id;
		bool isUp;
		bool isBroadcast;
		bool isLoopback;
		StdString address;
		StdString broadcastAddress;
		Interface ():
			id (0),
			isUp (false),
			isBroadcast (false),
			isLoopback (false) { }
	};
	struct Datagram {
		StdString targetHostname;
		int targetPort;
		Buffer *messageData;
		bool isBroadcast;
		Datagram ():
			targetPort (0),
			messageData (NULL),
			isBroadcast (false) { }
	};
	struct HttpRequestContext {
		StdString method;
		StdString url;
		StdString postData;
		StringList headerList;
		Network::HttpRequestCallbackContext callback;
		HttpRequestContext ():
			method ("GET") { }
	};

	// Populate the interface map with data regarding available network interfaces. Returns a Result value.
	OpResult resetInterfaces ();

	// Remove all items from the datagram queue
	void clearDatagramQueue ();

	// Remove all items from the HTTP request queue
	void clearHttpRequestQueue ();

	// Wait all HTTP request threads
	void waitHttpRequestThreads ();

	// Execute a sendto call to transmit a datagram packet
	int sendTo (const StdString &targetHostname, int targetPort, Buffer *messageData);

	// Execute sendto calls to transmit a datagram packet to each available broadcast address
	int broadcastSendTo (int targetPort, Buffer *messageData);

	// Execute operations to send an HTTP request and gather the response data. Returns a Result value. If successful, this method stores values in the provided pointers, and the caller is responsible for releasing any created SharedBuffer object.
	OpResult sendHttpRequest (Network::HttpRequestContext *item, int *statusCode, SharedBuffer **responseBuffer);

	// Callback functions for use with libcurl
	static size_t curlWrite (char *ptr, size_t size, size_t nmemb, void *userdata);
	static int curlProgress (void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

	std::map<StdString, Network::Interface> interfaceMap;
	SDL_Thread *datagramSendThread;
	SDL_Thread *datagramReceiveThread;
	std::queue<Network::Datagram> datagramQueue;
	SDL_mutex *datagramSendMutex;
	SDL_cond *datagramSendCond;
	int datagramSocket;
	std::queue<Network::HttpRequestContext> httpRequestQueue;
	std::list<Network::HttpRequestContext> httpShutdownList;
	SDL_mutex *httpRequestQueueMutex;
	SDL_cond *httpRequestQueueCond;
	std::list<SDL_Thread *> httpRequestThreadList;
	int httpRequestThreadStopCount;
	StdString caBundle;
	struct curl_blob caBundleBlob;
#if PLATFORM_WINDOWS
	bool isWsaStarted;
#endif
};
#endif
