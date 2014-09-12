/*
  Variable.cpp

  Qore programming language

  Copyright (C) 2003 - 2014 David Nichols

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <qore/QoreType.h>
#include <qore/intern/ParserSupport.h>
#include <qore/intern/QoreObjectIntern.h>
#include <qore/intern/QoreValue.h>
#include <qore/intern/qore_number_private.h>

#include <memory>

// global environment hash
QoreHashNode* ENV;

int qore_gvar_ref_u::write(ExceptionSink* xsink) const {
   if (_refptr & 1) {
      xsink->raiseException("ACCESS-ERROR", "attempt to write to read-only imported global variable '%s'", getPtr()->getName());
      return -1;
   }
   return 0;
}

int Var::getLValue(LValueHelper& lvh, bool for_remove) const {
   if (val.type == QV_Ref) {
      if (val.v.write(lvh.vl.xsink))
         return -1;
      return val.v.getPtr()->getLValue(lvh, for_remove);
   }

   lvh.setTypeInfo(typeInfo);
   lvh.setAndLock(rwl);
   if (checkFinalized(lvh.vl.xsink))
      return -1;

   lvh.setValue((QoreLValueGeneric&)val);
   return 0;
}

void Var::remove(LValueRemoveHelper& lvrh) {
   if (val.type == QV_Ref) {
      if (val.v.write(lvrh.getExceptionSink()))
         return;
      val.v.getPtr()->remove(lvrh);
      return;
   }

   QoreAutoRWWriteLocker al(rwl);
   lvrh.doRemove((QoreLValueGeneric&)val, typeInfo);
}

void Var::del(ExceptionSink* xsink) {
   if (val.type == QV_Ref) {
      printd(4, "Var::~Var() refptr=%p\n", val.v.getPtr());
      val.v.getPtr()->deref(xsink);
      // clear type so no further deleting will be done
   }
   else
      discard(val.remove(true), xsink);
}

bool Var::isImported() const {
   return val.type == QV_Ref;
}

const char* Var::getName() const {
   return name.c_str();
}

AbstractQoreNode* Var::eval() const {
   if (val.type == QV_Ref)
      return val.v.getPtr()->eval();
   QoreAutoRWReadLocker al(rwl);
   return val.getReferencedValue();
}

AbstractQoreNode* Var::eval(bool &needs_deref) const {
   if (val.type == QV_Ref)
      return val.v.getPtr()->eval(needs_deref);
   QoreAutoRWReadLocker al(rwl);
   return val.getReferencedValue(needs_deref, true);
}

bool Var::boolEval() const {
   if (val.type == QV_Ref)
      return val.v.getPtr()->boolEval();

   QoreAutoRWReadLocker al(rwl);
   return val.getAsBool();
}

int64 Var::bigIntEval() const {
   if (val.type == QV_Ref)
      return val.v.getPtr()->bigIntEval();

   QoreAutoRWReadLocker al(rwl);
   return val.getAsBigInt();
}

double Var::floatEval() const {
   if (val.type == QV_Ref)
      return val.v.getPtr()->floatEval();

   QoreAutoRWReadLocker al(rwl);
   return val.getAsFloat();
}

void Var::deref(ExceptionSink* xsink) {
   if (ROdereference()) {
      del(xsink);
      delete this;
   }
}

LValueHelper::LValueHelper(const ReferenceNode& ref, ExceptionSink* xsink, bool for_remove) : vl(xsink), v(0), lvid_set(0), 
#ifdef DO_OBJ_RECURSIVE_CHECK
											      robj(0), 
											      container_change(false), 
#endif
											      val(0), typeInfo(0) {
   RuntimeReferenceHelper rh(ref, xsink);
   doLValue(lvalue_ref::get(&ref)->vexp, for_remove);
}

LValueHelper::LValueHelper(const AbstractQoreNode* exp, ExceptionSink* xsink, bool for_remove) : vl(xsink), v(0), lvid_set(0), 
#ifdef DO_OBJ_RECURSIVE_CHECK
												 robj(0),
												 container_change(false), 
#endif
												 val(0), typeInfo(0) {
   // exp can be 0 when called from LValueRefHelper if the attach to the Program fails, for example
   //printd(5, "LValueHelper::LValueHelper() exp: %p (%s %d)\n", exp, get_type_name(exp), get_node_type(exp));
   if (exp)
      doLValue(exp, for_remove);
}

LValueHelper::~LValueHelper() {
#ifdef DO_OBJ_RECURSIVE_CHECK
   if (!container_change && !*vl.xsink && (val ? val->isContainer() : is_container(*v)))
      container_change = true;

   if (robj)
      robj->tRef();

   //printd(0, "LValueHelper::~LValueHelper() robj: %p container_change: %d (val: %s v: %s)\n", robj, container_change, val ? val->getTypeName() : "null", v ? get_type_name(*v) : "null");
#endif

   // first free any locks
   vl.del();

   // now delete temporary values (if any)
   for (nvec_t::iterator i = tvec.begin(), e = tvec.end(); i != e; ++i)
      discard(*i, vl.xsink);

   delete lvid_set;

#ifdef DO_OBJ_RECURSIVE_CHECK
   // recalculate recusive references for objects if necessary
   if (robj && container_change) {
      ObjectRSetHelper rsh(*robj);
   }

   if (robj)
      robj->tDeref();
#endif
}

void LValueHelper::setValue(QoreLValueGeneric& nv) {
   assert(!v);
   assert(!val);
   if (!nv.optimized())
      v = nv.getContainerValuePtr();
   else
      val = &nv;
}

static int var_type_err(const QoreTypeInfo* typeInfo, const char* type, ExceptionSink* xsink) {
   xsink->raiseException("RUNTIME-TYPE-ERROR", "cannot convert lvalue declared as %s to a %s", typeInfo->getName(), type);
   return -1;
}

int LValueHelper::doListLValue(const QoreTreeNode* tree, bool for_remove) {
   // first get index
   int ind = tree->right->integerEval(vl.xsink);
   if (*vl.xsink)
      return -1;
   if (ind < 0) {
      vl.xsink->raiseException("NEGATIVE-LIST-INDEX", "list index %d is invalid (index must evaluate to a non-negative integer)", ind);
      return -1;
   }

   // now get left hand side
   if (doLValue(tree->left, for_remove))
      return -1;

   if (!isNode())
      return for_remove ? -1 : var_type_err(typeInfo, "list", vl.xsink);

   QoreListNode* l;
   if (get_node_type(*v) == NT_LIST) {
      ensureUnique();
      l = reinterpret_cast<QoreListNode*>(*v);
   }
   else {
      if (for_remove)
         return -1;

      // if the lvalue is not already a list, then make it one
      // but first make sure the lvalue can be converted to a list
      if (!typeInfo->parseAcceptsReturns(NT_LIST))
         return var_type_err(typeInfo, "list", vl.xsink);

      // save the old value for dereferencing outside any locks that may have been acquired
      //printd(5, "LValueHelper::doListLValue() this: %p saving old value: %p '%s'\n", this, vp, get_type_name(vp));
      saveTemp(*v);
      *v = l = new QoreListNode;
   }

   resetPtr(l->get_entry_ptr(ind));
   return 0;
}

int LValueHelper::doHashObjLValue(const QoreTreeNode* tree, bool for_remove) {
   QoreNodeEvalOptionalRefHolder member(tree->right, vl.xsink);
   if (*vl.xsink)
      return -1;

   // convert to default character encoding
   QoreStringValueHelper mem(*member, QCS_DEFAULT, vl.xsink);
   if (*vl.xsink)
      return -1;

   if (doLValue(tree->left, for_remove))
      return -1;

   if (!isNode())
      return for_remove ? -1 : var_type_err(typeInfo, "hash", vl.xsink);

   qore_type_t t = get_node_type(*v);
   QoreObject* o = t == NT_OBJECT ? reinterpret_cast<QoreObject*>(*v) : 0;
   QoreHashNode* h = 0;

   //printd(5, "LValueHelper::doHashObjLValue() h=%p v: %p ('%s', refs: %d)\n", h, v, v ? v->getTypeName() : "(null)", v ? v->reference_count() : 0);

   if (!o) {
      if (t == NT_HASH) {
         ensureUnique();
         h = reinterpret_cast<QoreHashNode*>(*v);
      }
      else {
         if (for_remove)
            return -1;

         // if the variable's value is not already a hash or an object, then make it a hash
         // but first make sure the lvalue can be converted to a hash
         if (!typeInfo->parseAcceptsReturns(NT_HASH))
            return var_type_err(typeInfo, "hash", vl.xsink);

         //printd(5, "LValueHelper::doHashObjLValue() this: %p saving value to dereference before making hash: %p '%s'\n", this, vp, get_type_name(vp));
         saveTemp(*v);
         *v = h = new QoreHashNode;
      }

      //printd(5, "LValueHelper::doHashObjLValue() def=%s member %s \"%s\"\n", QCS_DEFAULT->getCode(), mem->getEncoding()->getCode(), mem->getBuffer());
      resetPtr(h->getKeyValuePtr(mem->getBuffer()));
      return 0;
   }

   //printd(5, "LValueHelper::doHashObjLValue() obj: %p member: %s\n", o, mem->getBuffer());

   clearPtr();

   bool intern = qore_class_private::runtimeCheckPrivateClassAccess(*o->getClass());
   if (!qore_object_private::getLValue(*o, mem->getBuffer(), *this, intern, for_remove, vl.xsink)) {
      if (!intern)
         vl.addMemberNotification(o, mem->getBuffer()); // add member notification for external updates
   }
   if (*vl.xsink)
      return -1;

#ifdef DO_OBJ_RECURSIVE_CHECK
   //if (!qore_object_private::isRecursive(*o))
      addOSet(o);
#endif

   return 0;
}

int LValueHelper::doLValue(const ReferenceNode* ref, bool for_remove) {
   //RuntimeReferenceHelper rh(*ref, vl.xsink);
   const lvalue_ref* r = lvalue_ref::get(ref);
   if (!lvid_set)
      lvid_set = new lvid_set_t;
#ifdef DEBUG
   else
      assert(lvid_set->find(r->lvalue_id) == lvid_set->end());
#endif
   lvid_set->insert(r->lvalue_id);
   return doLValue(r->vexp, for_remove);
}

int LValueHelper::doLValue(const AbstractQoreNode* n, bool for_remove) {
   // if we are already locked, then save the value and unlock before processing
   if (vl) {
      saveTemp(n->refSelf());
      vl.del();
   }
   qore_type_t ntype = n->getType();
   //printd(5, "LValueHelper::doLValue(exp: %p) %s %d\n", n, get_type_name(n), get_node_type(n));
   if (ntype == NT_VARREF) {
      const VarRefNode* v = reinterpret_cast<const VarRefNode*>(n);
      //printd(5, "LValueHelper::doLValue(): vref=%s (%p)\n", v->name, v);
      if (v->getLValue(*this, for_remove))
         return -1;
   }
   else if (ntype == NT_SELF_VARREF) {
      const SelfVarrefNode* v = reinterpret_cast<const SelfVarrefNode*>(n);
      // note that getStackObject() is guaranteed to return a value here (self varref is only valid in a method)
      QoreObject* obj = runtime_get_stack_object();
      assert(obj);
      // true is for "internal"
      if (qore_object_private::getLValue(*obj, v->str, *this, true, for_remove, vl.xsink))
         return -1;

#ifdef DO_OBJ_RECURSIVE_CHECK
      //if (!qore_object_private::isRecursive(*obj))
	 addOSet(obj);
#endif
   }
   else if (ntype == NT_CLASS_VARREF)
      reinterpret_cast<const StaticClassVarRefNode*>(n)->getLValue(*this);
   else if (ntype == NT_REFERENCE) {
      if (doLValue(reinterpret_cast<const ReferenceNode*>(n), for_remove))
	 return -1;
   }
   else {
      assert(n->getType() == NT_TREE);
      // it must be a tree
      const QoreTreeNode* tree = reinterpret_cast<const QoreTreeNode*>(n);
      if (tree->getOp() == OP_LIST_REF) {
         if (doListLValue(tree, for_remove))
            return -1;
      }
      else if (doHashObjLValue(tree, for_remove))
         return -1;
   }

#if 0
   if (v && *v)
      printd(0, "LValueHelper::doLValue() v: %p %s %d\n", *v, get_type_name(*v), get_node_type(*v));
   else if (val)
      printd(0, "LValueHelper::doLValue() val: %s %d\n", val->getTypeName(), val->getType());
#endif

   if (v && *v && (*v)->getType() == NT_REFERENCE) {
      const ReferenceNode* ref = reinterpret_cast<const ReferenceNode*>(*v);
      v = 0;
      return doLValue(ref, for_remove);
   }

   return 0;
}

void LValueHelper::setAndLock(QoreRWLock& rwl) {
   rwl.wrlock();
   vl.set(&rwl);
}

void LValueHelper::set(QoreRWLock& rwl) {
   vl.set(&rwl);
}

AbstractQoreNode* LValueHelper::getReferencedValue() const {
   if (val)
      return val->getReferencedValue();

   return *v ? (*v)->refSelf() : 0;
}

const qore_type_t LValueHelper::getType() const {
   if (val)
      return val->getType();
   return get_node_type(*v);
}

const char* LValueHelper::getTypeName() const {
   if (val)
      return val->getTypeName();
   return get_type_name(*v);
}

int64 LValueHelper::getAsBigInt() const {
   if (val) return val->getAsBigInt();
   return (*v) ? (*v)->getAsBigInt() : 0;
}

bool LValueHelper::getAsBool() const {
   if (val) return val->getAsBool();
   return (*v) ? (*v)->getAsBool() : 0;
}

double LValueHelper::getAsFloat() const {
   if (val) return val->getAsFloat();
   return (*v) ? (*v)->getAsFloat() : 0;
}

int LValueHelper::assign(AbstractQoreNode* n, const char* desc) {
   //printd(5, "LValueHelper::assign() this: %p (%s) n: %p '%s' typeInfo: %p '%s' lvid_set: %p\n", this, desc, n, get_type_name(n), typeInfo, typeInfo->getName(), lvid_set);

   // before we can entirely get rid of QoreNothingNode, try to convert Nothing -> 0
   if (n == &Nothing)
      n = 0;

   // check type for assignment
   n = typeInfo->acceptAssignment(desc, n, vl.xsink);
   if (*vl.xsink) {
      //printd(5, "LValueHelper::assign() this: %p saving type-rejected value: %p '%s'\n", this, n, get_type_name(n));
      saveTemp(n);
      return -1;
   }

   if (lvid_set && get_node_type(n) == NT_REFERENCE && (lvid_set->find(lvalue_ref::get(reinterpret_cast<const ReferenceNode*>(n))->lvalue_id) != lvid_set->end())) {
      vl.xsink->raiseException("REFERENCE-ERROR", "recursive reference detected in assignment");
      saveTemp(n);
      return -1;
   }

   if (val) {
      saveTemp(val->assign(n));
      return 0;
   }
   
   //printd(5, "LValueHelper::assign() this: %p saving old value: %p '%s'\n", this, *v, get_type_name(*v));
   saveTemp(*v);
   *v = n;
   return 0;
}

int LValueHelper::assignBigInt(int64 va, const char* desc) {
   // check type for assignment
   if (!typeInfo->parseAccepts(bigIntTypeInfo)) {
      typeInfo->doAcceptError(false, false, -1, desc, Zero, vl.xsink);
      return -1;
   }

   // type compatibility must have been checked at parse time
   if (val) {
      saveTemp(val->assign(va));
      return 0;
   }

   AbstractQoreNode* n = typeInfo->acceptAssignment(desc, new QoreBigIntNode(va), vl.xsink);
   if (*vl.xsink) {
      saveTemp(n);
      return -1;
   }

   saveTemp(*v);
   *v = n;
   return 0;
}

int LValueHelper::assignFloat(double va, const char* desc) {
   // check type for assignment
   if (!typeInfo->parseAccepts(floatTypeInfo)) {
      typeInfo->doAcceptError(false, false, -1, desc, ZeroFloat, vl.xsink);
      return -1;
   }

   // type compatibility must have been checked at parse time
   if (val) {
      saveTemp(val->assign(va));
      return 0;
   }

   AbstractQoreNode* n = typeInfo->acceptAssignment(desc, new QoreFloatNode(va), vl.xsink);
   if (*vl.xsink) {
      saveTemp(n);
      return -1;
   }

   saveTemp(*v);
   *v = n;
   return 0;
}

int64 LValueHelper::plusEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->plusEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val += va;
   return i->val;
}

int64 LValueHelper::minusEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->minusEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val -= va;
   return i->val;
}

int64 LValueHelper::multiplyEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->multiplyEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val *= va;
   return i->val;
}

int64 LValueHelper::divideEqualsBigInt(int64 va, const char* desc) {
   assert(va);

   if (val)
      return val->divideEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val /= va;
   return i->val;
}

int64 LValueHelper::orEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->orEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val |= va;
   return i->val;
}

int64 LValueHelper::xorEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->xorEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val ^= va;
   return i->val;
}

int64 LValueHelper::modulaEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->modulaEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val %= va;
   return i->val;
}

int64 LValueHelper::andEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->andEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val &= va;
   return i->val;
}

int64 LValueHelper::shiftLeftEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->shiftLeftEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val <<= va;
   return i->val;
}

int64 LValueHelper::shiftRightEqualsBigInt(int64 va, const char* desc) {
   if (val)
      return val->shiftRightEqualsBigInt(va, getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   i->val >>= va;
   return i->val;
}

int64 LValueHelper::preIncrementBigInt(const char* desc) {
   if (val)
      return val->preIncrementBigInt(getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   return ++i->val;
}

int64 LValueHelper::preDecrementBigInt(const char* desc) {
   if (val)
      return val->preDecrementBigInt(getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   return --i->val;
}

int64 LValueHelper::postIncrementBigInt(const char* desc) {
   if (val)
      return val->postIncrementBigInt(getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   return i->val++;
}

int64 LValueHelper::postDecrementBigInt(const char* desc) {
   if (val)
      return val->postDecrementBigInt(getTempRef());

   // increment current value
   QoreBigIntNode* i = ensureUnique<QoreBigIntNode, int64, NT_INT>(bigIntTypeInfo, desc);
   if (!i)
      return 0;
   return i->val--;
}

double LValueHelper::preIncrementFloat(const char* desc) {
   if (val)
      return val->preIncrementFloat(getTempRef());

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   return ++f->f;
}

double LValueHelper::preDecrementFloat(const char* desc) {
   if (val)
      return val->preDecrementFloat(getTempRef());

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   return --f->f;
}

double LValueHelper::postIncrementFloat(const char* desc) {
   if (val)
      return val->postIncrementFloat(getTempRef());

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   return f->f++;
}

double LValueHelper::postDecrementFloat(const char* desc) {
   if (val)
      return val->postDecrementFloat(getTempRef());

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   return f->f--;
}

double LValueHelper::plusEqualsFloat(double va, const char* desc) {
   if (val)
      return val->plusEqualsFloat(va, getTempRef());

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   f->f += va;
   return f->f;
}

double LValueHelper::minusEqualsFloat(double va, const char* desc) {
   if (val)
      return val->minusEqualsFloat(va, getTempRef());

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   f->f -= va;
   return f->f;
}

double LValueHelper::multiplyEqualsFloat(double va, const char* desc) {
   if (val) {
      if (val->getType() != NT_FLOAT) {
         typeInfo->doTypeException(0, desc, floatTypeInfo->getName(), vl.xsink);
         return 0.0;
      }
      return val->multiplyEqualsFloat(va, getTempRef());
   }

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   f->f *= va;
   return f->f;
}

double LValueHelper::divideEqualsFloat(double va, const char* desc) {
   assert(va);
   if (val) {
      if (val->getType() != NT_FLOAT) {
         typeInfo->doTypeException(0, desc, floatTypeInfo->getName(), vl.xsink);
         return 0.0;
      }
      return val->divideEqualsFloat(va, getTempRef());
   }

   // increment current value
   QoreFloatNode* f = ensureUnique<QoreFloatNode, double, NT_FLOAT>(floatTypeInfo, desc);
   if (!f)
      return 0.0;
   f->f /= va;
   return f->f;
}

void LValueHelper::preIncrementNumber(const char* desc) {
   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (n)
      qore_number_private::inc(*n);
}

void LValueHelper::preDecrementNumber(const char* desc) {
   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (n)
      qore_number_private::dec(*n);
}

QoreNumberNode* LValueHelper::postIncrementNumber(bool ref_rv, const char* desc) {
   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (!n)
      return 0;
   QoreNumberNode* rv = ref_rv ? new QoreNumberNode(*n) : 0;
   qore_number_private::inc(*n);
   return rv;
}

QoreNumberNode* LValueHelper::postDecrementNumber(bool ref_rv, const char* desc) {
   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (!n)
      return 0;
   QoreNumberNode* rv = ref_rv ? new QoreNumberNode(*n) : 0;
   qore_number_private::dec(*n);
   return rv;
}

void LValueHelper::plusEqualsNumber(const AbstractQoreNode* r, const char* desc) {
   SimpleRefHolder<QoreNumberNode> rn_holder;
   QoreNumberNode* rn;
   if (get_node_type(r) == NT_NUMBER)
      rn = const_cast<QoreNumberNode*>(reinterpret_cast<const QoreNumberNode*>(r));
   else
      rn_holder = (rn = new QoreNumberNode(r));

   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (n)
      qore_number_private::plusEquals(*n, *rn);
}

void LValueHelper::minusEqualsNumber(const AbstractQoreNode* r, const char* desc) {
   SimpleRefHolder<QoreNumberNode> rn_holder;
   QoreNumberNode* rn;
   if (get_node_type(r) == NT_NUMBER)
      rn = const_cast<QoreNumberNode*>(reinterpret_cast<const QoreNumberNode*>(r));
   else
      rn_holder = (rn = new QoreNumberNode(r));

   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (n)
      qore_number_private::minusEquals(*n, *rn);
}

void LValueHelper::multiplyEqualsNumber(const AbstractQoreNode* r, const char* desc) {
   SimpleRefHolder<QoreNumberNode> rn_holder;
   QoreNumberNode* rn;
   if (get_node_type(r) == NT_NUMBER)
      rn = const_cast<QoreNumberNode*>(reinterpret_cast<const QoreNumberNode*>(r));
   else
      rn_holder = (rn = new QoreNumberNode(r));

   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (n)
      qore_number_private::multiplyEquals(*n, *rn);
}

void LValueHelper::divideEqualsNumber(const AbstractQoreNode* r, const char* desc) {
   SimpleRefHolder<QoreNumberNode> rn_holder;
   QoreNumberNode* rn;
   if (get_node_type(r) == NT_NUMBER)
      rn = const_cast<QoreNumberNode*>(reinterpret_cast<const QoreNumberNode*>(r));
   else
      rn_holder = (rn = new QoreNumberNode(r));

   QoreNumberNode* n = ensureUniqueNumber(desc);
   if (n)
      qore_number_private::divideEquals(*n, *rn);
}

int64 LValueHelper::removeBigInt() {
   if (val)
      return val->removeBigInt(getTempRef());

   int64 rv = *v ? (*v)->getAsBigInt() : 0;
   saveTemp(*v);
   *v = 0;
   return rv;
}

double LValueHelper::removeFloat() {
   if (val)
      return val->removeFloat(getTempRef());

   double rv = *v ? (*v)->getAsFloat() : 0;
   saveTemp(*v);
   *v = 0;
   return rv;
}

AbstractQoreNode* LValueHelper::remove(bool for_del) {
   if (val)
      return val->remove(for_del);

   AbstractQoreNode* rv = *v;
   *v = 0;
   return rv;
}

LValueRemoveHelper::LValueRemoveHelper(const AbstractQoreNode* exp, ExceptionSink* n_xsink, bool fd) : xsink(n_xsink), for_del(fd) {
   doRemove(const_cast<AbstractQoreNode*>(exp));
}

LValueRemoveHelper::LValueRemoveHelper(const ReferenceNode& ref, ExceptionSink* n_xsink, bool fd) : xsink(n_xsink), for_del(fd) {
   RuntimeReferenceHelper rrh(ref, xsink);
   if (rrh)
      doRemove(const_cast<AbstractQoreNode*>(lvalue_ref::get(&ref)->vexp));
}

int64 LValueRemoveHelper::removeBigInt() {
   assert(!*xsink);
   ReferenceHolder<> dr(xsink);
   return rv.removeBigInt(dr.getRef());
}

double LValueRemoveHelper::removeFloat() {
   assert(!*xsink);
   ReferenceHolder<> dr(xsink);
   return rv.removeFloat(dr.getRef());
}

AbstractQoreNode* LValueRemoveHelper::remove() {
   assert(!*xsink);
   return rv.remove(for_del);
}

void LValueRemoveHelper::deleteLValue() {
   assert(!*xsink);
   assert(for_del);

   ReferenceHolder<> v(remove(), xsink);
   if (!v)
      return;

   qore_type_t t = v->getType();
   if (t != NT_OBJECT)
      return;

   QoreObject* o = reinterpret_cast<QoreObject* >(*v);
   if (o->isSystemObject()) {
      xsink->raiseException("SYSTEM-OBJECT-ERROR", "you cannot delete a system constant object");
      return;
   }

   o->doDelete(xsink);
}

void LValueRemoveHelper::doRemove(AbstractQoreNode* lvalue) {
   assert(lvalue);
   qore_type_t t = lvalue->getType();
   if (t == NT_VARREF) {
      reinterpret_cast<VarRefNode*>(lvalue)->remove(*this);
      return;
   }

   if (t == NT_SELF_VARREF) {
      rv.assignInitial(qore_object_private::takeMember(*(runtime_get_stack_object()), xsink, reinterpret_cast<SelfVarrefNode*>(lvalue)->str, false));
      return;
   }

   if (t == NT_CLASS_VARREF) {
      reinterpret_cast<StaticClassVarRefNode*>(lvalue)->remove(*this);
      return;
   }

   // could be any type if in a background expression
   if (t != NT_TREE) {
      rv.assignInitial(lvalue ? lvalue->refSelf() : 0);
      return;
   }

   // must be a tree
   assert(t == NT_TREE);
   QoreTreeNode* tree = reinterpret_cast<QoreTreeNode*>(lvalue);

   // can be only a list or object (hash) reference

   // if it's a list reference, see if the reference exists, if so, then remove it
   if (tree->getOp() == OP_LIST_REF) {
      int offset = tree->right->integerEval(xsink);
      if (*xsink)
         return;

      LValueHelper lvhb(tree->left, xsink, true);
      if (!lvhb || lvhb.getType() != NT_LIST)
         return;

      lvhb.ensureUnique();
      QoreListNode* l = reinterpret_cast<QoreListNode*>(lvhb.getValue());
      // take the value if it exists
      rv.assignInitial(l->takeExists(offset));
      return;
   }
   assert(tree->getOp() == OP_OBJECT_REF);

   // get the member name or names
   QoreNodeEvalOptionalRefHolder member(tree->right, xsink);
   if (*xsink)
      return;

   // find variable ptr, exit if doesn't exist anyway
   LValueHelper lvh(tree->left, xsink, true);
   if (!lvh)
      return;

   t = lvh.getType();
   if (t == NT_HASH)
      lvh.ensureUnique();

   QoreObject* o = t == NT_OBJECT ? reinterpret_cast<QoreObject* >(lvh.getValue()) : 0;
   QoreHashNode* h = !o && t == NT_HASH ? reinterpret_cast<QoreHashNode*>(lvh.getValue()) : 0;
   if (!o && !h)
      return;

   // remove a slice of the hash or object
   if (get_node_type(*member) == NT_LIST) {
      QoreHashNode* rvh = new QoreHashNode;
      rv.assignInitial(rvh);

      bool intern = o ? qore_class_private::runtimeCheckPrivateClassAccess(*o->getClass()) : false;

      ConstListIterator li(reinterpret_cast<const QoreListNode*>(*member));
      while (li.next()) {
         QoreStringValueHelper mem(li.getValue(), QCS_DEFAULT, xsink);
         if (*xsink)
            return;

         AbstractQoreNode* n = o ? qore_object_private::takeMember(*o, xsink, mem->getBuffer(), !intern) : h->takeKeyValue(mem->getBuffer());
         if (*xsink)
            return;

         // note that no exception can occur here
         rvh->setKeyValue(mem->getBuffer(), n, xsink);
         assert(!*xsink);
      }
      return;
   }

   QoreStringValueHelper mem(*member, QCS_DEFAULT, xsink);
   if (*xsink)
      return;

   rv.assignInitial(o ? o->takeMember(mem->getBuffer(), xsink) : h->takeKeyValue(mem->getBuffer()));
}

int LocalVarValue::getLValue(LValueHelper& lvh, bool for_remove) const {
   //printd(5, "LocalVarValue::getLValue() this: %p type: '%s' %d\n", this, val.getTypeName(), val.getType());
   if (val.getType() == NT_REFERENCE) {
      ReferenceNode* ref = reinterpret_cast<ReferenceNode*>(val.v.n);
      LocalRefHelper<LocalVarValue> helper(this, *ref, lvh.vl.xsink);
      return helper ? lvh.doLValue(ref, for_remove) : -1;
   }

   lvh.setValue((QoreLValueGeneric&)val);
   return 0;
}

void LocalVarValue::remove(LValueRemoveHelper& lvrh, const QoreTypeInfo* typeInfo) {
   if (val.getType() == NT_REFERENCE) {
      VarStackPointerHelper<LocalVarValue> helper(const_cast<LocalVarValue*>(this));
      ReferenceNode* ref = reinterpret_cast<ReferenceNode*>(val.v.n);
      lvrh.doRemove(lvalue_ref::get(ref)->vexp);
      return;
   }

   lvrh.doRemove((QoreLValueGeneric&)val, typeInfo);
}

int ClosureVarValue::getLValue(LValueHelper& lvh, bool for_remove) const {
   //printd(5, "ClosureVarValue::getLValue() this: %p type: '%s' %d\n", this, val.getTypeName(), val.getType());

   QoreSafeRWWriteLocker sl(const_cast<ClosureVarValue*>(this));
   if (val.getType() == NT_REFERENCE) {
      ReferenceHolder<ReferenceNode> ref(reinterpret_cast<ReferenceNode*>(val.v.n->refSelf()), lvh.vl.xsink);
      sl.unlock();
      LocalRefHelper<ClosureVarValue> helper(this, **ref, lvh.vl.xsink);
      return helper ? lvh.doLValue(*ref, for_remove) : -1;
   }

   lvh.setTypeInfo(typeInfo);
   lvh.set(*const_cast<ClosureVarValue*>(this));
   sl.stay_locked();
   lvh.setValue((QoreLValueGeneric&)val);
   return 0;
}

void ClosureVarValue::remove(LValueRemoveHelper& lvrh) {
   QoreSafeRWWriteLocker sl(this);
   if (val.getType() == NT_REFERENCE) {
      ReferenceHolder<ReferenceNode> ref(reinterpret_cast<ReferenceNode*>(val.v.n->refSelf()), lvrh.getExceptionSink());
      sl.unlock();
      // skip this entry in case it's a recursive reference
      VarStackPointerHelper<ClosureVarValue> helper(const_cast<ClosureVarValue*>(this));
      lvrh.doRemove(lvalue_ref::get(*ref)->vexp);
      return;
   }

   lvrh.doRemove((QoreLValueGeneric&)val, typeInfo);
}
