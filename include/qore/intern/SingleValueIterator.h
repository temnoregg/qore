/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  SingleValueIterator.h

  Qore Programming Language

  Copyright (C) 2003 - 2015 David Nichols

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

  Note that the Qore library is released under a choice of three open-source
  licenses: MIT (as above), LGPL 2+, or GPL 2+; see README-LICENSE for more
  information.
*/

#ifndef _QORE_SINGLEVALUEITERATOR_H

#define _QORE_SINGLEVALUEITERATOR_H

// the c++ object
class SingleValueIterator : public QoreIteratorBase {
protected:
   AbstractQoreNode* val;
   bool validp;

public:
   DLLLOCAL SingleValueIterator(const AbstractQoreNode* v) : val(!is_nothing(v) ? v->refSelf() : 0), validp(false) {
   }

   DLLLOCAL SingleValueIterator(const SingleValueIterator& old) : val(old.val ? old.val->refSelf() : 0), validp(old.validp) {
   }

   DLLLOCAL bool next() {
      if (!val)
         return false;
      return (validp = !validp);
   }

   DLLLOCAL int checkValid(ExceptionSink* xsink) const {
      if (!validp) {
         xsink->raiseException("ITERATOR-ERROR", "the %s is not pointing at a valid element; make sure %s::next() returns True before calling this method", getName(), getName());
         return -1;
      }
      return 0;
   }

   DLLLOCAL AbstractQoreNode* getValue() {
      return val ? val->refSelf() : 0;
   }

   DLLLOCAL bool valid() const {
      return validp;
   }

   DLLLOCAL void reset() {
      if (validp)
         validp = false;
   }

   using AbstractPrivateData::deref;
   DLLLOCAL virtual void deref(ExceptionSink* xsink) {
      if (ROdereference()) {
         if (val)
            val->deref(xsink);
         delete this;
      }
   }

   DLLLOCAL virtual const char* getName() const { return "SingleValueIterator"; }
};

#endif // _QORE_SINGLEVALUEITERATOR_H
