/*
  QoreSignal.h

  Qore Programming Language

  Copyright (C) 2003, 2004, 2005, 2006 David Nichols

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

#ifndef _QORE_QORESIGNAL_H

#define _QORE_QORESIGNAL_H

#include <signal.h>
#include <stdio.h>

// maximum number of signals
#define QORE_SIGNAL_MAX 128

class QoreSignalHandler {
   public:
      int sig;
      class Function *func;
      class QoreSignalHandler *next;
      
      QoreSignalHandler(int s, class Function *f)
      {
	 sig = s;
	 func = f;
      }
};

class QoreSignalManager {
   private:
      class QoreSignalHandler *head;

   public:
      inline QoreSignalManager() : head(NULL)
      {
      }
      inline ~QoreSignalManager()
      {
	 class QoreSignalHandler *w = head;
	 while (w)
	 {
	    head = w->next;
	    delete w;
	    w = head;
	 }
      }
      int setHandler(int sig, class Function *f);
      int removeHandler(int sig);
      class Function *getHandler(int sig);
      void handleSignal(int sig);
};

#include <qore/Function.h>

#endif
