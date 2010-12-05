/* -*- mode: c++; indent-tabs-mode: nil -*- */
/*
  QoreObjectIntern.h

  Qore Programming Language

  Copyright 2003 - 2010 David Nichols

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

#ifndef _QORE_QOREOBJECTINTERN_H

#define _QORE_QOREOBJECTINTERN_H

#include <stdlib.h>
#include <assert.h>

#include <map>

#include "intern/QoreClassIntern.h"

#define OS_OK            0
#define OS_DELETED      -1

// object access constants
#define QOA_OK           0
#define QOA_PRIV_ERROR   1
#define QOA_PUB_ERROR    2

#ifdef _QORE_CYCLE_CHECK
#define QORE_DEBUG_OBJ_REFS 0
#else
#define QORE_DEBUG_OBJ_REFS 5
#endif

/*
  Qore internal class data is stored against the object with this data structure
  against its qore_classid_t (class ID).  In a class hierarchy, for private data
  that is actually a C++ subclass of Qore parent classes, then we save the same
  private data against the qore class IDs of the parent classes, but we set the
  flag to true, meaning that we will not delete the private data when
  the parent classes' destructors are run, but rather only when the subclass that
  actually owns data has its turn to destroy private object data.

  So basically, the second boolean flag just means - does this class ID actually
  own the private data or not - if it's false, then it does not actually own the data,
  but is compatible with the data, so parent class builtin (C++) methods will get
  passed this private data as if it belonged to this class and as if it were saved
  directly to the object in the class' constructor.

  please note that this flag is called the "virtual" flag elsewhere in the code here,
  meaning that the data only "virtually" belongs to the class
 */
typedef std::pair<AbstractPrivateData *, bool> private_pair_t;

// mapping from qore class ID to the object data
typedef std::map<qore_classid_t, private_pair_t> keymap_t;

// for objects with multiple classes, private data has to be keyed
class KeyList {
private:
   keymap_t keymap;

public:
   DLLLOCAL AbstractPrivateData *getReferencedPrivateData(qore_classid_t key) const {
      keymap_t::const_iterator i = keymap.find(key);
      if (i == keymap.end())
	 return 0;

      AbstractPrivateData *apd = i->second.first;
      apd->ref();
      return apd;
   }

   DLLLOCAL void addToString(QoreString *str) const {
      for (keymap_t::const_iterator i = keymap.begin(), e = keymap.end(); i != e; ++i)
	 str->sprintf("%d=<0x%p>, ", i->first, i->second.first);
   }

   DLLLOCAL void derefAll(ExceptionSink *xsink) const {
      for (keymap_t::const_iterator i = keymap.begin(), e = keymap.end(); i != e; ++i)
	 if (!i->second.second)
	    i->second.first->deref(xsink);
   }

   DLLLOCAL AbstractPrivateData *getAndClearPtr(qore_classid_t key) {
      keymap_t::iterator i = keymap.find(key);
      if (i == keymap.end())
	 return 0;

      assert(!i->second.second);
      AbstractPrivateData *rv = i->second.first;
      keymap.erase(i);
      return rv;
   }

   DLLLOCAL void insert(qore_classid_t key, AbstractPrivateData *pd) {
      assert(pd);
      keymap.insert(std::make_pair(key, std::make_pair(pd, false)));
   }

   DLLLOCAL void insertVirtual(qore_classid_t key, AbstractPrivateData *pd) {
      assert(pd);
      if (keymap.find(key) == keymap.end())
	 keymap.insert(std::make_pair(key, std::make_pair(pd, true)));
   }
};

class RecursiveSet {
protected:
   obj_set_t oset;
   qore_size_t len;

public:
   DLLLOCAL RecursiveSet(QoreObject *obj1, QoreObject *obj2) {
      oset.insert(obj1);
      if (obj1 == obj2)
         len = 1;
      else {
         oset.insert(obj2);
         len = 2;
      }
   }
   DLLLOCAL void merge(RecursiveSet *rset);
   DLLLOCAL void add(QoreObject *obj) {
      assert(oset.find(obj) == oset.end());
      oset.insert(obj);
      ++len;      
   }
   DLLLOCAL void remove(QoreObject *obj) {
      assert(oset.find(obj) != oset.end());
      oset.erase(obj);
      --len;
   }
   DLLLOCAL bool find(QoreObject *obj) {
      return oset.find(obj) != oset.end() ? true : false;
   }
   DLLLOCAL qore_size_t size() {
      return len;
   }
};

class qore_object_private {
public:
   const QoreClass *theclass;
   int status;
   mutable QoreThreadLock mutex;
   // used for references, to ensure that assignments will not deadlock when the object is locked for update
   mutable QoreThreadLock ref_mutex;
   KeyList *privateData;
   QoreReferenceCounter tRefs;  // reference-references
   QoreHashNode *data;
   QoreProgram *pgm;
   bool system_object, delete_blocker_run, in_destructor;
   QoreObject *obj;

   // recursive reference count for entire object
   unsigned rcount;

   // set of objects reachable from this object that are part of circular references
   RecursiveSet *rset;

   DLLLOCAL qore_object_private(QoreObject *n_obj, const QoreClass *oc, QoreProgram *p, QoreHashNode *n_data) : 
      theclass(oc), status(OS_OK), 
      privateData(0), data(n_data), pgm(p), system_object(!p), delete_blocker_run(false), in_destructor(false),
      obj(n_obj), rcount(0), rset(0) {
#ifdef QORE_DEBUG_OBJ_REFS
      printd(QORE_DEBUG_OBJ_REFS, "qore_object_private::qore_object_private() obj=%p, pgm=%p, class=%s, references 0->1\n", obj, p, oc->getName());
#endif
      /* instead of referencing the class, we reference the program, because the
	 program contains the namespace that contains the class, and the class'
	 methods may call functions in the program as well that could otherwise
	 disappear when the program is deleted
      */
      if (p) {
	 printd(5, "qore_object_private::qore_object_private() obj=%p (%s) calling QoreProgram::depRef() (%p)\n", obj, theclass->getName(), p);
	 p->depRef();
      }
   }

   DLLLOCAL ~qore_object_private() {
      assert(!pgm);
      assert(!data);
      assert(!privateData);
   }

   // adds a referece to an object as part of a circular reference set
   // returns -1 if object already in circular reference set, 0 if not
   DLLLOCAL int addRecursive(const char *key, QoreObject *next, bool is_new) {
      printd(0, "  addRecursive() obj=%p (%s, rcount=%d, rset=%p) '%s' -> %p (%s rcount=%d, rset=%p) is_new=%d\n", obj, obj->getClassName(), rcount, rset, key, next, next->getClassName(), next->priv->rcount, next->priv->rset, is_new);

      if (next->priv->rset) {
         if (next->priv->rset == rset) {
            // increment recursive reference count of "next"
            if (is_new)
               ++next->priv->rcount;
            return -1;
         }
         ++next->priv->rcount;

         if (!rset) {
            rset = next->priv->rset;
            rset->add(obj);
         }
         else {
            if (rset->size() >= next->priv->rset->size()) {
               rset->merge(next->priv->rset);
               delete next->priv->rset;
               next->priv->rset = rset;
            }
            else {
               next->priv->rset->merge(rset);
               delete rset;
               rset = next->priv->rset;
            }
         }

         return 0;
      }
      ++next->priv->rcount;

      if (rset) {
         next->priv->rset = rset;
         rset->add(next);
      }
      else {
         rset = new RecursiveSet(obj, next);
         next->priv->rset = rset;
      }

      return 0;
   }

   DLLLOCAL int checkRecursive(ObjMap &omap, AutoVLock &vl, ExceptionSink *xsink);

   DLLLOCAL int verifyRecursive(QoreObject *obj) {      
      if (rset && rset->find(obj)) {
         ++rcount;
         return -1;
      }
      return 0;
   }

   DLLLOCAL void plusEquals(const AbstractQoreNode *v, ObjMap &omap, AutoVLock &vl, ExceptionSink *xsink) {
      if (!v)
         return;

      // do not need ensure_unique() for objects
      if (v->getType() == NT_OBJECT) {
         ReferenceHolder<QoreHashNode> h(const_cast<QoreObject *>(reinterpret_cast<const QoreObject *>(v))->copyData(xsink), xsink);
         if (h)
            merge(*h, omap, vl, xsink);
      }
      else if (v->getType() == NT_HASH)
         merge(reinterpret_cast<const QoreHashNode *>(v), omap, vl, xsink);
   }

   DLLLOCAL void merge(const QoreHashNode *h, ObjMap &omap, AutoVLock &vl, ExceptionSink *xsink) {
      // list for saving all overwritten values to be dereferenced outside the object lock
      ReferenceHolder<QoreListNode> holder(xsink);

      {
         AutoLocker al(mutex);

         if (status == OS_DELETED) {
            makeAccessDeletedObjectException(xsink, theclass->getName());
            return;
         }

         //printd(5, "qore_object_private::merge() obj=%p\n", obj);

#ifdef _QORE_CYCLE_CHECK
         ObjectCycleHelper och(omap, obj);
#endif

         ConstHashIterator hi(h);
         while (hi.next()) {
            const QoreTypeInfo *ti;

            // check member status
            if (checkMemberAccessGetTypeInfo(hi.getKey(), ti, xsink))
               return;
            
            // check type compatibility and perform type translations, if any
            ReferenceHolder<AbstractQoreNode> val(ti->acceptInputMember(hi.getKey(), hi.getReferencedValue(), xsink), xsink);
            if (*xsink)
               return;
            
            AbstractQoreNode *n = data->swapKeyValue(hi.getKey(), val.release());
            //printd(5, "QoreObject::merge() n=%p (rc=%d, type=%s)\n", n, n ? n->isReferenceCounted() : 0, get_type_name(n));
            // if we are overwriting a value, then save it in the list for dereferencing after the lock is released
            if (n && n->isReferenceCounted()) {
               if (!holder)
                  holder = new QoreListNode;
               holder->push(n);
            }

#ifdef _QORE_CYCLE_CHECK
            printd(0, "qore_object_private::merge() obj=%p key=%s\n", obj, hi.getKey());
            omap.reset(obj, hi.getKey());
            qoreCheckContainer(const_cast<AbstractQoreNode *>(hi.getValue()), omap, vl, xsink);
#endif
         }
      }
   }

   DLLLOCAL AbstractQoreNode **getMemberValuePtr(const char *key, AutoVLock *vl, const QoreTypeInfo *&typeInfo, ObjMap &omap, ExceptionSink *xsink) const;

   DLLLOCAL QoreHashNode *getSlice(const QoreListNode *l, ExceptionSink *xsink) const {
      AutoLocker al(mutex);

      if (status == OS_DELETED) {
	 makeAccessDeletedObjectException(xsink, theclass->getName());
	 return 0;
      }

      bool has_public_members = theclass->runtimeHasPublicMembersInHierarchy();
      bool private_access_ok = runtimeCheckPrivateClassAccess(theclass);

      // check key list if necessary
      if (has_public_members || !private_access_ok) {
	 ReferenceHolder<QoreListNode> nl(new QoreListNode, xsink);
	 ConstListIterator li(l);
	 while (li.next()) {
	    QoreStringValueHelper key(li.getValue(), QCS_DEFAULT, xsink);
	    if (*xsink)
	       return 0;

	    int rc = checkMemberAccessIntern(key->getBuffer(), has_public_members, private_access_ok);
	    if (!rc)
	       nl->push(new QoreStringNode(*key));
	    else if (rc == QOA_PUB_ERROR) {
	       doPublicException(key->getBuffer(), xsink);
	       return 0;
	    }
	 }
      }

      return data->getSlice(l, xsink);
   }

   DLLLOCAL int checkMemberAccessIntern(const char *mem, bool has_public_members, bool private_access_ok) const {
      // check public access
      if (has_public_members) {
	 bool priv_member;
	 if (!theclass->isPublicOrPrivateMember(mem, priv_member))
	    return QOA_PUB_ERROR;

	 if (priv_member && !private_access_ok)
	    return QOA_PRIV_ERROR;

	 return QOA_OK;
      }

      // if accessed outside the class and the member is a private member
      return (!private_access_ok && theclass->isPrivateMember(mem)) ? QOA_PRIV_ERROR : QOA_OK;
   }

   DLLLOCAL int checkMemberAccess(const char *mem) const {
      // check public access
      if (theclass->runtimeHasPublicMembersInHierarchy()) {
	 bool priv_member;
	 if (!theclass->isPublicOrPrivateMember(mem, priv_member))
	    return QOA_PUB_ERROR;

	 if (priv_member && !runtimeCheckPrivateClassAccess(theclass))
	    return QOA_PRIV_ERROR;

	 return QOA_OK;
      }

      // if accessed outside the class and the member is a private member
      return (!runtimeCheckPrivateClassAccess(theclass) && theclass->isPrivateMember(mem)) ? QOA_PRIV_ERROR : QOA_OK;
   }

   DLLLOCAL int checkMemberAccess(const char *mem, ExceptionSink *xsink) const {
      int rc = checkMemberAccess(mem);
      if (!rc)
	 return 0;

      if (rc == QOA_PRIV_ERROR)
	 doPrivateException(mem, xsink);
      else
	 doPublicException(mem, xsink);
      return -1;
   }

   DLLLOCAL int checkMemberAccessGetTypeInfo(const char *mem, const QoreTypeInfo *&typeInfo, ExceptionSink *xsink) const {
      bool priv;
      if (theclass->runtimeGetMemberInfo(mem, typeInfo, priv)) {
	 if (priv && !runtimeCheckPrivateClassAccess(theclass)) {
	    doPrivateException(mem, xsink);
	    return -1;
	 }
	 return 0;
      }

      // member is not declared
      if (theclass->runtimeHasPublicMembersInHierarchy()) {
	 doPublicException(mem, xsink);
	 return -1;
      }
      return 0;
   }

   // lock not held on entry
   DLLLOCAL void doDeleteIntern(ExceptionSink *xsink) {
      printd(5, "qore_object_private::doDeleteIntern() execing destructor() obj=%p\n", obj);

      // increment reference count for destructor
      {
	 AutoLocker slr(ref_mutex);
	 ++obj->references;
      }

      theclass->execDestructor(obj, xsink);

      QoreHashNode *td;
      {
	 AutoLocker al(mutex);
	 assert(status != OS_DELETED);
	 assert(data);
	 status = OS_DELETED;
	 td = data;
	 data = 0;
      }
      cleanup(xsink, td);

      obj->deref(xsink);
   }

   DLLLOCAL void cleanup(ExceptionSink *xsink, QoreHashNode *td) {
      if (privateData) {
	 delete privateData;
#ifdef DEBUG
	 privateData = 0;
#endif
      }

      if (pgm) {
	 printd(5, "qore_object_private::cleanup() obj=%p (%s) calling QoreProgram::depDeref() (%p)\n", obj, theclass->getName(), pgm);
	 pgm->depDeref(xsink);
#ifdef DEBUG
	 pgm = 0;
#endif
      }

      td->deref(xsink);
   }

   // this method is called when there is an exception in a constructor and the object should be deleted
   DLLLOCAL void obliterate(ExceptionSink *xsink) {
      printd(5, "qore_object_private::obliterate() obj=%p class=%s %d->%d\n", obj, theclass->getName(), obj->references, obj->references - 1);

#ifdef QORE_DEBUG_OBJ_REFS
      printd(QORE_DEBUG_OBJ_REFS, "qore_object_private::obliterate() obj=%p class=%s: references %d->%d\n", obj, theclass->getName(), obj->references, obj->references - 1);
#endif

      {
	 AutoLocker slr(ref_mutex);
	 if (--obj->references)
	    return;
      }

      {
	 SafeLocker sl(mutex);

	 if (in_destructor || status != OS_OK) {
	    printd(5, "qore_object_private::obliterate() obj=%p data=%p in_destructor=%d status=%d\n", obj, data, in_destructor, status);
	    //printd(0, "Object lock %p unlocked (safe)\n", &mutex);
	    sl.unlock();
	    tDeref();
	    return;
	 }

	 //printd(5, "Object lock %p locked   (safe)\n", &mutex);
	 printd(5, "qore_object_private::obliterate() obj=%p class=%s\n", obj, theclass->getName());

	 status = OS_DELETED;
	 QoreHashNode *td = data;
	 data = 0;
	 //printd(0, "Object lock %p unlocked (safe)\n", &mutex);
	 sl.unlock();

	 if (privateData)
	    privateData->derefAll(xsink);

	 cleanup(xsink, td);
      }
      tDeref();
   }

   DLLLOCAL void doPrivateException(const char *mem, ExceptionSink *xsink) const {
      xsink->raiseException("PRIVATE-MEMBER", "'%s' is a private member of class '%s'", mem, theclass->getName());
   }

   DLLLOCAL void doPublicException(const char *mem, ExceptionSink *xsink) const {
      xsink->raiseException("INVALID-MEMBER", "'%s' is not a registered member of class '%s'", mem, theclass->getName());
   }

   DLLLOCAL void tRef() const {
#ifdef QORE_DEBUG_OBJ_REFS
      printd(QORE_DEBUG_OBJ_REFS, "qore_object_private::tRef() obj=%p class=%s: tref %d->%d\n", obj, theclass->getName(), tRefs.reference_count(), tRefs.reference_count() + 1);
#endif
      tRefs.ROreference();
   }

   DLLLOCAL void tDeref() {
#ifdef QORE_DEBUG_OBJ_REFS
      printd(QORE_DEBUG_OBJ_REFS, "qore_object_private::tDeref() obj=%p class=%s: tref %d->%d\n", obj, status == OS_OK ? theclass->getName() : "<deleted>", tRefs.reference_count(), tRefs.reference_count() - 1);
#endif
      if (tRefs.ROdereference())
	 delete obj;
   }

   // add virtual IDs for private data to class list
   DLLLOCAL void addVirtualPrivateData(qore_classid_t key, AbstractPrivateData *apd) {
      // first get parent class corresponding to "key"
      QoreClass *qc = theclass->getClass(key);
      assert(qc);
      BCSMList *sml = qc->getBCSMList();
      if (!sml)
	 return;

      for (class_list_t::const_iterator i = sml->begin(), e = sml->end(); i != e; ++i)
	 if ((*i).second)
	    privateData->insertVirtual((*i).first->getID(), apd);
   }

   DLLLOCAL static int checkRecursive(QoreObject *obj, ObjMap &omap, AutoVLock &vl, ExceptionSink *xsink) {
      return obj->priv->checkRecursive(omap, vl, xsink);
   }

   DLLLOCAL static int verifyRecursive(QoreObject *obj, QoreObject *other) {
      return obj->priv->verifyRecursive(other);
   }

   DLLLOCAL static int addRecursive(QoreObject *obj, const char *key, QoreObject *next, bool is_new) {
      return obj->priv->addRecursive(key, next, is_new);
   }

   DLLLOCAL static AbstractQoreNode **getMemberValuePtr(const QoreObject *obj, const char *key, AutoVLock *vl, const QoreTypeInfo *&typeInfo, ObjMap &omap, ExceptionSink *xsink) {
      return obj->priv->getMemberValuePtr(key, vl, typeInfo, omap, xsink);
   }

   DLLLOCAL static void plusEquals(QoreObject *obj, const AbstractQoreNode *v, ObjMap &omap, AutoVLock &vl, ExceptionSink *xsink) {
      obj->priv->plusEquals(v, omap, vl, xsink);
   }
};

class qore_object_lock_handoff_helper {
private:
   qore_object_private *pobj;
   AutoVLock &vl;

public:
   DLLLOCAL qore_object_lock_handoff_helper(qore_object_private *n_pobj, AutoVLock &n_vl) : pobj(n_pobj), vl(n_vl) {
      if (pobj->obj == vl.getObject()) {
	 assert(vl.get() == &pobj->mutex);
	 vl.clear();
	 return;
      }

      // reference current object
      pobj->obj->tRef();

      // unlock previous lock and release from AutoVLock structure
      vl.del();

      // lock current object
      pobj->mutex.lock();
   }

   DLLLOCAL ~qore_object_lock_handoff_helper() {
      // unlock if lock not saved in AutoVLock structure
      if (pobj) {
	 //printd(0, "Object lock %p unlocked (handoff)\n", &pobj->mutex);
	 pobj->mutex.unlock();
	 pobj->obj->tDeref();
      }
   }

   DLLLOCAL void stay_locked() {
      vl.set(pobj->obj, &pobj->mutex);
      pobj = 0;
   }
};

class qore_object_recursive_lock_handoff_helper {
private:
   qore_object_private *pobj;
   AutoVLock &vl;
   bool locked;

public:
   DLLLOCAL qore_object_recursive_lock_handoff_helper(qore_object_private *n_pobj, AutoVLock &n_vl) : pobj(n_pobj), vl(n_vl) {
      // try to lock current object
      locked = !pobj->mutex.trylock();
   }

   DLLLOCAL ~qore_object_recursive_lock_handoff_helper() {
      // unlock current object
      if (locked)
         pobj->mutex.unlock();
   }

   DLLLOCAL operator bool() const {
      return locked;
   }
};

#endif