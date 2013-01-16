/*
 mySocket.cpp
 
 Qore Programming Language
 
 Copyright 2003 - 2012 David Nichols
 
 provides a thread-safe interface to the QoreSocket object
 
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

#include <qore/Qore.h>
#include <qore/intern/QC_Socket.h>

mySocket::mySocket(QoreSocket *s) : socket(s), cert(0), pk(0) {
}

mySocket::mySocket() : socket(new QoreSocket()), cert(0), pk(0) {
}

mySocket::~mySocket() {
   if (cert)
      cert->deref();
   if (pk)
      pk->deref();
   
   delete socket;
}

int mySocket::connect(const char *name, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connect(name, timeout_ms, xsink);
}

int mySocket::connectINET(const char *host, int port, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectINET(host, port, timeout_ms, xsink);
}

int mySocket::connectINET2(const char *name, const char *service, int family, int sock_type, int protocol, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectINET2(name, service, family, sock_type, protocol, timeout_ms, xsink);
}

int mySocket::connectUNIX(const char *p, int sock_type, int protocol, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectUNIX(p, sock_type, protocol, xsink);
}

// to bind to either a UNIX socket or an INET interface:port
int mySocket::bind(const char *name, bool reuseaddr) {
   AutoLocker al(m);
   return socket->bind(name, reuseaddr);
}

// to bind to an INET tcp port on all interfaces
int mySocket::bind(int port, bool reuseaddr) {
   AutoLocker al(m);
   return socket->bind(port, reuseaddr);
}

// to bind an open socket to an INET tcp port on a specific interface
int mySocket::bind(const char *iface, int port, bool reuseaddr) {
   AutoLocker al(m);
   return socket->bind(iface, port, reuseaddr);
}

int mySocket::bindUNIX(const char *name, int socktype, int protocol, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->bindUNIX(name, socktype, protocol, xsink);
}

int mySocket::bindINET(const char *name, const char *service, bool reuseaddr, int family, int socktype, int protocol, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->bindINET(name, service, reuseaddr, family, socktype, protocol, xsink);
}

// get port number for INET sockets
int mySocket::getPort() {
   AutoLocker al(m);
   return socket->getPort();
}

int mySocket::listen() {
   AutoLocker al(m);
   return socket->listen();
}

// send a buffer of a particular size
int mySocket::send(const char *buf, int size) {
   AutoLocker al(m);
   return socket->send(buf, size);
}

int mySocket::send(const char *buf, int size, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->send(buf, size, timeout_ms, xsink);
}

// send a null-terminated string
int mySocket::send(const QoreString *msg, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->send(msg, timeout_ms, xsink);
}

// send a binary object
int mySocket::send(const BinaryNode *b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->send(b, timeout_ms, xsink);
}

int mySocket::send(const BinaryNode *b) {
   AutoLocker al(m);
   return socket->send(b);
}

// send from a file descriptor
int mySocket::send(int fd, int size) {
   AutoLocker al(m);
   return socket->send(fd, size);
}

// send bytes and convert to network order
int mySocket::sendi1(char b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi1(b, timeout_ms, xsink);
}

int mySocket::sendi2(short b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi2(b, timeout_ms, xsink);
}

int mySocket::sendi4(int b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi4(b, timeout_ms, xsink);
}

int mySocket::sendi8(int64 b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi8(b, timeout_ms, xsink);
}

int mySocket::sendi2LSB(short b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi2LSB(b, timeout_ms, xsink);
}

int mySocket::sendi4LSB(int b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi4LSB(b, timeout_ms, xsink);
}

int mySocket::sendi8LSB(int64 b, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->sendi8LSB(b, timeout_ms, xsink);
}

// receive a packet of bytes as a string
QoreStringNode* mySocket::recv(int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recv(timeout_ms, xsink);
}

// receive a certain number of bytes as a string
QoreStringNode* mySocket::recv(qore_offset_t bufsize, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recv(bufsize, timeout_ms, xsink);
}

// receive a packet of bytes as a binary
BinaryNode* mySocket::recvBinary(int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvBinary(timeout_ms, xsink);
}

// receive a certain number of bytes as a binary object
BinaryNode* mySocket::recvBinary(int bufsize, int timeout_ms, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvBinary(bufsize, timeout_ms, xsink);
}

// receive and write data to a file descriptor
int mySocket::recv(int fd, int size, int timeout_ms) {
   AutoLocker al(m);
   return socket->recv(fd, size, timeout_ms);
}

// receive integers and convert from network byte order
int64 mySocket::recvi1(int timeout_ms, char *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi1(timeout_ms, b, xsink);
}

int64 mySocket::recvi2(int timeout_ms, short *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi2(timeout_ms, b, xsink);
}

int64 mySocket::recvi4(int timeout_ms, int *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi4(timeout_ms, b, xsink);
}

int64 mySocket::recvi8(int timeout_ms, int64 *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi8(timeout_ms, b, xsink);
}

int64 mySocket::recvi2LSB(int timeout_ms, short *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi2LSB(timeout_ms, b, xsink);
}

int64 mySocket::recvi4LSB(int timeout_ms, int *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi4LSB(timeout_ms, b, xsink);
}

int64 mySocket::recvi8LSB(int timeout_ms, int64 *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvi8LSB(timeout_ms, b, xsink);
}

// receive integers and convert from network byte order
int64 mySocket::recvu1(int timeout_ms, unsigned char *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvu1(timeout_ms, b, xsink);
}

int64 mySocket::recvu2(int timeout_ms, unsigned short *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvu2(timeout_ms, b, xsink);
}

int64 mySocket::recvu4(int timeout_ms, unsigned int *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvu4(timeout_ms, b, xsink);
}

int64 mySocket::recvu2LSB(int timeout_ms, unsigned short *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvu2LSB(timeout_ms, b, xsink);
}

int64 mySocket::recvu4LSB(int timeout_ms, unsigned int *b, ExceptionSink* xsink) {
   AutoLocker al(m);
   return socket->recvu4LSB(timeout_ms, b, xsink);
}

// send HTTP message
int mySocket::sendHTTPMessage(ExceptionSink* xsink, QoreHashNode* info, const char *method, const char *path, const char *http_version, const QoreHashNode *headers, const void *ptr, int size, int source, int timeout_ms) {
   AutoLocker al(m);
   return socket->sendHTTPMessage(xsink, info, method, path, http_version, headers, ptr, size, source, timeout_ms);
}

// send HTTP response
int mySocket::sendHTTPResponse(ExceptionSink* xsink, int code, const char *desc, const char *http_version, const QoreHashNode *headers, const void *ptr, int size, int source, int timeout_ms) {
   AutoLocker al(m);
   return socket->sendHTTPResponse(xsink, code, desc, http_version, headers, ptr, size, source, timeout_ms);
}

// receive a binary message in HTTP chunked format
QoreHashNode *mySocket::readHTTPChunkedBodyBinary(int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->readHTTPChunkedBodyBinary(timeout_ms, xsink);
}

// receive a string message in HTTP chunked format
QoreHashNode *mySocket::readHTTPChunkedBody(int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->readHTTPChunkedBody(timeout_ms, xsink);
}

// read and parse HTTP header
AbstractQoreNode *mySocket::readHTTPHeader(ExceptionSink* xsink, QoreHashNode* info, int timeout_ms) {
   AutoLocker al(m);
   return socket->readHTTPHeader(xsink, info, timeout_ms);
}

int mySocket::setSendTimeout(int ms) {
   AutoLocker al(m);
   return socket->setSendTimeout(ms);
}

int mySocket::setRecvTimeout(int ms) {
   AutoLocker al(m);
   return socket->setRecvTimeout(ms);
}

int mySocket::getSendTimeout() {
   AutoLocker al(m);
   return socket->getSendTimeout();
}

int mySocket::getRecvTimeout() {
   AutoLocker al(m);
   return socket->getRecvTimeout();
}

int mySocket::close() { 
   AutoLocker al(m);
   return socket->close();
}

int mySocket::shutdown() { 
   AutoLocker al(m);
   return socket->shutdown();
}

int mySocket::shutdownSSL(ExceptionSink *xsink) { 
   AutoLocker al(m);
   return socket->shutdownSSL(xsink);
}

const char *mySocket::getSSLCipherName() { 
   AutoLocker al(m);
   return socket->getSSLCipherName();
}

const char *mySocket::getSSLCipherVersion() { 
   AutoLocker al(m);
   return socket->getSSLCipherVersion();
}

bool mySocket::isSecure() {
   AutoLocker al(m);
   return socket->isSecure();
}

long mySocket::verifyPeerCertificate() {
   AutoLocker al(m);
   return socket->verifyPeerCertificate();
}

int mySocket::getSocket() {
   return socket->getSocket();
}

void mySocket::setEncoding(const QoreEncoding *id) {
   socket->setEncoding(id);
}

const QoreEncoding *mySocket::getEncoding() const {
   return socket->getEncoding();
}

bool mySocket::isDataAvailable(int timeout_ms) {
   AutoLocker al(m);
   return socket->isDataAvailable(timeout_ms);
}

bool mySocket::isWriteFinished(int timeout_ms) {
   AutoLocker al(m);
   return socket->isWriteFinished(timeout_ms);
}

bool mySocket::isOpen() const {
   return socket->isOpen();
}

int mySocket::connectINETSSL(const char *host, int port, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectINETSSL(host, port, timeout_ms, 
				 cert ? cert->getData() : 0,
				 pk ? pk->getData() : 0,
				 xsink);
}

int mySocket::connectINET2SSL(const char *name, const char *service, int family, int sock_type, int protocol, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectINET2SSL(name, service, family, sock_type, protocol, timeout_ms, 
				  cert ? cert->getData() : 0,
				  pk ? pk->getData() : 0,
				  xsink);
}

int mySocket::connectUNIXSSL(const char *p, int sock_type, int protocol, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectUNIXSSL(p, sock_type, protocol, 
				 cert ? cert->getData() : 0,
				 pk ? pk->getData() : 0,
				 xsink);
}

int mySocket::connectSSL(const char *name, int timeout_ms, ExceptionSink *xsink) {
   AutoLocker al(m);
   return socket->connectSSL(name, timeout_ms,
			     cert ? cert->getData() : 0,
			     pk ? pk->getData() : 0,
			     xsink);
}

mySocket *mySocket::accept(SocketSource *source, ExceptionSink *xsink) {
   QoreSocket *s;
   {
      AutoLocker al(m);
      s = socket->accept(source, xsink);
   }
   return s ? new mySocket(s) : 0;
}

mySocket *mySocket::acceptSSL(SocketSource *source, ExceptionSink *xsink) {
   QoreSocket *s;
   {
      AutoLocker al(m);
      s = socket->acceptSSL(source, cert ? cert->getData() : 0, pk ? pk->getData() : 0, xsink);
   }
   return s ? new mySocket(s) : 0;
}

mySocket *mySocket::accept(int timeout_ms, ExceptionSink *xsink) {
   QoreSocket *s;
   {
      AutoLocker al(m);
      s = socket->accept(timeout_ms, xsink);
   }
   return s ? new mySocket(s) : 0;
}

mySocket *mySocket::acceptSSL(int timeout_ms, ExceptionSink *xsink) {
   QoreSocket *s;
   {
      AutoLocker al(m);
      s = socket->acceptSSL(timeout_ms, cert ? cert->getData() : 0, pk ? pk->getData() : 0, xsink);
   }
   return s ? new mySocket(s) : 0;
}

// c must be already referenced before this call
void mySocket::setCertificate(QoreSSLCertificate *c) {
   AutoLocker al(m);
   if (cert)
      cert->deref();
   cert = c;
}

// p must be already referenced before this call
void mySocket::setPrivateKey(QoreSSLPrivateKey *p) {
   AutoLocker al(m);
   if (pk)
      pk->deref();
   pk = p;
}

void mySocket::upgradeClientToSSL(ExceptionSink* xsink) {
   AutoLocker al(m);
   socket->upgradeClientToSSL(cert ? cert->getData() : 0, pk ? pk->getData() : 0, xsink);
}

void mySocket::upgradeServerToSSL(ExceptionSink* xsink) {
   AutoLocker al(m);
   socket->upgradeServerToSSL(cert ? cert->getData() : 0, pk ? pk->getData() : 0, xsink);
}

void mySocket::setEventQueue(Queue *cbq, ExceptionSink *xsink) {
   AutoLocker al(m);
   socket->setEventQueue(cbq, xsink);
}

int mySocket::setNoDelay(int nodelay) {   
   AutoLocker al(m);
   return socket->setNoDelay(nodelay);
}

int mySocket::getNoDelay() {   
   AutoLocker al(m);
   return socket->getNoDelay();
}

QoreHashNode *mySocket::getPeerInfo(ExceptionSink *xsink) const {
   AutoLocker al(m);
   return socket->getPeerInfo(xsink);
}

QoreHashNode *mySocket::getSocketInfo(ExceptionSink *xsink) const {
   AutoLocker al(m);
   return socket->getSocketInfo(xsink);
}
