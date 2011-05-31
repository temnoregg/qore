/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
 QC_AutoGate.h
 
 Qore Programming Language
 
 Copyright 2003 - 2011 David Nichols
 
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

#ifndef _QORE_QC_AUTOGATE_H

#define _QORE_QC_AUTOGATE_H

#include <qore/Qore.h>
#include <qore/intern/QC_Gate.h>

DLLEXPORT extern qore_classid_t CID_AUTOGATE;

DLLLOCAL QoreClass *initAutoGateClass(QoreClass *Gate);

class QoreAutoGate : public AbstractPrivateData {
   QoreGate *g;

public:
   DLLLOCAL QoreAutoGate(QoreGate *gt, ExceptionSink *xsink) {
      g = gt;
      g->grab(xsink);
   }

   using AbstractPrivateData::deref;
   DLLLOCAL virtual void deref(ExceptionSink *xsink) {
      if (ROdereference()) {
	 g->deref(xsink);
	 delete this;
      }
   }

   DLLLOCAL virtual void destructor(ExceptionSink *xsink) {
      g->release(xsink);
   }
};

#endif
