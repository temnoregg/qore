/*
  QoreValueList.cpp

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
#include <qore/intern/qore_value_list_private.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <qore/minitest.hpp>
#ifdef DEBUG_TESTS
#  include "tests/List_tests.cpp"
#endif

#include <algorithm>

#define LIST_BLOCK 20
#define LIST_PAD   15

QoreValueList::QoreValueList() : AbstractQoreNode(NT_VALUE_LIST, true, false), priv(new qore_value_list_private) {
   //printd(5, "QoreValueList::QoreValueList() 1 this=%p ne=%d v=%d\n", this, needs_eval_flag, value);
}

/*
QoreValueList::QoreValueList(bool i) : AbstractQoreNode(NT_VALUE_LIST, !i, i), priv(new qore_value_list_private) {
   //printd(5, "QoreValueList::QoreValueList() 2 this=%p ne=%d v=%d\n", this, needs_eval_flag, value);
}
*/

QoreValueList::~QoreValueList() {
   delete priv;
}

AbstractQoreNode* QoreValueList::realCopy() const {
   return copy();
}

bool QoreValueList::is_equal_soft(const AbstractQoreNode* v, ExceptionSink* xsink) const {
   const QoreValueList* l = v && v->getType() == NT_VALUE_LIST ? reinterpret_cast<const QoreValueList*>(v) : 0;
   if (!l || l->size() != size())
      return false;

   for (size_t i = 0; i < l->size(); ++i) {
      if (!l->retrieveEntry(i).isEqualSoft(retrieveEntry(i), xsink) || *xsink)
         return false;
   }
   return true;
}

bool QoreValueList::is_equal_hard(const AbstractQoreNode* v, ExceptionSink* xsink) const {
   const QoreValueList* l = v && v->getType() == NT_VALUE_LIST ? reinterpret_cast<const QoreValueList*>(v) : 0;
   if (!l || l->size() != size())
      return false;

   for (size_t i = 0; i < l->size(); i++)
      if (!l->retrieveEntry(i).isEqualHard(retrieveEntry(i)))
         return false;
   return true;
}

// returns the type name as a c string
const char* QoreValueList::getTypeName() const {
   return getStaticTypeName();
}

const QoreValue QoreValueList::retrieveEntry(size_t num) const {
   if (num >= priv->length)
      return QoreValue();
   return priv->entry[num];
}

QoreValue QoreValueList::retrieveEntry(size_t num) {
   if (num >= priv->length)
      return QoreValue();
   return priv->entry[num];
}

QoreValue QoreValueList::getReferencedEntry(size_t num) const {
   if (num >= priv->length)
      return QoreValue();
   return priv->entry[num].refSelf();
}

QoreValue& QoreValueList::getEntryReference(size_t num) {
   return priv->getEntryReference(num);
}

QoreValue* QoreValueList::getExistingEntryPtr(size_t num) {
   assert(reference_count() == 1);
   if (num >= priv->length)
      return 0;
   return &priv->entry[num];
}

void QoreValueList::setEntry(size_t index, QoreValue val, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   val.sanitize();
   QoreValue& v = getEntryReference(index);
   if (v.hasNode()) {
      if (get_container_obj(v.getInternalNode()))
	 priv->incObjectCount(-1);

      v.discard(xsink);
   }
   v = val;
   v.sanitize();

   if (v.hasNode() && get_container_obj(v.getInternalNode()))
      priv->incObjectCount(1);
}

void QoreValueList::push(QoreValue val) {
   assert(reference_count() == 1);
   priv->push(val);
}

void QoreValueList::merge(const QoreValueList* list) {
   assert(reference_count() == 1);
   int start = priv->length;
   priv->resize(priv->length + list->priv->length);
   for (size_t i = 0; i < list->priv->length; i++) {
      QoreValue p = list->priv->entry[i];
      priv->entry[start + i] = p.refSelf();
      if (p.hasNode() && get_container_obj(p.getInternalNode()))
	 priv->incObjectCount(1);
   }
}

/*
// FIXME: derefs potentially inside a lock
// delete an priv->entry and move down the rest of the entries
void QoreValueList::popEntry(size_t ind, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   if (ind >= priv->length)
      return;

   QoreValue& e = priv->entry[ind];
   if (e.hasNode()) {
      if (e.getType() == NT_OBJECT)
	 e.get<QoreObject>()->doDelete(xsink);

      if (get_container_obj(e.getInternalNode()))
	 priv->incObjectCount(-1);
   
      e.discard(xsink);
      e = QoreValue();
   }

   // resize list
   priv->length--;
   if (ind < priv->length)
      memmove(priv->entry + ind, priv->entry + ind + 1, sizeof(QoreValue) * (priv->length - ind));
   priv->resize(priv->length);
}
*/

void QoreValueList::insert(QoreValue val) {
   assert(reference_count() == 1);
   priv->resize(priv->length + 1);
   if (priv->length - 1)
      memmove(priv->entry + 1, priv->entry, sizeof(QoreValue) * (priv->length - 1));
   priv->entry[0] = val;
   if (val.hasNode() && get_container_obj(val.getInternalNode()))
      priv->incObjectCount(1);
}

QoreValue QoreValueList::shift() {
   assert(reference_count() == 1);
   if (!priv->length)
      return QoreValue();
   QoreValue& rv = priv->entry[0];
   size_t pos = priv->length - 1;
   memmove(priv->entry, priv->entry + 1, sizeof(QoreValue) * pos);
   priv->entry[pos] = QoreValue();
   priv->resize(pos);

   if (rv.hasNode() && get_container_obj(rv.getInternalNode()))
      priv->incObjectCount(-1);

   return rv;
}

QoreValue QoreValueList::pop() {
   assert(reference_count() == 1);
   if (!priv->length)
      return QoreValue();
   QoreValue& rv = priv->entry[priv->length - 1];
   size_t pos = priv->length - 1;
   priv->entry[pos] = QoreValue();
   priv->resize(pos);

   if (rv.hasNode() && get_container_obj(rv.getInternalNode()))
      priv->incObjectCount(-1);

   return rv;
}

AbstractQoreNode* QoreValueList::evalImpl(ExceptionSink* xsink) const {
   return priv->eval(xsink);
}

AbstractQoreNode* QoreValueList::evalImpl(bool &needs_deref, ExceptionSink* xsink) const {
   return evalList(needs_deref, xsink);
}

QoreValueList* QoreValueList::evalList(ExceptionSink* xsink) const {
   if (value) {
      ref();
      return const_cast<QoreValueList*>(this);
   }

   return priv->eval(xsink);
}

QoreValueList* QoreValueList::evalList(bool &needs_deref, ExceptionSink* xsink) const {
   if (value) {
      needs_deref = false;
      return const_cast<QoreValueList*>(this);
   }
   needs_deref = true;
   return priv->eval(xsink);
}

int64 QoreValueList::bigIntEvalImpl(ExceptionSink* xsink) const {
   return 0;
}

int QoreValueList::integerEvalImpl(ExceptionSink* xsink) const {
   return 0;
}

bool QoreValueList::boolEvalImpl(ExceptionSink* xsink) const {
   return false;
}

double QoreValueList::floatEvalImpl(ExceptionSink* xsink) const {
   return 0.0;
}

QoreValueList* QoreValueList::copy() const {
   QoreValueList* nl = new QoreValueList;
   for (size_t i = 0; i < priv->length; ++i)
      nl->push(priv->entry[i].refSelf());
   return nl;
}

QoreValueList* QoreValueList::copyListFrom(size_t index) const {
   QoreValueList* nl = new QoreValueList;
   for (size_t i = index; i < priv->length; i++)
      nl->push(priv->entry[i].refSelf());
   return nl;
}

QoreValueList* QoreValueList::splice(ptrdiff_t offset, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   size_t n_offset = priv->checkOffset(offset);
   if (n_offset == priv->length)
      return 0;
   return priv->spliceIntern(n_offset, priv->length - n_offset, xsink);
}

QoreValueList* QoreValueList::splice(ptrdiff_t offset, ptrdiff_t len, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   size_t n_offset, n_len;
   priv->checkOffset(offset, len, n_offset, n_len);
   if (n_offset == priv->length)
      return 0;
   return priv->spliceIntern(n_offset, n_len, xsink);
}

QoreValueList* QoreValueList::splice(ptrdiff_t offset, ptrdiff_t len, const QoreValue l, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   size_t n_offset, n_len;
   priv->checkOffset(offset, len, n_offset, n_len);
   return priv->spliceIntern(n_offset, n_len, l, xsink);
}

QoreValueList* QoreValueList::extract(ptrdiff_t offset, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   size_t n_offset = priv->checkOffset(offset);
   if (n_offset == priv->length)
      return new QoreValueList;

   return priv->spliceIntern(n_offset, priv->length - n_offset, xsink, true);
}

QoreValueList* QoreValueList::extract(ptrdiff_t offset, ptrdiff_t len, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   size_t n_offset, n_len;
   priv->checkOffset(offset, len, n_offset, n_len);
   if (n_offset == priv->length)
      return new QoreValueList;
   return priv->spliceIntern(n_offset, n_len, xsink, true);
}

QoreValueList* QoreValueList::extract(ptrdiff_t offset, ptrdiff_t len, const QoreValue l, ExceptionSink* xsink) {
   assert(reference_count() == 1);
   size_t n_offset, n_len;
   priv->checkOffset(offset, len, n_offset, n_len);
   return priv->spliceIntern(n_offset, n_len, l, xsink, true);
}

QoreValueList* QoreValueList::sort(ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->qsort(0, 0, priv->length - 1, true, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sortDescending(ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->qsort(0, 0, priv->length - 1, false, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sort(const ResolvedCallReferenceNode* fr, ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->qsort(fr, 0, priv->length - 1, true, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sortDescending(const ResolvedCallReferenceNode* fr, ExceptionSink* xsink) const {   
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->qsort(fr, 0, priv->length - 1, false, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sortStable(ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->mergesort(0, true, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sortDescendingStable(ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->mergesort(0, false, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sortStable(const ResolvedCallReferenceNode* fr, ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->mergesort(fr, true, xsink))
	 return 0;

   return rv.release();
}

QoreValueList* QoreValueList::sortDescendingStable(const ResolvedCallReferenceNode* fr, ExceptionSink* xsink) const {
   ReferenceHolder<QoreValueList> rv(copy(), xsink);
   if (priv->length)
      if (rv->priv->mergesort(fr, false, xsink))
	 return 0;

   return rv.release();
}

// does a deep dereference
bool QoreValueList::derefImpl(ExceptionSink* xsink) {
   for (size_t i = 0; i < priv->length; i++)
      priv->entry[i].discard(xsink);
#ifdef DEBUG
   priv->length = 0;
#endif
   return true;
}

size_t QoreValueList::size() const {
   return priv->length;
}

bool QoreValueList::empty() const {
   return !priv->length;
}

/*
void QoreValueList::clearNeedsEval() {
   value = true;
   needs_eval_flag = false;
}

void QoreValueList::setNeedsEval() {
   value = false;
   needs_eval_flag = true;
}
*/

QoreValue QoreValueList::minValue(ExceptionSink* xsink) const {   
   if (!priv->length)
      return QoreValue();
   QoreValue rv = priv->entry[0];
   
   for (size_t i = 1; i < priv->length; ++i) {
      QoreValue v = priv->entry[i];
      if (QoreLogicalLessThanOperatorNode::doLessThan(v, rv, xsink))
	 rv = v;
      if (*xsink)
	 return QoreValue();
   }
   return rv.refSelf();
}

QoreValue QoreValueList::maxValue(ExceptionSink* xsink) const {
   if (!priv->length)
      return QoreValue();
   QoreValue rv = priv->entry[0];

   for (size_t i = 0; i < priv->length; ++i) {
      QoreValue v = priv->entry[i];
      
      if (QoreLogicalGreaterThanOperatorNode::doGreaterThan(v, rv, xsink))
	 rv = v;
      if (*xsink)
	 return QoreValue();
   }
   return rv.refSelf();
}

QoreValue QoreValueList::minValue(const ResolvedCallReferenceNode* fr, ExceptionSink* xsink) const {
   if (!priv->length)
      return QoreValue();
   QoreValue rv = priv->entry[0];

   for (size_t i = 1; i < priv->length; ++i) {
      QoreValue v = priv->entry[i];

      safe_qorevaluelist_t args(do_args(v, rv), xsink);
      ValueHolder result(fr->execValue(*args, xsink), xsink);
      if (*xsink)
	 return QoreValue();
      if (result->getAsBigInt() < 0)
	 rv = v;
   }
   return rv.refSelf();
}

QoreValue QoreValueList::maxValue(const ResolvedCallReferenceNode* fr, ExceptionSink* xsink) const {
   if (!priv->length)
      return QoreValue();
   QoreValue rv = priv->entry[0];

   for (size_t i = 1; i < priv->length; ++i) {
      QoreValue v = priv->entry[i];

      safe_qorevaluelist_t args(do_args(v, rv), xsink);
      ValueHolder result(fr->execValue(*args, xsink), xsink);
      if (*xsink)
	 return QoreValue();
      if (result->getAsBigInt() > 0)
	 rv = v;
   }
   return rv.refSelf();
}

QoreValueList* QoreValueList::reverse() const {
   QoreValueList* l = new QoreValueList;
   l->priv->resize(priv->length);
   for (size_t i = 0; i < priv->length; ++i) {
      QoreValue n = priv->entry[priv->length - i - 1];
      l->priv->entry[i] = n.refSelf();
   }
   return l;
}

int QoreValueList::getAsString(QoreString &str, int foff, ExceptionSink* xsink) const {
   QoreContainerHelper cch(this);
   if (!cch) {
      str.sprintf("[ERROR: recursive reference to list %p]", this);
      return 0;
   }

   if (foff == FMT_YAML_SHORT) {
      str.concat('[');
      ConstValueListIterator li(this);
      while (li.next()) {
	 QoreValue n = li.getValue();
	 if (n.getAsString(str, foff, xsink))
	    return -1;
	 if (!li.last())
	    str.concat(", ");
      }
      str.concat(']');
      return 0;
   }

   if (!size()) {
      str.concat(&EmptyListString);
      return 0;
   }
   str.concat("list: (");

   if (foff != FMT_NONE)
      str.sprintf("%d element%s)\n", priv->length, priv->length == 1 ? "" : "s");

   for (size_t i = 0; i < priv->length; ++i) {
      if (foff != FMT_NONE) {
	 str.addch(' ', foff + 2);
	 str.sprintf("[%d]=", i);
      }
      
      QoreValue n = priv->entry[i];
      if (n.getAsString(str, foff != FMT_NONE ? foff + 2 : foff, xsink))
	 return -1;
      
      if (i != (priv->length - 1)) {
	 if (foff != FMT_NONE)
	    str.concat('\n');
	 else
	    str.concat(", ");
      }
   }
   if (foff == FMT_NONE)
      str.concat(')');

   return 0;
}

// get string representation (for %n and %N), foff is for multi-line formatting offset, -1 = no line breaks
// if del is true, then the returned QoreString * should be deleted, if false, then it must not be
// the ExceptionSink is only needed for QoreObject where a method may be executed
// use the QoreNodeAsStringHelper class (defined in QoreStringNode.h) instead of using this function directly
QoreString* QoreValueList::getAsString(bool &del, int foff, ExceptionSink* xsink) const {
   if (!priv->length && foff != FMT_YAML_SHORT) {
      del = false;
      return &EmptyListString;
   }
   
   TempString rv(new QoreString);
   if (getAsString(*(*rv), foff, xsink))
      return 0;

   del = true;
   return rv.release();
}

bool QoreValueList::getAsBoolImpl() const {
   // check if we should do perl-style boolean evaluation
   if (runtime_check_parse_option(PO_STRICT_BOOLEAN_EVAL))
      return false;
   return !empty();
}

ValueListIterator::ValueListIterator(QoreValueList* lst, size_t n_pos) : l(lst) {
   set(n_pos);
}

ValueListIterator::ValueListIterator(QoreValueList &lst, size_t n_pos) : l(&lst) {
   set(n_pos);
}

bool ValueListIterator::next() {
   if (++pos == (ptrdiff_t)l->size()) {
      pos = -1;
      return false; // finished
   }
   return true;
}

bool ValueListIterator::prev() {
   if (l->empty())
      return false; // empty
   if (pos == -1) {
      pos = l->size() - 1;
      return true;
   }
   if (!pos) {
      pos = -1;
      return false; // finished
   }
   --pos;
   return true;
}

int ValueListIterator::set(size_t n_pos) {
   if (n_pos >= l->size()) {
      pos = -1;
      return -1;
   }
   pos = n_pos;
   return 0;
}

QoreValue ValueListIterator::getValue() const {
   return l->retrieveEntry(pos);
}

QoreValue ValueListIterator::getReferencedValue() const {
   l->retrieveEntry(pos).refSelf();
}

QoreValue* ValueListIterator::getValueReference() const {
   if (pos > (ptrdiff_t)l->size())
      return 0;
   return &l->getEntryReference(pos);
}

bool ValueListIterator::last() const {
   return (bool)(pos == (ptrdiff_t)(l->size() - 1));
} 

bool ValueListIterator::first() const {
   return !pos; 
} 

ConstValueListIterator::ConstValueListIterator(const QoreValueList* lst, size_t n_pos) : l(lst) {
   set(n_pos);
}

ConstValueListIterator::ConstValueListIterator(const QoreValueList &lst, size_t n_pos) : l(&lst) {
   set(n_pos);
}

bool ConstValueListIterator::next() {
   if (++pos == (ptrdiff_t)l->size()) {
      pos = -1;
      return false; // finished
   }
   return true;
}

bool ConstValueListIterator::prev() {
   if (l->empty())
      return false; // empty
   if (pos == -1) {
      pos = l->size() - 1;
      return true;
   }
   if (!pos) {
      pos = -1;
      return false; // finished
   }
   --pos;
   return true;
}

int ConstValueListIterator::set(size_t n_pos) {
   if (n_pos >= l->size()) {
      pos = -1;
      return -1;
   }
   pos = n_pos;
   return 0;
}

const QoreValue ConstValueListIterator::getValue() const {
   return l->retrieveEntry(pos);
}

QoreValue ConstValueListIterator::getReferencedValue() const {
   return l->retrieveEntry(pos).refSelf();
}

bool ConstValueListIterator::last() const {
   return (bool)(pos == (ptrdiff_t)(l->size() - 1));
} 

bool ConstValueListIterator::first() const {
   return !pos;
} 

void ConstValueListIterator::reset() {
   pos = -1;
}

/*
bool QoreValueList::isFinalized() const {
   return priv->finalized;
}

void QoreValueList::setFinalized() {
   priv->finalized = true;
}

bool QoreValueList::isVariableList() const {
   return priv->vlist;
}

void QoreValueList::setVariableList() {
   priv->vlist = true;
}
*/

QoreValueList* QoreValueList::listRefSelf() const {
   ref();
   return const_cast<QoreValueList*>(this);
}

/*
QoreValueList* QoreValueList::parseInitList(LocalVar* oflag, int pflag, int& lvids, const QoreTypeInfo*& typeInfo) {
   typeInfo = listTypeInfo;

   QoreValueListParseInitHelper li(this, oflag, pflag, lvids);
   while (li.next()) {
      const QoreTypeInfo *argTypeInfo = 0;
      li.parseInit(argTypeInfo);
   }

   //printd(0, "QoreValueList::parseInit() this: %p ne: %d v: %d\n", this, needs_eval_flag, value);

   return this;
}

AbstractQoreNode* QoreValueList::parseInit(LocalVar* oflag, int pflag, int& lvids, const QoreTypeInfo*& typeInfo) {
   return parseInitList(oflag, pflag, lvids, typeInfo);
}
*/

QoreValue QoreValueList::swap(ptrdiff_t offset, QoreValue val) {
   QoreValue& r = getEntryReference(offset);
   QoreValue rv = r;
   r = val;
   return rv;
}

QoreValue QoreValueList::takeExists(ptrdiff_t offset) {
   QoreValue* ptr = getExistingEntryPtr(offset);
   if (!ptr)
      return QoreValue();
   QoreValue rv = *ptr;
   *ptr = QoreValue();
   return rv;
}
