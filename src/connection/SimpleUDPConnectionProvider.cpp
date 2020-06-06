/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *                         Benedikt-Alexander Mokroß <oatpp@bamkrs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "./SimpleUDPConnectionProvider.hpp"

#include "oatpp/core/utils/ConversionUtils.hpp"

#include <fcntl.h>

#if defined(WIN32) || defined(_WIN32)
#include <io.h>
  #include <WinSock2.h>
  #include <WS2udpip.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/udp.h>
#include <unistd.h>
#if defined(__FreeBSD__)
#include <netinet/in.h>
#endif
#endif

#define MAX_UDP_PAYLOAD_SIZE 65507

namespace oatpp { namespace network { namespace server {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExtendedUDPConnection

const char* const SimpleUDPConnectionProvider::ExtendedUDPConnection::PROPERTY_PEER_ADDRESS = "peer_address";
const char* const SimpleUDPConnectionProvider::ExtendedUDPConnection::PROPERTY_PEER_ADDRESS_FORMAT = "peer_address_format";
const char* const SimpleUDPConnectionProvider::ExtendedUDPConnection::PROPERTY_PEER_PORT = "peer_port";

SimpleUDPConnectionProvider::ExtendedUDPConnection::ExtendedUDPConnection(v_io_handle handle, data::stream::Context::Properties&& properties)
    : m_context(data::stream::StreamType::STREAM_FINITE, std::forward<data::stream::Context::Properties>(properties))
    , m_handle(handle)
{

  m_in.resize(MAX_UDP_PAYLOAD_SIZE);
  memset(m_in.data(), 0, MAX_UDP_PAYLOAD_SIZE);
  struct sockaddr_storage clientAddress;
  socklen_t clientAddressSize = sizeof(clientAddress);
  memset(&clientAddress, 0, sizeof(clientAddressSize));

  // we don't want to actually read the data, just peek to get the extended data info#
  ssize_t rc = recvfrom(m_handle, (char *)m_in.data(), MAX_UDP_PAYLOAD_SIZE,
                     MSG_WAITALL, ( struct sockaddr *) &clientAddress,
                     &clientAddressSize);
  if (rc < 0) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::ExtendedUDPConnection::ExtendedUDPConnection()]", "Call to recvfrom() failed (%l).", rc);
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::ExtendedUDPConnection::ExtendedUDPConnection()]: Error. Call to recvfrom() failed.");
  }

  m_in.resize(rc);

  oatpp::data::stream::Context::Properties props;

  if (clientAddress.ss_family == AF_INET) {

    char strIp[INET_ADDRSTRLEN];
    struct sockaddr_in* sockAddress = (struct sockaddr_in*) &clientAddress;
    inet_ntop(AF_INET, &sockAddress->sin_addr, strIp, INET_ADDRSTRLEN);

    props.put_LockFree(ExtendedUDPConnection::PROPERTY_PEER_ADDRESS, oatpp::String((const char*) strIp));
    props.put_LockFree(ExtendedUDPConnection::PROPERTY_PEER_ADDRESS_FORMAT, "ipv4");
    props.put_LockFree(ExtendedUDPConnection::PROPERTY_PEER_PORT, oatpp::utils::conversion::int32ToStr(sockAddress->sin_port));

  } else if (clientAddress.ss_family == AF_INET6) {

    char strIp[INET6_ADDRSTRLEN];
    struct sockaddr_in6* sockAddress = (struct sockaddr_in6*) &clientAddress;
    inet_ntop(AF_INET6, &sockAddress->sin6_addr, strIp, INET6_ADDRSTRLEN);

    props.put_LockFree(ExtendedUDPConnection::PROPERTY_PEER_ADDRESS, oatpp::String((const char*) strIp));
    props.put_LockFree(ExtendedUDPConnection::PROPERTY_PEER_ADDRESS_FORMAT, "ipv6");
    props.put_LockFree(ExtendedUDPConnection::PROPERTY_PEER_PORT, oatpp::utils::conversion::int32ToStr(sockAddress->sin6_port));

  } else {

    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::getExtendedUDPConnection()]", "Error. Unknown address family %u.", clientAddress.ss_family);
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::getExtendedUDPConnection()]: Error. Unknown address family");

  }

  m_context = oatpp::data::stream::DefaultInitializedContext(data::stream::STREAM_FINITE, std::move(props));
}

oatpp::data::stream::Context& SimpleUDPConnectionProvider::ExtendedUDPConnection::getOutputStreamContext() {
  return m_context;
}

oatpp::data::stream::Context& SimpleUDPConnectionProvider::ExtendedUDPConnection::getInputStreamContext() {
  return m_context;
}

v_io_size SimpleUDPConnectionProvider::ExtendedUDPConnection::write(const void *buff,
                                                                    v_buff_size count,
                                                                    async::Action &action) {
  if (m_out.size() == MAX_UDP_PAYLOAD_SIZE) {
    return -ENOMEM;
  }
  if (m_out.size() + count > MAX_UDP_PAYLOAD_SIZE) {
    count = MAX_UDP_PAYLOAD_SIZE - m_out.size();
  }
  size_t writeTo = m_out.size();
  m_out.resize(m_out.size() + count);
  memcpy(m_out.data() + writeTo, buff, count);
  return count;
}

v_io_size SimpleUDPConnectionProvider::ExtendedUDPConnection::read(void *buff,
                                                                   v_buff_size count,
                                                                   async::Action &action) {
  if (m_in.size() < (m_in_off + count)) {
    count = m_in.size() - m_in_off;
  }

  memcpy(buff, m_in.data() + m_in_off, count);
  m_in_off += count;
  return count;
}

void SimpleUDPConnectionProvider::ExtendedUDPConnection::setOutputStreamIOMode(data::stream::IOMode ioMode) {
  m_out_m = ioMode;
}

data::stream::IOMode SimpleUDPConnectionProvider::ExtendedUDPConnection::getOutputStreamIOMode() {
  return m_out_m;
}

void SimpleUDPConnectionProvider::ExtendedUDPConnection::setInputStreamIOMode(data::stream::IOMode ioMode) {
  m_in_m = ioMode;
}

data::stream::IOMode SimpleUDPConnectionProvider::ExtendedUDPConnection::getInputStreamIOMode() {
  return m_in_m;
}

void SimpleUDPConnectionProvider::ExtendedUDPConnection::close() {
  if (m_context.getProperties().get(PROPERTY_PEER_ADDRESS_FORMAT) == "ipv4") {
    struct sockaddr_in cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = strtol((const char*)m_context.getProperties().get(PROPERTY_PEER_PORT)->c_str(), nullptr, 10);
    inet_pton(AF_INET, m_context.getProperties().get(PROPERTY_PEER_ADDRESS)->c_str(), &(cliaddr.sin_addr));

    sendto(m_handle, (const char*)m_out.data(), m_out.size(), 0,
                  (struct sockaddr*)&cliaddr, sizeof(cliaddr));
  } else if  (m_context.getProperties().get(PROPERTY_PEER_ADDRESS_FORMAT) == "ipv6") {
    struct sockaddr_in6 cliaddr;
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin6_family = AF_INET6;
    cliaddr.sin6_port = strtol((const char*)m_context.getProperties().get(PROPERTY_PEER_PORT)->c_str(), nullptr, 10);
    inet_pton(AF_INET6, m_context.getProperties().get(PROPERTY_PEER_ADDRESS)->c_str(), &(cliaddr.sin6_addr));

    sendto(m_handle, (const char*)m_out.data(), m_out.size(), 0,
                  (struct sockaddr*)&cliaddr, sizeof(cliaddr));
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleUDPConnectionProvider

SimpleUDPConnectionProvider::SimpleUDPConnectionProvider(v_uint16 port)
    : m_port(port)
    , m_closed(false)
{
  m_serverHandle = instantiateServer();
  setProperty(PROPERTY_HOST, "localhost");
  setProperty(PROPERTY_PORT, oatpp::utils::conversion::int32ToStr(port));
}

SimpleUDPConnectionProvider::~SimpleUDPConnectionProvider() {
  close();
}

void SimpleUDPConnectionProvider::close() {
  if(!m_closed) {
    m_closed = true;
#if defined(WIN32) || defined(_WIN32)
    ::closesocket(m_serverHandle);
#else
    ::close(m_serverHandle);
#endif
  }
}

#if defined(WIN32) || defined(_WIN32)

oatpp::v_io_handle SimpleUDPConnectionProvider::instantiateServer(){

  int iResult;

  SOCKET ListenSocket = INVALID_SOCKET;

  struct addrinfo *result = NULL;
  struct addrinfo hints;

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;
  auto portStr = oatpp::utils::conversion::int32ToStr(m_port);

  iResult = getaddrinfo(NULL, (const char*) portStr->getData(), &hints, &result);
  if ( iResult != 0 ) {
    printf("getaddrinfo failed with error: %d\n", iResult);
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Call to getaddrinfo() failed with result=%d", iResult);
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Call to getaddrinfo() failed.");
  }

  ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (ListenSocket == INVALID_SOCKET) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Call to socket() failed with result=%ld", WSAGetLastError());
    freeaddrinfo(result);
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Call to socket() failed.");
  }

  // Setup the TCP listening socket
  iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (iResult == SOCKET_ERROR) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Call to bind() failed with result=%ld", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ListenSocket);
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Call to bind() failed.");
  }

  freeaddrinfo(result);

  iResult = listen(ListenSocket, SOMAXCONN);
  if (iResult == SOCKET_ERROR) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Call to listen() failed with result=%ld", WSAGetLastError());
    closesocket(ListenSocket);
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Call to listen() failed.");
  }

  u_long flags = 1;
  if(NO_ERROR != ioctlsocket(ListenSocket, FIONBIO, &flags)) {
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Call to ioctlsocket failed.");
  }

  return ListenSocket;

}

#else

oatpp::v_io_handle SimpleUDPConnectionProvider::instantiateServer(){

  oatpp::v_io_handle serverHandle;
  v_int32 ret;
  int yes = 1;

  struct addrinfo *result = NULL;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = AI_PASSIVE;
  auto portStr = oatpp::utils::conversion::int32ToStr(m_port);

  ret = getaddrinfo(NULL, (const char *) portStr->getData(), &hints, &result);
  if (ret != 0) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Call to getaddrinfo() failed with result=%d: %s", ret, strerror(errno));
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Call to getaddrinfo() failed.");
  }

  serverHandle = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (serverHandle < 0) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Couldn't open a socket: socket(%d, %d, %d) %s",
               result->ai_family, result->ai_socktype, result->ai_protocol, strerror(errno));
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Couldn't open a socket");
  }

  ret = setsockopt(serverHandle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if(ret < 0) {
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Warning. Failed to set %s for accepting socket: %s", "SO_REUSEADDR", strerror(errno));
  }


  ret = bind(serverHandle, result->ai_addr, (int) result->ai_addrlen);
  if(ret != 0) {
    ::close(serverHandle);
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Failed to bind port %d: %s", m_port, strerror(errno));
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Can't bind to address: %s");
  }

  struct ip_mreq mreq;

  /* the multicast group you want to join */
  mreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
/* the IP of the listening NIC */
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  if (0 != setsockopt(serverHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof mreq)) {
    ::close(serverHandle);
    OATPP_LOGE("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]", "Error. Failed to setsockopt: %s", m_port, strerror(errno));
    throw std::runtime_error("[oatpp::network::server::SimpleUDPConnectionProvider::instantiateServer()]: Error. Failed to setsockopt: %s");
  }

  fcntl(serverHandle, F_SETFL, O_NONBLOCK);

  return serverHandle;

}

#endif

std::shared_ptr<oatpp::data::stream::IOStream> SimpleUDPConnectionProvider::getUDPConnection() {
  data::stream::Context::Properties properties;
  return std::make_shared<ExtendedUDPConnection>(m_serverHandle, std::move(properties));
}

std::shared_ptr<oatpp::data::stream::IOStream> SimpleUDPConnectionProvider::getConnection() {

  fd_set set;
  struct timeval timeout;
  FD_ZERO(&set);
  FD_SET(m_serverHandle, &set);

  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  while(!m_closed) {

    auto res = select(m_serverHandle + 1, &set, nullptr, nullptr, &timeout);

    if (res == 0) {
      return nullptr;
    }

    if (res > 0) {
      break;
    }

  }

  return getUDPConnection();
}

void SimpleUDPConnectionProvider::invalidateConnection(const std::shared_ptr<IOStream>& connection) {

  /************************************************
   * WARNING!!!
   *
   * shutdown(handle, SHUT_RDWR)    <--- DO!
   * close(handle);                 <--- DO NOT!
   *
   * DO NOT CLOSE file handle here -
   * USE shutdown instead.
   * Using close prevent FDs popping out of epoll,
   * and they'll be stuck there forever.
   ************************************************/

  auto c = std::static_pointer_cast<network::Connection>(connection);
  v_io_handle handle = c->getHandle();

#if defined(WIN32) || defined(_WIN32)
  shutdown(handle, SD_BOTH);
#else
  shutdown(handle, SHUT_RDWR);
#endif

}

}}}