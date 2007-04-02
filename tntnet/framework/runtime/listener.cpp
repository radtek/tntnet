/* listener.cpp
 * Copyright (C) 2003 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "tnt/listener.h"
#include "tnt/tntnet.h"
#include <cxxtools/log.h>
#include <errno.h>
#include <unistd.h>

#ifdef WITH_GNUTLS
#  include "tnt/gnutls.h"
#endif

#ifdef WITH_OPENSSL
#  include "tnt/openssl.h"
#endif

#ifdef HAVE_TCP_DEFER_ACCEPT
#  include <netinet/tcp.h>
#endif

log_define("tntnet.listener")

static void doListenRetry(cxxtools::net::Server& server,
  const char* ipaddr, unsigned short int port)
{
  for (unsigned n = 1; true; ++n)
  {
    try
    {
      log_debug("listen " << ipaddr << ':' << port);
      server.listen(ipaddr, port, tnt::Listener::getBacklog());

#ifdef HAVE_TCP_DEFER_ACCEPT
      int deferSecs = 30;
      if (::setsockopt(server.getFd(), SOL_TCP, TCP_DEFER_ACCEPT,
          &deferSecs, sizeof(deferSecs)) < 0)
        throw cxxtools::net::Exception(errno, "setsockopt");
#endif

      return;
    }
    catch (const cxxtools::net::Exception& e)
    {
      log_debug("cxxtools::net::Exception caught: errno=" << e.getErrno() << " msg=" << e.what());
      if (e.getErrno() != EADDRINUSE || n > tnt::Listener::getListenRetry())
      {
        log_debug("rethrow exception");
        throw;
      }
      log_warn("address " << ipaddr << ':' << port << " in use - retry; n = " << n);
      ::sleep(1);
    }
  }
}

namespace tnt
{
  void ListenerBase::doStop()
  {
    log_warn("stop listener " << ipaddr << ':' << port);
    try
    {
      // connect once to wake up listener, so it will check stop-flag
      cxxtools::net::Stream(ipaddr, port);
    }
    catch (const std::exception& e)
    {
      log_warn("error waking up listener: " << e.what() << " try 127.0.0.1");
      cxxtools::net::Stream("127.0.0.1", port);
    }
  }

  int Listener::backlog = 16;
  unsigned Listener::listenRetry = 5;

  Listener::Listener(const std::string& ipaddr, unsigned short int port, Jobqueue& q)
    : ListenerBase(ipaddr, port),
      queue(q)
  {
    log_info("listen ip=" << ipaddr << " port=" << port);
    doListenRetry(server, ipaddr.c_str(), port);
    queue.put(new Tcpjob(server, queue));
  }

#ifdef WITH_GNUTLS
#define USE_SSL

#endif

#ifdef WITH_OPENSSL
#define USE_SSL

#endif

#ifdef USE_SSL
  Ssllistener::Ssllistener(const char* certificateFile,
      const char* keyFile,
      const std::string& ipaddr, unsigned short int port,
      Jobqueue& q)
    : ListenerBase(ipaddr, port),
      server(certificateFile, keyFile),
      queue(q)
  {
    log_info("listen ip=" << ipaddr << " port=" << port << " (ssl)");
    doListenRetry(server, ipaddr.c_str(), port);
    queue.put(new SslTcpjob(server, queue));
  }

#endif // USE_SSL

}
