/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreHttpClientObjectIntern.h

  Qore Programming Language

  Copyright (C) 2006 - 2014 Qore Technologies, sro
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef QORE_HTTP_CLIENT_OBJECT_INTERN_H_
#define QORE_HTTP_CLIENT_OBJECT_INTERN_H_

#include <map>
#include <set>

// ssl-enabled protocols are stored as negative numbers, non-ssl as positive
#define make_protocol(a, b) ((a) * ((b) ? -1 : 1))
#define get_port(a) ((a) * (((a) < 0) ? -1 : 1))
#define get_ssl(a) ((a) * (((a) < 0) ? true : false))

// protocol map class to recognize user-defined protocols (mostly useful for derived classes)
typedef std::map<std::string, int> prot_map_t;
typedef std::map<std::string, bool, ltstrcase> method_map_t;
typedef std::set<std::string, ltstrcase> strcase_set_t;
typedef std::map<std::string, std::string> header_map_t;

struct con_info {
   int port;
   std::string host, path, username, password;
   bool ssl, is_unix;

   DLLLOCAL con_info(int n_port = 0) : port(n_port), ssl(false), is_unix(false) {
   }

   DLLLOCAL bool has_url() const {
      return !host.empty();
   }

   DLLLOCAL int set_url(QoreURL &url, bool &port_set, ExceptionSink *xsink) {
      port = 0;
      if (url.getPort()) {
	 port = url.getPort();
	 port_set = true;
      }
   
      host = url.getHost() ? url.getHost()->getBuffer() : "";

      // check if hostname is really a local port number (for a URL string like: "8080")
      if (!url.getPort() && !host.empty()) {
	 char *aux;
	 int val = strtol(host.c_str(), &aux, 10);
	 if (aux == (host.c_str() + host.size())) {
	    host = HTTPCLIENT_DEFAULT_HOST;
	    port = val;
	    port_set = true;
	 }
      }
   
      const QoreString *tmp = url.getPath();
      path = tmp ? tmp->getBuffer() : "";
      tmp = url.getUserName();
      username = tmp ? tmp->getBuffer() : "";
      tmp = url.getPassword();
      password = tmp ? tmp->getBuffer() : "";

      if (username.empty() && !password.empty()) {
	 xsink->raiseException("HTTP-CLIENT-URL-ERROR", "invalid authorization credentials: password set without username");
	 return -1;
      }

      if (!username.empty() && password.empty()) {
	 xsink->raiseException("HTTP-CLIENT-URL-ERROR", "invalid authorization credentials: username set without password");
	 return -1;
      }

      if (!port && !host.empty() && host.c_str()[0] == '/')
	 is_unix = true;

      return 0;
   }

   DLLLOCAL QoreStringNode *get_url() const {
      QoreStringNode *pstr = new QoreStringNode("http");
      if (ssl)
	 pstr->concat("s://");
      else
	 pstr->concat("://");
      if (!username.empty())
	 pstr->sprintf("%s:%s@", username.c_str(), password.c_str());

      if (!port) {
	 // concat and encode "host" when using a UNIX domain socket
	 pstr->concat("socket=");
	 for (unsigned i = 0; i < host.size(); ++i) {
	    char c = host[i];
	    switch (c) {
	       case ' ': pstr->concat("%20"); break;
	       case '/': pstr->concat("%2f"); break;
	       default: pstr->concat(c); break;
	    }
	 }
      }
      else
	 pstr->concat(host.c_str());
      if (port && port != 80)
	 pstr->sprintf(":%d", port);
      if (!path.empty()) {
         if (path[0] != '/')
            pstr->concat('/');
         pstr->concat(path.c_str());
      }
      return pstr;
   }

   DLLLOCAL void setUserPassword(const char *user, const char *pass) {
      assert(user && pass);
      username = user;
      password = pass;
   }

   DLLLOCAL void clearUserPassword() {
      username.clear();
      password.clear();
   }

   DLLLOCAL void clear() {
      port = 0;
      username.clear();
      password.clear();
      host.clear();
      path.clear();
      ssl = false;
      is_unix = false;
   }
};

DLLLOCAL extern method_map_t method_map;
DLLLOCAL extern strcase_set_t header_ignore;

DLLLOCAL void do_content_length_event(Queue *cb_queue, int64 id, int len);
DLLLOCAL void do_redirect_event(Queue *cb_queue, int64 id, const QoreStringNode *loc, const QoreStringNode *msg);
DLLLOCAL void do_event(Queue *cb_queue, int64 id, int event);
DLLLOCAL void check_headers(const char *str, int len, bool &multipart, QoreHashNode &ans, const QoreEncoding *enc, ExceptionSink *xsink);

#endif
