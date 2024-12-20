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
#include "Config.h"
#include <errno.h>
#include <sys/types.h>
#if PLATFORM_LINUX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#endif
#if PLATFORM_MACOS
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif
#if PLATFORM_WINDOWS
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <Ipifcons.h>
#endif
#include "curl/curl.h"
#include "openssl/ssl.h"
#include "App.h"
#include "SdlUtil.h"
#include "Log.h"
#include "Buffer.h"
#include "SharedBuffer.h"
#include "StringList.h"
#include "Ipv4Address.h"
#include "Resource.h"
#include "Network.h"

Network *Network::instance = NULL;
constexpr const int maxDatagramSize = 1500; // bytes
constexpr const char *caBundlePath = "tls/cert.pem";

Network::Network ()
: maxRequestThreads (Network::defaultMaxRequestThreads)
, allowUnverifiedHttps (false)
, enableDatagramSocket (false)
, isStarted (false)
, isStopped (false)
, datagramPort (0)
, datagramSendThread (NULL)
, datagramReceiveThread (NULL)
, datagramSocket (-1)
, httpRequestThreadStopCount (0)
#if PLATFORM_WINDOWS
, isWsaStarted (false)
#endif
{
	SdlUtil::createMutex (&datagramSendMutex);
	SdlUtil::createCond (&datagramSendCond);
	SdlUtil::createMutex (&httpRequestQueueMutex);
	SdlUtil::createCond (&httpRequestQueueCond);

	caBundleBlob.data = NULL;
	caBundleBlob.len = 0;
	caBundleBlob.flags = 0;
}
Network::~Network () {
	stop ();
	SdlUtil::destroyCond (&datagramSendCond);
	SdlUtil::destroyMutex (&datagramSendMutex);
	SdlUtil::destroyCond (&httpRequestQueueCond);
	SdlUtil::destroyMutex (&httpRequestQueueMutex);
}

void Network::createInstance () {
	if (! Network::instance) {
		Network::instance = new Network ();
	}
}
void Network::freeInstance () {
	if (Network::instance) {
		delete (Network::instance);
		Network::instance = NULL;
	}
}

void Network::clearDatagramQueue () {
	Network::Datagram item;

	SDL_LockMutex (datagramSendMutex);
	while (! datagramQueue.empty ()) {
		item = datagramQueue.front ();
		if (item.messageData) {
			delete (item.messageData);
			item.messageData = NULL;
		}
		datagramQueue.pop ();
	}
	SDL_CondBroadcast (datagramSendCond);
	SDL_UnlockMutex (datagramSendMutex);
}

void Network::clearHttpRequestQueue () {
	SDL_LockMutex (httpRequestQueueMutex);
	while (! httpRequestQueue.empty ()) {
		httpRequestQueue.pop ();
	}
	SDL_CondBroadcast (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
}

void Network::waitHttpRequestThreads () {
	std::list<SDL_Thread *>::iterator i1, i2;
	int result;

	SDL_LockMutex (httpRequestQueueMutex);
	SDL_CondBroadcast (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);

	i1 = httpRequestThreadList.begin ();
	i2 = httpRequestThreadList.end ();
	while (i1 != i2) {
		SDL_WaitThread (*i1, &result);
		++i1;
	}
	httpRequestThreadList.clear ();
}

OpResult Network::start () {
	Buffer *buffer;
	struct sockaddr_in saddr;
	OpResult result;
	int i, cresult;
	socklen_t namelen;
	SDL_Thread *thread;
#if PLATFORM_LINUX || PLATFORM_MACOS
	int sockopt;
	struct protoent *proto;
#endif
#if PLATFORM_WINDOWS
	char sockopt;
	WORD versionrequested;
	WSADATA wsadata;
#endif

	if (isStarted) {
		return (OpResult::Success);
	}
	if (maxRequestThreads <= 0) {
		maxRequestThreads = Network::defaultMaxRequestThreads;
	}
#if PLATFORM_WINDOWS
	if (! isWsaStarted) {
		versionrequested = MAKEWORD (2, 2);
		cresult = WSAStartup (versionrequested, &wsadata);
		if (cresult != 0) {
			Log::err ("Network start failed; err=\"WSAStartup: %i\"", cresult);
			return (OpResult::SocketOperationFailedError);
		}
		Log::debug ("WSAStartup; wsaVersion=%i.%i", HIBYTE (wsadata.wVersion), LOBYTE (wsadata.wVersion));
		isWsaStarted = true;
	}
#endif

	buffer = Resource::instance->loadFile (StdString (caBundlePath));
	if (! buffer) {
		Log::debug ("Network initialization error: failed to load file resource");
	}
	else {
		caBundle.assignBuffer (buffer);
		Resource::instance->unloadFile (StdString (caBundlePath));
		caBundleBlob.data = (void *) caBundle.c_str ();
		caBundleBlob.len = caBundle.length ();
		caBundleBlob.flags = CURL_BLOB_COPY;
	}

	SSL_library_init ();
	cresult = curl_global_init (CURL_GLOBAL_ALL);
	if (cresult != 0) {
		return (OpResult::LibcurlOperationFailedError);
	}
	result = resetInterfaces ();
	if (result != OpResult::Success) {
		return (result);
	}
	datagramSocket = -1;
	datagramPort = 0;
	if (enableDatagramSocket) {
#if PLATFORM_LINUX || PLATFORM_MACOS
		proto = getprotobyname ("udp");
		if (! proto) {
			Log::err ("Network start failed; err=\"getprotobyname: %s\"", strerror (errno));
			return (OpResult::SocketOperationFailedError);
		}
		datagramSocket = socket (PF_INET, SOCK_DGRAM, proto->p_proto);
		endprotoent ();
#endif
#if PLATFORM_WINDOWS
		datagramSocket = socket (AF_INET, SOCK_DGRAM, 0);
#endif
		if (datagramSocket < 0) {
			Log::err ("Network start failed; err=\"socket: %s\"", strerror (errno));
			return (OpResult::SocketOperationFailedError);
		}

		sockopt = 1;
		if (setsockopt (datagramSocket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof (sockopt)) < 0) {
			Log::err ("Network start failed; err=\"setsockopt SO_REUSEADDR: %s\"", strerror (errno));
			return (OpResult::SocketOperationFailedError);
		}
		sockopt = 1;
		if (setsockopt (datagramSocket, SOL_SOCKET, SO_BROADCAST, &sockopt, sizeof (sockopt)) < 0) {
			Log::err ("Network start failed; err=\"setsockopt SO_BROADCAST: %s\"", strerror (errno));
			return (OpResult::SocketOperationFailedError);
		}

		memset (&saddr, 0, sizeof (struct sockaddr_in));
		saddr.sin_family = AF_INET;
		saddr.sin_addr.s_addr = htonl (INADDR_ANY);
		if (bind (datagramSocket, (struct sockaddr *) (&saddr), sizeof (struct sockaddr_in)) < 0) {
			Log::err ("Network start failed; err=\"bind: %s\"", strerror (errno));
			return (OpResult::SocketOperationFailedError);
		}

		memset (&saddr, 0, sizeof (struct sockaddr_in));
		namelen = sizeof (struct sockaddr_in);
		if (getsockname (datagramSocket, (struct sockaddr *) &saddr, &namelen) < 0) {
			Log::err ("Network start failed; err=\"getsockname: %s\"", strerror (errno));
			return (OpResult::SocketOperationFailedError);
		}
		datagramPort = (int) ntohs (saddr.sin_port);

		datagramReceiveThread = SDL_CreateThread (Network::runDatagramReceiveThread, "runDatagramReceiveThread", this);
		if (! datagramReceiveThread) {
			Log::err ("Network start failed; err=\"thread create failed\"");
			return (OpResult::ThreadCreateFailedError);
		}
		datagramSendThread = SDL_CreateThread (Network::runDatagramSendThread, "runDatagramSendThread", this);
		if (! datagramSendThread) {
			Log::err ("Network start failed; err=\"thread create failed\"");
			return (OpResult::ThreadCreateFailedError);
		}
	}

	for (i = 0; i < maxRequestThreads; ++i) {
		thread = SDL_CreateThread (Network::runHttpRequestThread, StdString::createSprintf ("runHttpRequestThread_%i", i).c_str (), this);
		if (! thread) {
			return (OpResult::ThreadCreateFailedError);
		}
		httpRequestThreadList.push_back (thread);
	}

	isStarted = true;
	Log::debug ("Network start; datagramSocket=%i datagramPort=%i maxRequestThreads=%i", datagramSocket, datagramPort, maxRequestThreads);
	return (OpResult::Success);
}

void Network::stop () {
	Network::HttpRequestContext item;

#if PLATFORM_WINDOWS
	if (isWsaStarted) {
		isWsaStarted = false;
		WSACleanup ();
	}
#endif
	if ((! isStarted) || isStopped) {
		return;
	}
	isStopped = true;
	if (datagramSocket >= 0) {
		shutdown (datagramSocket, SHUT_RDWR);
#if PLATFORM_WINDOWS
		closesocket (datagramSocket);
#else
		close (datagramSocket);
#endif
		datagramSocket = -1;
	}
	clearDatagramQueue ();

	httpShutdownList.clear ();
	SDL_LockMutex (httpRequestQueueMutex);
	while (! httpRequestQueue.empty ()) {
		item = httpRequestQueue.front ();
		httpShutdownList.push_back (item);
		httpRequestQueue.pop ();
	}
	SDL_CondBroadcast (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
	curl_global_cleanup ();
}

void Network::waitThreads () {
	std::list<Network::HttpRequestContext>::iterator i1, i2;
	int result;

	clearDatagramQueue ();
	clearHttpRequestQueue ();

	i1 = httpShutdownList.begin ();
	i2 = httpShutdownList.end ();
	while (i1 != i2) {
		if (i1->callback.callback) {
			i1->callback.callback (i1->callback.callbackData, i1->url, 0, NULL);
		}
		++i1;
	}
	httpShutdownList.clear ();

	waitHttpRequestThreads ();
	if (datagramReceiveThread) {
		SDL_WaitThread (datagramReceiveThread, &result);
		datagramReceiveThread = NULL;
	}
	if (datagramSendThread) {
		SDL_WaitThread (datagramSendThread, &result);
		datagramSendThread = NULL;
	}
}

bool Network::isStopComplete () {
	return (isStopped && (httpRequestThreadStopCount >= maxRequestThreads));
}

OpResult Network::resetInterfaces () {
#if PLATFORM_LINUX
	struct ifreq req, *i, *end;
	struct ifconf conf;
	struct sockaddr_in *addr;
	char confbuf[1024], addrbuf[1024];
	int fd, id;
	OpResult result;
	StdString name;
	Network::Interface interface;

	fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (fd < 0) {
		Log::err ("Failed to detect network interfaces; err=\"socket: %s\"", strerror (errno));
		close (fd);
		return (OpResult::SocketOperationFailedError);
	}

	conf.ifc_len = sizeof (confbuf);
	conf.ifc_buf = confbuf;
	if (ioctl (fd, SIOCGIFCONF, &conf) < 0) {
		Log::err ("Failed to detect network interfaces; err=\"ioctl SIOCGIFCONF: %s\"", strerror (errno));
		close (fd);
		return (OpResult::SocketOperationFailedError);
	}

	interfaceMap.clear ();
	id = 0;
	result = OpResult::Success;
	i = conf.ifc_req;
	end = i + (conf.ifc_len / sizeof (struct ifreq));
	while (i != end) {
		strncpy (req.ifr_name, i->ifr_name, sizeof (req.ifr_name) - 1);
		++i;

		name.assign (req.ifr_name);
		if (ioctl (fd, SIOCGIFADDR, &req) < 0) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"ioctl SIOCGIFADDR: %s\"", name.c_str (), strerror (errno));
			continue;
		}
		if (req.ifr_addr.sa_family != AF_INET) {
			Log::debug ("Skip network interface (not AF_INET); name=\"%s\"", name.c_str ());
			continue;
		}
		addr = (struct sockaddr_in *) &(req.ifr_addr);
		if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
			continue;
		}
		interface.address.assign (addrbuf);

		if (ioctl (fd, SIOCGIFFLAGS, &req) < 0) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"ioctl SIOCGIFFLAGS: %s\"", name.c_str (), strerror (errno));
			continue;
		}
		interface.isUp = (req.ifr_flags & IFF_UP) ? true : false;
		interface.isBroadcast = (req.ifr_flags & IFF_BROADCAST) ? true : false;
		interface.isLoopback = (req.ifr_flags & IFF_LOOPBACK) ? true : false;

		if (! interface.isLoopback) {
			if (! interface.isBroadcast) {
				interface.broadcastAddress.assign ("");
			}
			else {
				if (ioctl (fd, SIOCGIFBRDADDR, &req) < 0) {
					Log::warning ("Failed to read network interface; name=\"%s\" err=\"ioctl SIOCGIFBRDADDR: %s\"", name.c_str (), strerror (errno));
					continue;
				}
				addr = (struct sockaddr_in *) &(req.ifr_broadaddr);
				if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
					Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
					continue;
				}
				interface.broadcastAddress.assign (addrbuf);
			}
		}

		interface.id = id;
		++id;
		Log::debug ("Detected network interface; id=%i name=\"%s\" isUp=%s isBroadcast=%s isLoopback=%s address=%s broadcastAddress=%s", interface.id, name.c_str (), BOOL_STRING (interface.isUp), BOOL_STRING (interface.isBroadcast), BOOL_STRING (interface.isLoopback), interface.address.c_str (), interface.broadcastAddress.c_str ());
		interfaceMap.insert (std::pair<StdString, Network::Interface> (name, interface));
	}

	close (fd);
	return (result);
#endif
#if PLATFORM_MACOS
	StdString name;
	Network::Interface interface;
	struct ifaddrs *ifp, *item;
	struct sockaddr_in *addr;
	char addrbuf[1024];
	int result, id;

	result = getifaddrs (&ifp);
	if (result != 0) {
		Log::warning ("Failed to detect network interfaces; err=\"getifaddrs: %s\"", strerror (errno));
		return (OpResult::SocketOperationFailedError);
	}
	id = 0;
	item = ifp;
	while (item) {
		name.assign (item->ifa_name);
		if (item->ifa_addr->sa_family != AF_INET) {
			Log::debug ("Skip network interface (not AF_INET); name=\"%s\"", name.c_str ());
			item = item->ifa_next;
			continue;
		}
		addr = (struct sockaddr_in *) item->ifa_addr;
		if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
			Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
			item = item->ifa_next;
			continue;
		}
		interface.address.assign (addrbuf);

		interface.isUp = (item->ifa_flags & IFF_UP) ? true : false;
		interface.isBroadcast = (item->ifa_flags & IFF_BROADCAST) ? true : false;
		interface.isLoopback = (item->ifa_flags & IFF_LOOPBACK) ? true : false;

		if (! interface.isLoopback) {
			if (! interface.isBroadcast) {
				interface.broadcastAddress.assign ("");
			}
			else {
				addr = (struct sockaddr_in *) item->ifa_dstaddr;
				if (! inet_ntop (AF_INET, &(addr->sin_addr), addrbuf, sizeof (addrbuf))) {
					Log::warning ("Failed to read network interface; name=\"%s\" err=\"inet_ntop: %s\"", name.c_str (), strerror (errno));
					item = item->ifa_next;
					continue;
				}
				interface.broadcastAddress.assign (addrbuf);
			}
		}

		interface.id = id;
		++id;
		Log::debug ("Detected network interface; id=%i name=\"%s\" isUp=%s isBroadcast=%s isLoopback=%s address=%s broadcastAddress=%s", interface.id, name.c_str (), BOOL_STRING (interface.isUp), BOOL_STRING (interface.isBroadcast), BOOL_STRING (interface.isLoopback), interface.address.c_str (), interface.broadcastAddress.c_str ());
		interfaceMap.insert (std::pair<StdString, Network::Interface> (name, interface));

		item = item->ifa_next;
	}

	freeifaddrs (ifp);
	return (OpResult::Success);
#endif
#if PLATFORM_WINDOWS
	PMIB_IPADDRTABLE table;
	DWORD sz, retval;
	Ipv4Address ipaddr;
	StdString name;
	Network::Interface interface;
	IN_ADDR inaddr;
	int i, id;

	sz = 0;
	retval = 0;
	table = (MIB_IPADDRTABLE *) malloc (sizeof (MIB_IPADDRTABLE));
	if (! table) {
		Log::err ("Failed to detect network interfaces (out of memory)");
		return (OpResult::OutOfMemoryError);
	}
	retval = GetIpAddrTable (table, &sz, 0);
	if (retval == ERROR_INSUFFICIENT_BUFFER) {
		free (table);
		table = (MIB_IPADDRTABLE *) malloc (sz);
		if (! table) {
			Log::err ("Failed to detect network interfaces (out of memory)");
			return (OpResult::OutOfMemoryError);
		}
		retval = GetIpAddrTable (table, &sz, 0);
	}
	if (retval != NO_ERROR) {
		free (table);
		Log::err ("Failed to detect network interfaces (GetIpAddrTable); result=%i", (int) retval);
		return (OpResult::SystemOperationFailedError);
	}

	id = 0;
	for (i = 0; i < (int) table->dwNumEntries; i++) {
		name.sprintf ("%i", table->table[i].dwIndex);
		inaddr.S_un.S_addr = (u_long) table->table[i].dwAddr;
		ipaddr.parse (inet_ntoa (inaddr));
		inaddr.S_un.S_addr = (u_long) table->table[i].dwMask;
		ipaddr.parseNetmask (inet_ntoa (inaddr));

		interface.address.assign (ipaddr.toString ());
		interface.broadcastAddress.assign (ipaddr.getBroadcastAddress ());

		interface.isUp = false;
		interface.isLoopback = ipaddr.isLocalhost ();
		interface.isBroadcast = (! interface.broadcastAddress.equals (interface.address));
		if (table->table[i].wType & MIB_IPADDR_PRIMARY) {
			interface.isUp = true;
		}

		interface.id = id;
		++id;
		Log::debug ("Detected network interface; id=%i name=\"%s\" isUp=%s isBroadcast=%s isLoopback=%s address=%s broadcastAddress=%s", interface.id, name.c_str (), BOOL_STRING (interface.isUp), BOOL_STRING (interface.isBroadcast), BOOL_STRING (interface.isLoopback), interface.address.c_str (), interface.broadcastAddress.c_str ());
		interfaceMap.insert (std::pair<StdString, Network::Interface> (name, interface));
	}

	if (table) {
		free (table);
		table = NULL;
	}
	return (OpResult::Success);
#endif
}

StdString Network::getPrimaryInterfaceAddress () {
	std::map<StdString, Network::Interface>::iterator i1, i2;
	StdString address;
	Network::Interface *interface;
	int minid;

	minid = -1;
	i1 = interfaceMap.begin ();
	i2 = interfaceMap.end ();
	while (i1 != i2) {
		interface = &(i1->second);
		if (interface->isUp && (! interface->isLoopback) && interface->isBroadcast && (! interface->address.empty ())) {
			if ((minid < 0) || (interface->id < minid)) {
				minid = interface->id;
				address = interface->address;
			}
		}
		++i1;
	}
	return (address);
}

int Network::runDatagramReceiveThread (void *itPtr) {
	Network *it = (Network *) itPtr;
	struct sockaddr_in srcaddr;
	socklen_t addrlen;
	int msglen;
	char buf[maxDatagramSize], host[NI_MAXHOST];

	while (true) {
		if (it->isStopped || (it->datagramSocket < 0)) {
			break;
		}
		addrlen = sizeof (struct sockaddr_in);
		msglen = recvfrom (it->datagramSocket, buf, sizeof (buf), 0, (struct sockaddr *) &srcaddr, &addrlen);
		if (msglen < 0) {
			break;
		}
		if (msglen == 0) {
			break;
		}
		if (getnameinfo ((struct sockaddr *) &srcaddr, addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			memset (host, 0, sizeof (host));
		}
		if (it->datagramCallback.callback) {
			it->datagramCallback.callback (it->datagramCallback.callbackData, buf, msglen, host, (int) srcaddr.sin_port);
		}
	}

	return (0);
}

int Network::runDatagramSendThread (void *itPtr) {
	Network *it = (Network *) itPtr;
	Network::Datagram item;
	int result;

	SDL_LockMutex (it->datagramSendMutex);
	while (true) {
		if (it->isStopped || (it->datagramSocket < 0)) {
			break;
		}
		if (it->datagramQueue.empty ()) {
			SDL_CondWait (it->datagramSendCond, it->datagramSendMutex);
			continue;
		}

		item = it->datagramQueue.front ();
		it->datagramQueue.pop ();
		SDL_UnlockMutex (it->datagramSendMutex);

		if (! item.messageData) {
			Log::warning ("Discard queued datagram (no message data provided)");
		}
		else {
			if (item.isBroadcast) {
				result = it->broadcastSendTo (item.targetPort, item.messageData);
			}
			else {
				result = it->sendTo (item.targetHostname, item.targetPort, item.messageData);
			}

			if (result != OpResult::Success) {
				Log::debug ("Failed to send datagram; err=%i", result);
			}
			delete (item.messageData);
			item.messageData = NULL;
		}

		SDL_LockMutex (it->datagramSendMutex);
	}
	SDL_UnlockMutex (it->datagramSendMutex);

	return (0);
}

void Network::sendDatagram (const StdString &targetHostname, int targetPort, Buffer *messageData) {
	Network::Datagram item;

	item.targetHostname.assign (targetHostname);
	item.targetPort = targetPort;
	item.messageData = messageData;
	SDL_LockMutex (datagramSendMutex);
	datagramQueue.push (item);
	SDL_CondSignal (datagramSendCond);
	SDL_UnlockMutex (datagramSendMutex);
}

void Network::sendBroadcastDatagram (int targetPort, Buffer *messageData) {
	Network::Datagram item;

	item.targetPort = targetPort;
	item.messageData = messageData;
	item.isBroadcast = true;
	SDL_LockMutex (datagramSendMutex);
	datagramQueue.push (item);
	SDL_CondSignal (datagramSendCond);
	SDL_UnlockMutex (datagramSendMutex);
}

void Network::sendHttpGet (const StdString &targetUrl, Network::HttpRequestCallbackContext callback, const StringList &headerList) {
	Network::HttpRequestContext item;

	item.method.assign ("GET");
	item.url.assign (targetUrl);
	item.callback = callback;
	item.headerList.assign (headerList);
	SDL_LockMutex (httpRequestQueueMutex);
	httpRequestQueue.push (item);
	SDL_CondSignal (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
}

void Network::sendHttpPost (const StdString &targetUrl, const StdString &postData, Network::HttpRequestCallbackContext callback, const StringList &headerList) {
	Network::HttpRequestContext item;

	item.method.assign ("POST");
	item.url.assign (targetUrl);
	item.postData.assign (postData);
	item.callback = callback;
	item.headerList.assign (headerList);
	SDL_LockMutex (httpRequestQueueMutex);
	httpRequestQueue.push (item);
	SDL_CondSignal (httpRequestQueueCond);
	SDL_UnlockMutex (httpRequestQueueMutex);
}

int Network::sendTo (const StdString &targetHostname, int targetPort, Buffer *messageData) {
	StdString portstr;
	struct addrinfo hints;
	struct addrinfo *addr;
	int result;

	if ((! isStarted) || (datagramSocket < 0)) {
		return (OpResult::SocketNotConnectedError);
	}
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	portstr.sprintf ("%i", targetPort);
	result = getaddrinfo (targetHostname.c_str (), portstr.c_str (), &hints, &addr);
	if (result != 0) {
		return (OpResult::UnknownHostnameError);
	}
	if (! addr) {
		return (OpResult::SocketOperationFailedError);
	}

	result = sendto (datagramSocket, (char *) messageData->data, messageData->length, 0, addr->ai_addr, addr->ai_addrlen);
	freeaddrinfo (addr);
	if (result < 0) {
		return (OpResult::SocketOperationFailedError);
	}
	return (OpResult::Success);
}

int Network::broadcastSendTo (int targetPort, Buffer *messageData) {
	std::map<StdString, Network::Interface>::iterator i1, i2;
	Network::Interface *interface;
	int result, sendtoresult, successcount;

	result = OpResult::Success;
	successcount = 0;
	i1 = interfaceMap.begin ();
	i2 = interfaceMap.end ();
	while (i1 != i2) {
		interface = &(i1->second);
		if (interface->isBroadcast && interface->isUp && (! interface->isLoopback) && (! interface->broadcastAddress.empty ()) && (! interface->broadcastAddress.equals ("0.0.0.0"))) {
			sendtoresult = sendTo (interface->broadcastAddress, targetPort, messageData);
			if (sendtoresult == OpResult::Success) {
				++successcount;
			}
			else {
				result = sendtoresult;
			}
		}
		++i1;
	}
	if ((result != OpResult::Success) && (successcount > 0)) {
		result = OpResult::Success;
	}
	return (result);
}

int Network::runHttpRequestThread (void *itPtr) {
	Network *it = (Network *) itPtr;
	Network::HttpRequestContext item;
	int result, statuscode;
	SharedBuffer *responsebuffer;

	SDL_LockMutex (it->httpRequestQueueMutex);
	while (true) {
		if (it->isStopped) {
			break;
		}
		if (it->httpRequestQueue.empty ()) {
			SDL_CondWait (it->httpRequestQueueCond, it->httpRequestQueueMutex);
			continue;
		}
		item = it->httpRequestQueue.front ();
		it->httpRequestQueue.pop ();
		SDL_UnlockMutex (it->httpRequestQueueMutex);

		statuscode = 0;
		responsebuffer = NULL;
		result = it->sendHttpRequest (&item, &statuscode, &responsebuffer);
		if (result != OpResult::Success) {
			statuscode = 0;
		}
		if (item.callback.callback) {
			item.callback.callback (item.callback.callbackData, item.url, statuscode, responsebuffer);
		}
		if (responsebuffer) {
			responsebuffer->release ();
			responsebuffer = NULL;
		}

		SDL_LockMutex (it->httpRequestQueueMutex);
	}
	++(it->httpRequestThreadStopCount);
	SDL_UnlockMutex (it->httpRequestQueueMutex);

	return (0);
}

OpResult Network::sendHttpRequest (Network::HttpRequestContext *item, int *statusCode, SharedBuffer **responseBuffer) {
	CURL *curl;
	struct curl_slist *headers;
	CURLcode code;
	SharedBuffer *buffer;
	StringList::const_iterator i1, i2;
	StdString headertext;
	long responsecode;
	OpResult result;

	curl = curl_easy_init ();
	if (! curl) {
		return (OpResult::LibcurlOperationFailedError);
	}
	result = OpResult::Success;
	responsecode = 0;
	headers = NULL;
	code = CURLE_UNKNOWN_OPTION;
	buffer = new SharedBuffer ();
	buffer->retain ();
	curl_easy_setopt (curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, Network::curlWrite);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, buffer);
	curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, Network::curlProgress);

	if (! item->headerList.empty ()) {
		i1 = item->headerList.cbegin ();
		i2 = item->headerList.cend ();
		while (i1 != i2) {
			headertext = *i1;
			++i1;
			if (i1 == i2) {
				break;
			}
			headertext.append (": ");
			headertext.append (*i1);
			++i1;
			headers = curl_slist_append (headers, headertext.c_str ());
		}
		curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
	}

	curl_easy_setopt (curl, CURLOPT_URL, item->url.c_str ());
	if (! httpUserAgent.empty ()) {
		curl_easy_setopt (curl, CURLOPT_USERAGENT, httpUserAgent.c_str ());
	}

	if (item->url.startsWith ("https://")) {
		curl_easy_setopt (curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
		if (allowUnverifiedHttps) {
			curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt (curl, CURLOPT_SSL_VERIFYHOST, 0);
		}
		else {
			if (caBundleBlob.data) {
				curl_easy_setopt (curl, CURLOPT_CAINFO_BLOB, &caBundleBlob);
			}
		}
	}

	if (item->method.equals ("GET")) {
		code = curl_easy_perform (curl);
	}
	else if (item->method.equals ("POST")) {
		curl_easy_setopt (curl, CURLOPT_POST, 1);
		curl_easy_setopt (curl, CURLOPT_POSTFIELDS, item->postData.c_str ());
		curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, item->postData.length ());
		code = curl_easy_perform (curl);
	}
	else {
		result = OpResult::UnknownMethodError;
	}

	if (result == OpResult::Success) {
		if (code != CURLE_OK) {
			result = OpResult::LibcurlOperationFailedError;
		}
	}

	if (result != OpResult::Success) {
		delete (buffer);
	}
	else {
		code = curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &responsecode);
		if (code == CURLE_OK) {
			if (statusCode) {
				*statusCode = (int) responsecode;
			}
		}
		if (responseBuffer) {
			*responseBuffer = buffer;
		}
		else {
			delete (buffer);
		}
	}

	curl_easy_cleanup (curl);
	if (headers) {
		curl_slist_free_all (headers);
		headers = NULL;
	}
	return (result);
}

size_t Network::curlWrite (char *ptr, size_t size, size_t nmemb, void *userdata) {
	SharedBuffer *buffer;
	size_t total;

	buffer = (SharedBuffer *) userdata;
	total = size * nmemb;
	buffer->add ((uint8_t *) ptr, (int) total);
	return (total);
}

int Network::curlProgress (void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
	if (App::instance->isShuttingDown || App::instance->isShutdown) {
		return (-1);
	}
	return (0);
}
