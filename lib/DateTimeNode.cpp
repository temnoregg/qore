/*
  DateTimeNode.cpp

  DateTimeNode Class Definition

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

#include <qore/Qore.h>
#include <qore/intern/qore_date_private.h>

DateTimeNode::DateTimeNode(qore_date_private *n_priv) : SimpleValueQoreNode(NT_DATE), DateTime(n_priv) {
}

DateTimeNode::DateTimeNode(bool r) : SimpleValueQoreNode(NT_DATE), DateTime(r) {
}

DateTimeNode::~DateTimeNode() {
}

DateTimeNode::DateTimeNode(int y, int mo, int d, int h, int mi, int s, short ms, bool r) : SimpleValueQoreNode(NT_DATE), DateTime(y, mo, d, h, mi, s, ms, r) {
}

DateTimeNode::DateTimeNode(int64 seconds) : SimpleValueQoreNode(NT_DATE), DateTime(seconds) {
}

DateTimeNode::DateTimeNode(int64 seconds, int ms) : SimpleValueQoreNode(NT_DATE), DateTime(seconds, ms) {
}

DateTimeNode::DateTimeNode(const char *date) : SimpleValueQoreNode(NT_DATE), DateTime(date) {
}

DateTimeNode::DateTimeNode(const AbstractQoreZoneInfo *zone, const char *date) : SimpleValueQoreNode(NT_DATE), DateTime(zone, date) {
}

DateTimeNode::DateTimeNode(struct tm *tms) : SimpleValueQoreNode(NT_DATE), DateTime(tms) {
}

DateTimeNode::DateTimeNode(const DateTime &dt) : SimpleValueQoreNode(NT_DATE), DateTime(dt) {
}

DateTimeNode::DateTimeNode(const DateTimeNode &dt) : SimpleValueQoreNode(NT_DATE), DateTime(dt) {
}

// get the value of the type in a string context (default implementation = del = false and returns NullString)
// if del is true, then the returned QoreString * should be deleted, if false, then it must not be
// use the QoreStringValueHelper class (defined in QoreStringNode.h) instead of using this function directly
QoreString *DateTimeNode::getStringRepresentation(bool &del) const {
   del = true;
   return new QoreString(this);
}

// concatenate string representation to a QoreString (no action for complex types = default implementation)
void DateTimeNode::getStringRepresentation(QoreString &str) const {
   str.concat(this);
}

// if del is true, then the returned DateTime * should be deleted, if false, then it should not
DateTime *DateTimeNode::getDateTimeRepresentation(bool &del) const {
   del = false;
   return const_cast<DateTimeNode *>(this);
}

// assign date representation to a DateTime (no action for complex types = default implementation)
void DateTimeNode::getDateTimeRepresentation(DateTime &dt) const {
   dt.setDate(*this);
}

bool DateTimeNode::getAsBoolImpl() const {
   // always the same logic with or without perl-style boolean evaluation
   return hasValue();
}

int DateTimeNode::getAsIntImpl() const {
   return (int)getEpochSeconds();
}

int64 DateTimeNode::getAsBigIntImpl() const {
   return getEpochSeconds();
}

double DateTimeNode::getAsFloatImpl() const {
   return (double)getEpochSeconds();
}

// get string representation (for %n and %N), foff is for multi-line formatting offset, -1 = no line breaks
// if del is true, then the returned QoreString * should be deleted, if false, then it must not be
// the ExceptionSink is only needed for QoreObject where a method may be executed
// use the QoreNodeAsStringHelper class (defined in QoreStringNode.h) instead of using this function directly
QoreString *DateTimeNode::getAsString(bool &del, int foff, ExceptionSink *xsink) const {
   del = true;
   QoreString *str = new QoreString;
   getAsString(*str, foff, xsink);
   return str;
}

int DateTimeNode::getAsString(QoreString &str, int foff, ExceptionSink *xsink) const {
   priv->getAsString(str);
   return 0;
}

AbstractQoreNode *DateTimeNode::realCopy() const {
   return new DateTimeNode(*this);
}

bool DateTimeNode::is_equal_soft(const AbstractQoreNode *v, ExceptionSink *xsink) const {
   DateTimeValueHelper date(v);
   return isEqual(*date);
}

bool DateTimeNode::is_equal_hard(const AbstractQoreNode *v, ExceptionSink *xsink) const {
   const DateTimeNode *date = dynamic_cast<const DateTimeNode *>(v);
   if (!date)
      return false;

   return isEqual(date);
}

const char *DateTimeNode::getTypeName() const {
   return getStaticTypeName();
}

DateTimeNode *DateTimeNode::copy() const {
   return new DateTimeNode(*this);
}

DateTimeNode *DateTimeNode::add(const DateTime *dt) const {
   assert(dt);
   DateTimeNode *rv;
   if (isRelative()) {
      rv = new DateTimeNode(*dt);
      rv->priv->add(*priv);
   }
   else {
      rv = new DateTimeNode(*this);
      rv->priv->add(*dt->priv);
   }
   return rv;
}

DateTimeNode *DateTimeNode::subtractBy(const DateTime *dt) const {
   DateTimeNode *rv;
   if (isRelative()) {
      rv = new DateTimeNode(*dt);
      rv->priv->subtractBy(*priv);
   }
   else {
      rv = new DateTimeNode(*this);
      rv->priv->subtractBy(*dt->priv);
   }
   return rv;
}

// note that ISO-8601 week days go from 1 - 7 = Mon - Sun
// a NULL return value means an exception was raised
// static method
DateTimeNode *DateTimeNode::getDateFromISOWeek(int year, int week, int day, ExceptionSink *xsink) {
   SimpleRefHolder<DateTimeNode> rv(new DateTimeNode());
   if (qore_date_private::getDateFromISOWeek(*rv->priv, year, week, day, xsink))
      return 0;
   return rv.release();
}

DateTimeNode *DateTimeNode::unaryMinus() const {
   DateTimeNode *rv = new DateTimeNode(*this);
   rv->priv->unaryMinus();
   return rv;
}

DateTimeNode *DateTimeNode::makeAbsolute(const AbstractQoreZoneInfo *z, int y, int mo, int d, int h, int mi, int s, int u) {
   return new DateTimeNode(new qore_date_private(z, y, mo, d, h, mi, s, u));
}

DateTimeNode *DateTimeNode::makeAbsolute(const AbstractQoreZoneInfo *zone, int64 seconds, int us) {
   return new DateTimeNode(new qore_date_private(zone, seconds, us));
}

DateTimeNode *DateTimeNode::makeAbsoluteLocal(const AbstractQoreZoneInfo *zone, int64 seconds, int us) {
   DateTimeNode *rv = new DateTimeNode(new qore_date_private);
   rv->priv->setLocalDate(zone, seconds, us);
   return rv;
}

DateTimeNode *DateTimeNode::makeRelative(int y, int mo, int d, int h, int mi, int s, int u) {
   return new DateTimeNode(new qore_date_private(y, mo, d, h, mi, s, u, true));
}

DateTimeValueHelper::DateTimeValueHelper(const AbstractQoreNode* n) {
   // optmization without virtual function call for most common case
   if (n) {
      if (n->getType() == NT_DATE) {
	 dt = reinterpret_cast<const DateTimeNode *>(n);
	 del = false;
      }
      else
	 dt = n->getDateTimeRepresentation(del);
   }
   else {
      dt = ZeroDate;
      del = false;
   }
}

DateTimeValueHelper::DateTimeValueHelper(const QoreValue& n) {
   if (!n.isNullOrNothing()) {
      switch (n.type) {
	 case QV_Node: {
	    dt = n.getInternalNode()->getDateTimeRepresentation(del);
	    return;
	 }
	 case QV_Bool:
	 case QV_Int: {
	    dt = new DateTimeNode(n.getAsBigInt());
	    del = true;
	    return;
	 }
	 case QV_Float: {
	    dt = DateTimeNode::makeAbsoluteLocal(currentTZ(), (int64)n.v.f, (int)((n.v.f - (float)((int)n.v.f)) * 1000000));
	    del = true;
	    return;
	 }
	 default:
	    assert(false);
	    // no break
      }
   }
   dt = ZeroDate;
   del = false;
}

DateTimeValueHelper::~DateTimeValueHelper() {
   if (del)
      delete const_cast<DateTime*>(dt);
}
