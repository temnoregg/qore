/*
  QoreClassIntern.h

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

#ifndef _QORE_QORECLASSINTERN_H

#define _QORE_QORECLASSINTERN_H

#include <qore/safe_dslist>

#include <list>
#include <map>
#include <string>

#define OTF_USER    CT_USER
#define OTF_BUILTIN CT_BUILTIN

// forward reference for base class (constructor) argument list
class BCAList;
// forward reference for base class list
class BCList;
// forward reference for base class (constructor) evaluated argument list
class BCEAList;

class MethodVariantBase : public AbstractQoreFunctionVariant {
protected:
   // is the method private or not
   bool priv_flag;

public:
   DLLLOCAL MethodVariantBase(bool n_priv_flag) : priv_flag(n_priv_flag) {
   }
   DLLLOCAL bool isPrivate() const {
      return priv_flag;
   }
};

#define METHVB(f) (reinterpret_cast<MethodVariantBase *>(f))
#define METHVB_const(f) (reinterpret_cast<const MethodVariantBase *>(f))

class MethodVariant : public MethodVariantBase {
public:
   DLLLOCAL MethodVariant(bool n_priv_flag) : MethodVariantBase(n_priv_flag) {
   }
   DLLLOCAL virtual AbstractQoreNode *evalNormalMethod(const QoreMethod &method, QoreObject *self, const QoreListNode *args, ExceptionSink *xsink) const = 0;
   DLLLOCAL virtual AbstractQoreNode *evalStaticMethod(const QoreMethod &method, const QoreListNode *args, ExceptionSink *xsink) const = 0;
};

#define METHV(f) (reinterpret_cast<MethodVariant *>(f))
#define METHV_const(f) (reinterpret_cast<const MethodVariant *>(f))

class ConstructorMethodVariant : public MethodVariantBase {
protected:
   // evaluates base class constructors and initializes members
   DLLLOCAL int constructorPrelude(const QoreClass &thisclass, CodeEvaluationHelper &ceh, QoreObject *self, BCList *bcl, BCEAList *bceal, ExceptionSink *xsink) const;

public:
   DLLLOCAL ConstructorMethodVariant(bool n_priv_flag) : MethodVariantBase(n_priv_flag) {
   }
   DLLLOCAL virtual const BCAList *getBaseClassArgumentList() const = 0;
   DLLLOCAL virtual void evalConstructor(const QoreClass &thisclass, QoreObject *self, CodeEvaluationHelper &ceh, BCList *bcl, BCEAList *bceal, ExceptionSink *xsink) const = 0;
};

#define CONMV(f) (reinterpret_cast<ConstructorMethodVariant *>(f))
#define CONMV_const(f) (reinterpret_cast<const ConstructorMethodVariant *>(f))

class DestructorMethodVariant : public MethodVariantBase {
protected:
public:
   DLLLOCAL DestructorMethodVariant() : MethodVariantBase(false) {
   }
   DLLLOCAL virtual void evalDestructor(const QoreClass &thisclass, QoreObject *self, ExceptionSink *xsink) const = 0;
};

#define DESMV(f) (reinterpret_cast<DestructorMethodVariant *>(f))
#define DESMV_const(f) (reinterpret_cast<const DestructorMethodVariant *>(f))

class CopyMethodVariant : public MethodVariantBase {
protected:
public:
   DLLLOCAL CopyMethodVariant(bool n_priv_flag) : MethodVariantBase(n_priv_flag) {
   }
   DLLLOCAL virtual void evalCopy(const QoreClass &thisclass, QoreObject *self, QoreObject *old, CodeEvaluationHelper &ceh, BCList *scl, ExceptionSink *xsink) const = 0;
};

#define COPYMV(f) (reinterpret_cast<CopyMethodVariant *>(f))
#define COPYMV_const(f) (reinterpret_cast<const CopyMethodVariant *>(f))

class UserMethodVariant : public MethodVariant, public UserVariantBase {
public:
   DLLLOCAL UserMethodVariant(bool n_priv_flag, StatementBlock *b, int n_sig_first_line, int n_sig_last_line, AbstractQoreNode *params, QoreParseTypeInfo *rv, bool synced) : MethodVariant(n_priv_flag), UserVariantBase(b, n_sig_first_line, n_sig_last_line, params, rv, synced) {
   }

   // the following defines the pure virtual functions that are common to all user variants
   COMMON_USER_VARIANT_FUNCTIONS

   DLLLOCAL void parseInitMethod(const QoreClass &parent_class, bool static_flag) {
      // resolve and push current return type on stack
      ReturnTypeInfoHelper rtih(signature.parseGetReturnTypeInfo());
      
      // must be called even if statements is NULL
      if (!static_flag)
	 statements->parseInitMethod(parent_class.getTypeInfo(), &signature);
      else
	 statements->parseInit(&signature);
   }
   DLLLOCAL virtual AbstractQoreNode *evalStaticMethod(const QoreMethod &method, const QoreListNode *args, ExceptionSink *xsink) const {
      return eval(method.getName(), args, 0, xsink, method.getClass()->getName());
   }
   DLLLOCAL virtual AbstractQoreNode *evalNormalMethod(const QoreMethod &method, QoreObject *self, const QoreListNode *args, ExceptionSink *xsink) const {
      return eval(method.getName(), args, self, xsink, method.getClass()->getName());
   }
};

#define UMV(f) (reinterpret_cast<UserMethodVariant *>(f))
#define UMV_const(f) (reinterpret_cast<const UserMethodVariant *>(f))

class UserConstructorVariant : public ConstructorMethodVariant, public UserVariantBase {
protected:
   // base class argument list for constructors
   BCAList *bcal;

public:
   DLLLOCAL UserConstructorVariant(bool n_priv_flag, StatementBlock *b, int n_sig_first_line, int n_sig_last_line, AbstractQoreNode *params, BCAList *n_bcal) : ConstructorMethodVariant(n_priv_flag), UserVariantBase(b, n_sig_first_line, n_sig_last_line, params, 0, false), bcal(n_bcal) {
   }
   DLLLOCAL ~UserConstructorVariant();

   // the following defines the pure virtual functions that are common to all user variants
   COMMON_USER_VARIANT_FUNCTIONS

   DLLLOCAL virtual const BCAList *getBaseClassArgumentList() const {
      return bcal;
   }

   DLLLOCAL virtual void evalConstructor(const QoreClass &thisclass, QoreObject *self, CodeEvaluationHelper &ceh, BCList *bcl, BCEAList *bceal, ExceptionSink *xsink) const {
      ReferenceHolder<QoreListNode> argv(xsink);
      if (setupCall(ceh.getArgs(), argv, xsink))
	 return;

      CodeContextHelper cch("constructor", self, xsink);
#ifdef QORE_RUNTIME_THREAD_STACK_TRACE
      // push call on call stack in debugging mode
      CallStackHelper csh("constructor", CT_USER, self, xsink);
#endif

      if (constructorPrelude(thisclass, ceh, self, bcl, bceal, xsink))
	 return;

      discard(evalIntern(argv, self, xsink, thisclass.getName()), xsink);
   }

   DLLLOCAL void parseInitConstructor(const QoreClass &parent_class, LocalVar &selfid, BCList *bcl);
};

#define UCONV(f) (reinterpret_cast<UserConstructorVariant *>(f))
#define UCONV_const(f) (reinterpret_cast<const UserConstructorVariant *>(f))

class UserDestructorVariant : public DestructorMethodVariant, public UserVariantBase {
protected:
public:
   DLLLOCAL UserDestructorVariant(StatementBlock *b, int n_sig_first_line, int n_sig_last_line) : DestructorMethodVariant(), UserVariantBase(b, n_sig_first_line, n_sig_last_line, 0, 0, false) {
   }

   // the following defines the pure virtual functions that are common to all user variants
   COMMON_USER_VARIANT_FUNCTIONS

   DLLLOCAL void parseInitDestructor(const QoreClass &parent_class) {
      assert(!signature.getReturnTypeInfo());

      // push return type on stack (no return value can be used)
      ReturnTypeInfoHelper rtih(nothingTypeInfo);

      // must be called even if statements is NULL
      statements->parseInitMethod(parent_class.getTypeInfo(), &signature);
   }

   DLLLOCAL virtual void evalDestructor(const QoreClass &thisclass, QoreObject *self, ExceptionSink *xsink) const {
      // there cannot be any params
      assert(!signature.numParams());
      discard(eval("destructor", 0, self, xsink, thisclass.getName()), xsink);
   }
};

#define UDESV(f) (reinterpret_cast<UserDestructorVariant *>(f))
#define UDESV_const(f) (reinterpret_cast<const UserDestructorVariant *>(f))

class UserCopyVariant : public CopyMethodVariant, public UserVariantBase {
protected:
public:
   DLLLOCAL UserCopyVariant(bool n_priv_flag, StatementBlock *b, int n_sig_first_line, int n_sig_last_line, AbstractQoreNode *params, QoreParseTypeInfo *rv, bool synced) : CopyMethodVariant(n_priv_flag), UserVariantBase(b, n_sig_first_line, n_sig_last_line, params, rv, synced) {
   }

   // the following defines the pure virtual functions that are common to all user variants
   COMMON_USER_VARIANT_FUNCTIONS

   DLLLOCAL void parseInitCopy(const QoreClass &parent_class);
   DLLLOCAL virtual void evalCopy(const QoreClass &thisclass, QoreObject *self, QoreObject *old, CodeEvaluationHelper &ceh, BCList *scl, ExceptionSink *xsink) const;
};

#define UCOPYV(f) (reinterpret_cast<UserCopyVariant *>(f))

class BuiltinMethodVariant : public MethodVariant, public BuiltinFunctionVariantBase {
public:
   DLLLOCAL BuiltinMethodVariant(bool n_priv_flag, int n_functionality, const QoreTypeInfo *n_returnTypeInfo, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : MethodVariant(n_priv_flag), BuiltinFunctionVariantBase(n_functionality, n_returnTypeInfo, n_num_params, n_typeList, n_defaultArgList) {}

   // the following defines the pure virtual functions that are common to all builtin variants
   COMMON_BUILTIN_VARIANT_FUNCTIONS
};

class BuiltinNormalMethodVariantBase : public BuiltinMethodVariant {
public:
   DLLLOCAL BuiltinNormalMethodVariantBase(bool n_priv_flag, int n_functionality, const QoreTypeInfo *n_returnTypeInfo, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinMethodVariant(n_priv_flag, n_functionality, n_returnTypeInfo, n_num_params, n_typeList, n_defaultArgList) {}

   DLLLOCAL virtual AbstractQoreNode *evalNormalMethod(const QoreMethod &method, QoreObject *self, const QoreListNode *args, ExceptionSink *xsink) const {
      CODE_CONTEXT_HELPER(CT_BUILTIN, method.getName(), self, xsink);

      return self->evalBuiltinMethodWithPrivateData(method, this, args, xsink);
   }
   // this function should never be called
   DLLLOCAL virtual AbstractQoreNode *evalStaticMethod(const QoreMethod &method, const QoreListNode *args, ExceptionSink *xsink) const {
      assert(false);
      return 0;
   }
   DLLLOCAL virtual AbstractQoreNode *evalImpl(const QoreMethod &meth, QoreObject *self, AbstractPrivateData *private_data, const QoreListNode *args, ExceptionSink *xsink) const = 0;
};

class BuiltinNormalMethodVariant : public BuiltinNormalMethodVariantBase {
protected:
   q_method_t method;

public:
   DLLLOCAL BuiltinNormalMethodVariant(q_method_t m, bool n_priv_flag, int n_functionality = QDOM_DEFAULT, const QoreTypeInfo *n_returnTypeInfo = 0, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinNormalMethodVariantBase(n_priv_flag, n_functionality, n_returnTypeInfo, n_num_params, n_typeList, n_defaultArgList), method(m) {
   }
   DLLLOCAL virtual AbstractQoreNode *evalImpl(const QoreMethod &meth, QoreObject *self, AbstractPrivateData *private_data, const QoreListNode *args, ExceptionSink *xsink) const {
      return method(self, private_data, args, xsink);
   }
};

class BuiltinNormalMethod2Variant : public BuiltinNormalMethodVariantBase {
protected:
   q_method2_t method;

public:
   DLLLOCAL BuiltinNormalMethod2Variant(q_method2_t m, bool n_priv_flag, int n_functionality = QDOM_DEFAULT, const QoreTypeInfo *n_returnTypeInfo = 0, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinNormalMethodVariantBase(n_priv_flag, n_functionality, n_returnTypeInfo, n_num_params, n_typeList, n_defaultArgList), method(m) {
   }
   DLLLOCAL virtual AbstractQoreNode *evalImpl(const QoreMethod &meth, QoreObject *self, AbstractPrivateData *private_data, const QoreListNode *args, ExceptionSink *xsink) const {
      return method(meth, self, private_data, args, xsink);
   }
};

class BuiltinStaticMethodVariant : public BuiltinMethodVariant {
protected:
   q_func_t static_method;

public:
   DLLLOCAL BuiltinStaticMethodVariant(q_func_t m, bool n_priv_flag, int n_functionality = QDOM_DEFAULT, const QoreTypeInfo *n_returnTypeInfo = 0, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinMethodVariant(n_priv_flag, n_functionality, n_returnTypeInfo, n_num_params, n_typeList, n_defaultArgList), static_method(m) {
   }
   DLLLOCAL virtual AbstractQoreNode *evalStaticMethod(const QoreMethod &method, const QoreListNode *args, ExceptionSink *xsink) const {
      CODE_CONTEXT_HELPER(CT_BUILTIN, method.getName(), 0, xsink);

      return static_method(args, xsink);
   }
   // this function should never be called
   DLLLOCAL virtual AbstractQoreNode *evalNormalMethod(const QoreMethod &method, QoreObject *self, const QoreListNode *args, ExceptionSink *xsink) const {
      assert(false);
      return 0;
   }
};

class BuiltinStaticMethod2Variant : public BuiltinMethodVariant {
protected:
   q_static_method2_t static_method;

public:
   DLLLOCAL BuiltinStaticMethod2Variant(q_static_method2_t m, bool n_priv_flag, int n_functionality = QDOM_DEFAULT, const QoreTypeInfo *n_returnTypeInfo = 0, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinMethodVariant(n_priv_flag, n_functionality, n_returnTypeInfo, n_num_params, n_typeList, n_defaultArgList), static_method(m) {
   }
   DLLLOCAL AbstractQoreNode *evalStaticMethod(const QoreMethod &method, const QoreListNode *args, ExceptionSink *xsink) const {
      CODE_CONTEXT_HELPER(CT_BUILTIN, method.getName(), 0, xsink);

      return static_method(method, args, xsink);
   }
   // this function should never be called
   DLLLOCAL virtual AbstractQoreNode *evalNormalMethod(const QoreMethod &method, QoreObject *self, const QoreListNode *args, ExceptionSink *xsink) const {
      assert(false);
      return 0;
   }
};

class BuiltinConstructorVariantBase : public ConstructorMethodVariant, public BuiltinFunctionVariantBase {
public:
   // return type info is set to 0 because the new operator actually returns the new object, not the constructor
   DLLLOCAL BuiltinConstructorVariantBase(bool n_priv_flag, int n_functionality = QDOM_DEFAULT, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : ConstructorMethodVariant(n_priv_flag), BuiltinFunctionVariantBase(n_functionality, 0, n_num_params, n_typeList, n_defaultArgList) {
   }

   // the following defines the pure virtual functions that are common to all builtin variants
   COMMON_BUILTIN_VARIANT_FUNCTIONS

   DLLLOCAL virtual const BCAList *getBaseClassArgumentList() const {
      return 0;
   }
};

class BuiltinConstructorVariant : public BuiltinConstructorVariantBase {
protected:
   q_constructor_t constructor;

public:
   DLLLOCAL BuiltinConstructorVariant(q_constructor_t m, bool n_priv_flag, int n_functionality = QDOM_DEFAULT, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinConstructorVariantBase(n_priv_flag, n_functionality, n_num_params, n_typeList, n_defaultArgList), constructor(m) {
   }

   DLLLOCAL virtual void evalConstructor(const QoreClass &thisclass, QoreObject *self, CodeEvaluationHelper &ceh, BCList *bcl, BCEAList *bceal, ExceptionSink *xsink) const {
      CodeContextHelper cch("constructor", self, xsink);
#ifdef QORE_RUNTIME_THREAD_STACK_TRACE
      // push call on call stack
      CallStackHelper csh("constructor", CT_BUILTIN, self, xsink);
#endif

      if (constructorPrelude(thisclass, ceh, self, bcl, bceal, xsink))
	 return;

      constructor(self, ceh.getArgs(), xsink);
   }
};

class BuiltinConstructor2Variant : public BuiltinConstructorVariantBase {
protected:
   q_constructor2_t constructor;

public:
   // return type info is set to 0 because the new operator actually returns the new object, not the constructor
   DLLLOCAL BuiltinConstructor2Variant(q_constructor2_t m, bool n_priv_flag, int n_functionality = QDOM_DEFAULT, unsigned n_num_params = 0, const QoreTypeInfo **n_typeList = 0, const AbstractQoreNode **n_defaultArgList = 0) : BuiltinConstructorVariantBase(n_priv_flag, n_functionality, n_num_params, n_typeList, n_defaultArgList), constructor(m) {
   }
   DLLLOCAL virtual void evalConstructor(const QoreClass &thisclass, QoreObject *self, CodeEvaluationHelper &ceh, BCList *bcl, BCEAList *bceal, ExceptionSink *xsink) const { 
      CodeContextHelper cch("constructor", self, xsink);
#ifdef QORE_RUNTIME_THREAD_STACK_TRACE
      // push call on call stack
      CallStackHelper csh("constructor", CT_BUILTIN, self, xsink);
#endif

      if (constructorPrelude(thisclass, ceh, self, bcl, bceal, xsink))
	 return;

      constructor(thisclass, self, ceh.getArgs(), xsink);
   }
};

class BuiltinDestructorVariantBase : public DestructorMethodVariant, public BuiltinFunctionVariantBase {
public:
   // the following defines the pure virtual functions that are common to all builtin variants
   COMMON_BUILTIN_VARIANT_FUNCTIONS
};

class BuiltinDestructorVariant : public BuiltinDestructorVariantBase {
protected:
   q_destructor_t destructor;

public:
   DLLLOCAL BuiltinDestructorVariant(q_destructor_t n_destructor) : destructor(n_destructor) {
   }

   DLLLOCAL virtual void evalDestructor(const QoreClass &thisclass, QoreObject *self, ExceptionSink *xsink) const {
      CODE_CONTEXT_HELPER(CT_BUILTIN, "destructor", self, xsink);

      AbstractPrivateData *private_data = self->getAndClearPrivateData(thisclass.getID(), xsink);
      if (!private_data)
	 return;
      destructor(self, private_data, xsink);
   }
};

class BuiltinDestructor2Variant : public BuiltinDestructorVariantBase {
protected:
   q_destructor2_t destructor;

public:
   DLLLOCAL BuiltinDestructor2Variant(q_destructor2_t n_destructor) : destructor(n_destructor) {
   }
   DLLLOCAL virtual void evalDestructor(const QoreClass &thisclass, QoreObject *self, ExceptionSink *xsink) const {
      CODE_CONTEXT_HELPER(CT_BUILTIN, "destructor", self, xsink);

      AbstractPrivateData *private_data = self->getAndClearPrivateData(thisclass.getID(), xsink);
      if (!private_data)
	 return;
      destructor(thisclass, self, private_data, xsink);
   }
};

class BuiltinCopyVariantBase : public CopyMethodVariant, public BuiltinFunctionVariantBase {
protected:
public:
   DLLLOCAL BuiltinCopyVariantBase(const QoreClass *c) : CopyMethodVariant(false), BuiltinFunctionVariantBase(QDOM_DEFAULT, c->getTypeInfo()) {
   }

   // the following defines the pure virtual functions that are common to all builtin variants
   COMMON_BUILTIN_VARIANT_FUNCTIONS

   DLLLOCAL virtual void evalCopy(const QoreClass &thisclass, QoreObject *self, QoreObject *old, CodeEvaluationHelper &ceh, BCList *scl, ExceptionSink *xsink) const;
   DLLLOCAL virtual void evalImpl(const QoreClass &thisclass, QoreObject *self, QoreObject *old, AbstractPrivateData *private_data, ExceptionSink *xsink) const = 0;
};

class BuiltinCopyVariant : public BuiltinCopyVariantBase {
protected:
   q_copy_t copy;

public:
   DLLLOCAL BuiltinCopyVariant(QoreClass *c, q_copy_t m) : BuiltinCopyVariantBase(c), copy(m) {
   }
   DLLLOCAL virtual void evalImpl(const QoreClass &thisclass, QoreObject *self, QoreObject *old, AbstractPrivateData *private_data, ExceptionSink *xsink) const {
      copy(self, old, private_data, xsink);
   }
};

class BuiltinCopy2Variant : public BuiltinCopyVariantBase {
protected:
   q_copy2_t copy;

public:
   DLLLOCAL BuiltinCopy2Variant(QoreClass *c, q_copy2_t m) : BuiltinCopyVariantBase(c), copy(m) {
   }
   DLLLOCAL virtual void evalImpl(const QoreClass &thisclass, QoreObject *self, QoreObject *old, AbstractPrivateData *private_data, ExceptionSink *xsink) const {
      copy(thisclass, self, old, private_data, xsink);
   }
};

// abstract class for method functions (static and non-static)
class MethodFunction : public MethodFunctionBase {
public:
   DLLLOCAL virtual ~MethodFunction() {
   }
   DLLLOCAL virtual void parseInitMethod(const QoreClass &parent_class, bool static_flag);

   // if the variant was identified at parse time, then variant will not be NULL, otherwise if NULL then it is identified at run time
   DLLLOCAL AbstractQoreNode *evalNormalMethod(const AbstractQoreFunctionVariant *variant, const QoreMethod &method, QoreObject *self, const QoreListNode *args, ExceptionSink *xsink) const;

   // if the variant was identified at parse time, then variant will not be NULL, otherwise if NULL then it is identified at run time
   DLLLOCAL AbstractQoreNode *evalStaticMethod(const AbstractQoreFunctionVariant *variant, const QoreMethod &method, const QoreListNode *args, ExceptionSink *xsink) const;
};

#define METHF(f) (reinterpret_cast<MethodFunction *>(f))

// abstract class for constructor method functions
class ConstructorMethodFunction : public MethodFunctionBase {
public:
   DLLLOCAL void parseInitConstructor(const QoreClass &parent_class, LocalVar &selfid, BCList *bcl);
   DLLLOCAL virtual const char *getName() const {
      return "constructor";
   }
   // if the variant was identified at parse time, then variant will not be NULL, otherwise if NULL then it is identified at run time
   DLLLOCAL void evalConstructor(const AbstractQoreFunctionVariant *variant, const QoreClass &thisclass, QoreObject *self, const QoreListNode *args, BCList *bcl, BCEAList *bceal, ExceptionSink *xsink) const;
};

#define CONMF(f) (reinterpret_cast<ConstructorMethodFunction *>(f))

// abstract class for destructor method functions
class DestructorMethodFunction : public MethodFunctionBase {
public:
   DLLLOCAL void parseInitDestructor(const QoreClass &parent_class);
   DLLLOCAL virtual const char *getName() const {
      return "destructor";
   }
   DLLLOCAL void evalDestructor(const QoreClass &thisclass, QoreObject *self, ExceptionSink *xsink) const;
};

#define DESMF(f) (reinterpret_cast<DestructorMethodFunction *>(f))

// abstract class for copy method functions
class CopyMethodFunction : public MethodFunctionBase {
public:
   DLLLOCAL void parseInitCopy(const QoreClass &parent_class);
   DLLLOCAL virtual const char *getName() const {
      return "copy";
   }
   DLLLOCAL void evalCopy(const QoreClass &thisclass, QoreObject *self, QoreObject *old, BCList *scl, ExceptionSink *xsink) const;
};

#define COPYMF(f) (reinterpret_cast<CopyMethodFunction *>(f))

class BuiltinSystemConstructorBase : public MethodFunctionBase {
public:
   DLLLOCAL BuiltinSystemConstructorBase() {
   }
   DLLLOCAL const char *getName() const {
      return "<system_constructor>";
   }
   DLLLOCAL virtual void eval(const QoreClass &thisclass, QoreObject *self, int code, va_list args) const = 0;
};

#define BSYSCONB(f) (reinterpret_cast<BuiltinSystemConstructorBase *>(f))

// system constructors are not accessed from userspace so we don't need to conform
// to the abstract class structure
class BuiltinSystemConstructor : public BuiltinSystemConstructorBase {
protected:
   q_system_constructor_t system_constructor;

public:
   DLLLOCAL BuiltinSystemConstructor(q_system_constructor_t m) : system_constructor(m) {
   }
   DLLLOCAL virtual void eval(const QoreClass &thisclass, QoreObject *self, int code, va_list args) const {
      system_constructor(self, code, args);
   }
};

class BuiltinSystemConstructor2 : public BuiltinSystemConstructorBase {
protected:
   q_system_constructor2_t system_constructor;

public:
   DLLLOCAL BuiltinSystemConstructor2(q_system_constructor2_t m) : system_constructor(m) {
   }

   DLLLOCAL virtual void eval(const QoreClass &thisclass, QoreObject *self, int code, va_list args) const {
      system_constructor(thisclass, self, code, args);
   }
};

class BuiltinMethod : public MethodFunction, public BuiltinFunctionBase {
public:
   DLLLOCAL BuiltinMethod(const char *mname) : BuiltinFunctionBase(mname) {
   }
   DLLLOCAL virtual const char *getName() const { 
      return name;
   }
};

// not visible to user code, does not follow abstract class pattern
class BuiltinDeleteBlocker : public BuiltinMethod {
protected:
   q_delete_blocker_t delete_blocker;

public:
   DLLLOCAL BuiltinDeleteBlocker(q_delete_blocker_t m) : BuiltinMethod("<delete_blocker>"), delete_blocker(m) {
   }   
   DLLLOCAL bool eval(QoreObject *self, AbstractPrivateData *private_data) const {
      return delete_blocker(self, private_data);
   }
};

#define BDELB(f) (reinterpret_cast<BuiltinDeleteBlocker *>(f))

class UserMethodBase {
protected:
   std::string name;

   DLLLOCAL UserMethodBase(const char *mname) : name(mname) {
   }
};

class UserMethod : public MethodFunction, public UserMethodBase {
public:
   DLLLOCAL UserMethod(const char *mname) : UserMethodBase(mname) {
   }
   DLLLOCAL virtual const char *getName() const {
      return name.c_str();
   }
};

struct QoreMemberInfo : public QoreParseTypeInfo {
   AbstractQoreNode *exp;
   // store parse location in case of errors
   int first_line, last_line;
   const char *file;

   DLLLOCAL QoreMemberInfo(int nfl, int nll, qore_type_t t, AbstractQoreNode *e = 0) : QoreParseTypeInfo(t), exp(e), first_line(nfl), last_line(nll) {
      file = get_parse_file();
   }
   DLLLOCAL QoreMemberInfo(int nfl, int nll, char *n, AbstractQoreNode *e = 0) : QoreParseTypeInfo(n), exp(e), first_line(nfl), last_line(nll) {
      file = get_parse_file();
   }
   DLLLOCAL QoreMemberInfo(int nfl, int nll, const QoreClass *qc, AbstractQoreNode *e) : QoreParseTypeInfo(qc), exp(e), first_line(nfl), last_line(nll) {
      file = get_parse_file();
   }
   DLLLOCAL ~QoreMemberInfo() {
      if (exp)
	 exp->deref(0);
   }
   DLLLOCAL QoreMemberInfo *copy() const {
      if (!this)
         return 0;

      assert(has_type);
      QoreMemberInfo *mi;
      if (qc)
         mi = new QoreMemberInfo(first_line, last_line, qc, exp ? exp->refSelf() : 0);
      else
	 mi = new QoreMemberInfo(first_line, last_line, qt, exp ? exp->refSelf() : 0);
      mi->file = file;
      return mi;
   }

   DLLLOCAL void parseInit(const char *name, bool priv) {
      resolve();

      if (exp) {
	 const QoreTypeInfo *argTypeInfo = 0;
	 int lvids = 0;
	 exp = exp->parseInit(0, 0, lvids, argTypeInfo);
	 if (lvids) {
	    update_parse_location(first_line, last_line, file);
	    parse_error("illegal local variable declaration in member initialization expression");
	    while (lvids)
	       pop_local_var();
	 }
	 // throw a type exception only if parse exceptions are enabled
	 if (!parseEqual(argTypeInfo) && getProgram()->getParseExceptionSink()) {
            QoreStringNode *desc = new QoreStringNode("initialization expression for ");
	    desc->sprintf("%s member '$.%s' returns ", priv ? "private" : "public", name);
            argTypeInfo->getThisType(*desc);
            desc->concat(", but the member was declared as ");
            getThisType(*desc);
	    update_parse_location(first_line, last_line, file);
            getProgram()->makeParseException("PARSE-TYPE-ERROR", desc);
         }
      }
#if 0
      else if (hasType() && qt == NT_OBJECT) {
	 update_parse_location(first_line, last_line, file);
	 parseException("PARSE-TYPE-ERROR", "%s member '$.%s' has been defined with a complex type and must be assigned when instantiated", priv ? "private" : "public", name);
      }
#endif
   }
};

typedef std::map<char *, QoreMemberInfo *, ltstr> member_map_t;

class QoreMemberMap : public member_map_t {
public:
   DLLLOCAL ~QoreMemberMap() {
      member_map_t::iterator j;
      while ((j = begin()) != end()) {
         char *n = j->first;
         delete j->second;
         erase(j);
         //printd(5, "QoreMemberMap::~QoreMemberMap() this=%p freeing pending private member %p '%s'\n", this, n, n);
         free(n);
      }
   }
};

/*
  BCANode
  base class constructor argument node
*/
class BCANode : public FunctionCallBase {
public:
   QoreClass *sclass;
   NamedScope *ns;
   char *name;

   // this method takes ownership of n and arg
   DLLLOCAL BCANode(NamedScope *n, QoreListNode *n_args) : FunctionCallBase(n_args), sclass(0), ns(n), name(0) {
   }
   // this method takes ownership of n and arg
   DLLLOCAL BCANode(char *n, QoreListNode *n_args) : FunctionCallBase(n_args), sclass(0), ns(0), name(n) {
   }
   DLLLOCAL ~BCANode() {
      delete ns;
      if (name)
	 free(name);
   }
   // resolves classes, parses arguments, and attempts to find constructor variant
   DLLLOCAL void parseInit(BCList *bcl, const char *classname);
};

typedef safe_dslist<BCANode *> bcalist_t;

//  BCAList
//  base class constructor argument list
//  this data structure will not be modified even if the class is copied
//  to a subprogram object
class BCAList : public bcalist_t {
public:
   DLLLOCAL BCAList(BCANode *n);
   DLLLOCAL ~BCAList();
   // returns 0 for no errors, -1 for exception raised
   DLLLOCAL int execBaseClassConstructorArgs(BCEAList *bceal, ExceptionSink *xsink) const;
};

typedef std::pair<QoreClass *, bool> class_virt_pair_t;
typedef std::list<class_virt_pair_t> class_list_t;

// BCSMList: Base Class Special Method List
// unique list of base classes for a class hierarchy to ensure that "special" methods, constructor(), destructor(), copy() - are executed only once
// this class also tracks virtual classes to ensure that they are not inserted into the list in a complex tree and executed here
class BCSMList : public class_list_t {
   public:
      DLLLOCAL void add(QoreClass *thisclass, QoreClass *qc, bool is_virtual);
      DLLLOCAL void addBaseClassesToSubclass(QoreClass *thisclass, QoreClass *sc, bool is_virtual);
      DLLLOCAL bool isBaseClass(QoreClass *qc) const;
      DLLLOCAL QoreClass *getClass(qore_classid_t cid) const;
      //DLLLOCAL void execConstructors(QoreObject *o, BCEAList *bceal, ExceptionSink *xsink) const;
      DLLLOCAL void execDestructors(QoreObject *o, ExceptionSink *xsink) const;
      DLLLOCAL void execSystemDestructors(QoreObject *o, ExceptionSink *xsink) const;
      DLLLOCAL void execCopyMethods(QoreObject *self, QoreObject *old, ExceptionSink *xsink) const;
};

// BCNode 
// base class pointer, also stores arguments for base class constructors
class BCNode {
public:
   NamedScope *cname;
   char *cstr;
   QoreClass *sclass;
   bool priv : 1;
   bool is_virtual : 1;
   
   // FIXME: check new BCNode(...)! constructors changed

   DLLLOCAL BCNode(NamedScope *c, bool p) : cname(c), cstr(0), sclass(0), priv(p), is_virtual(false) {
   }
   // this method takes ownership of *str
   DLLLOCAL BCNode(char *str, bool p) : cname(0), cstr(str), sclass(0), priv(p), is_virtual(false) {
   }
   // for builtin base classes
   DLLLOCAL BCNode(QoreClass *qc, bool n_virtual = false) 
      : cname(0), cstr(0), sclass(qc), priv(false), is_virtual(n_virtual) {
   }
   DLLLOCAL ~BCNode();
   DLLLOCAL bool isPrivate() const { return priv; }
   DLLLOCAL const QoreClass *getClass(qore_classid_t cid, bool &n_priv) const {
      assert(sclass);
      const QoreClass *qc = (sclass->getID() == cid) ? sclass : sclass->getClassIntern(cid, n_priv);
      if (qc && !n_priv && priv)
	 n_priv = true;
      return qc;
   }
};

typedef safe_dslist<BCNode *> bclist_t;

//  BCList
//  linked list of base classes, constructors called head->tail, 
//  destructors called in reverse order (tail->head) (stored in BCSMList)
//  note that this data structure cannot be modified even if the class is
//  copied to a subprogram object and extended
//  this class is a QoreReferenceCounter so it won't be copied when the class is copied
class BCList : public QoreReferenceCounter, public bclist_t {
protected:
   DLLLOCAL inline ~BCList();
      
public:
   // special method (constructor, destructor, copy) list for superclasses
   BCSMList sml;

   DLLLOCAL BCList(BCNode *n) {
      push_back(n);
   }
   DLLLOCAL BCList() {
   }
   DLLLOCAL void parseInit(QoreClass *thisclass, bool &has_delete_blocker);
   DLLLOCAL const QoreMethod *parseResolveSelfMethod(const char *name);

   // only looks in committed method lists
   DLLLOCAL const QoreMethod *parseFindCommittedMethod(const char *name);
   //DLLLOCAL const QoreMethod *parseFindCommittedStaticMethod(const char *name);

   // looks in committed and pending method lists
   DLLLOCAL const QoreMethod *parseFindMethodTree(const char *name);
   DLLLOCAL const QoreMethod *parseFindStaticMethodTree(const char *name);

   DLLLOCAL const QoreMethod *findCommittedMethod(const char *name, bool &priv_flag) const;
   DLLLOCAL const QoreMethod *findCommittedStaticMethod(const char *name, bool &priv_flag) const;

   DLLLOCAL bool match(const QoreClass *cls);
   DLLLOCAL void execConstructors(QoreObject *o, BCEAList *bceal, ExceptionSink *xsink) const;
   DLLLOCAL bool execDeleteBlockers(QoreObject *o, ExceptionSink *xsink) const;
   DLLLOCAL bool parseCheckHierarchy(const QoreClass *cls) const;
   DLLLOCAL bool isPrivateMember(const char *str) const;
   DLLLOCAL const QoreClass *parseFindPublicPrivateMember(const char *mem, const QoreTypeInfo *&typeInfo, bool &priv) const;
   DLLLOCAL bool parseHasPublicMembersInHierarchy() const;
   DLLLOCAL bool isPublicOrPrivateMember(const char *mem, bool &priv) const;
   DLLLOCAL void ref() const;
   DLLLOCAL void deref();
   DLLLOCAL const QoreClass *getClass(qore_classid_t cid, bool &priv) const {
      for (bclist_t::const_iterator i = begin(), e = end(); i != e; ++i) {
	 const QoreClass *qc = (*i)->getClass(cid, priv);
	 if (qc)
	    return qc;
      }
	 
      return 0;
   }
/*
   DLLLOCAL int initMembers(QoreObject *o, ExceptionSink *xsink) const {
      for (bclist_t::const_iterator i = begin(), e = end(); i != e; ++i) {
	 if ((*i)->sclass->initMembers(o, xsink))
	    return -1;
      }
      
      return 0;
   }
*/
};

// BCEANode
// base constructor evaluated argument node; created locally at run time
class BCEANode {
public:
   QoreListNode *args;
   const AbstractQoreFunctionVariant *variant;
   bool execed;
      
   DLLLOCAL BCEANode(QoreListNode *n_args, const AbstractQoreFunctionVariant *n_variant) : args(n_args), variant(n_variant), execed(false) {}
   DLLLOCAL BCEANode() : args(0), variant(0), execed(true) {}
};

struct ltqc {
   bool operator()(const class QoreClass *qc1, const class QoreClass *qc2) const {
      return qc1 < qc2;
   }
};

typedef std::map<const QoreClass *, class BCEANode *, ltqc> bceamap_t;

/*
  BCEAList
  base constructor evaluated argument list
*/
class BCEAList : public bceamap_t {
protected:
   DLLLOCAL ~BCEAList() { }
   
public:
   DLLLOCAL void deref(ExceptionSink *xsink);
   // evaluates arguments, returns -1 if an exception was thrown
   DLLLOCAL int add(const QoreClass *qc, const QoreListNode *arg, const AbstractQoreFunctionVariant *variant, ExceptionSink *xsink);
   DLLLOCAL QoreListNode *findArgs(const QoreClass *qc, bool *aexeced, const AbstractQoreFunctionVariant *&variant);
};

#endif
