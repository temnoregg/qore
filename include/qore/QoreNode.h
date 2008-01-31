/*
  QoreNode.h
  
  Qore Programming Language

  Copyright (C) 2003, 2004, 2005, 2006, 2007 David Nichols

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

#ifndef _QORE_NODE_H

#define _QORE_NODE_H

#include <qore/common.h>
#include <qore/ReferenceObject.h>
#include <qore/node_types.h>

#include <string>

#include <assert.h>

#define FMT_NONE   -1
#define FMT_NORMAL 0

union node_u {
      // for function reference calls
      class FunctionReferenceCall *funcrefcall;
};

class QoreNode : public ReferenceObject
{
   private:
      // not implemented
      DLLLOCAL QoreNode(const QoreNode&);
      DLLLOCAL QoreNode& operator=(const QoreNode&);

      DLLEXPORT QoreNode(class QoreString *str);
      DLLEXPORT QoreNode(class DateTime *dt);
      DLLEXPORT QoreNode(const char *str);
      DLLEXPORT QoreNode(const std::string &str);
      DLLEXPORT QoreNode(class QoreHash *h);
      DLLEXPORT QoreNode();
      DLLEXPORT QoreNode(class QoreListNode *l);
      DLLEXPORT QoreNode(class QoreObject *o);
      DLLEXPORT QoreNode(class BinaryNode *b);
      DLLEXPORT QoreNode(int64 v);
      DLLEXPORT QoreNode(long v);
      DLLEXPORT QoreNode(const QoreType *t, int64 v);
      DLLEXPORT QoreNode(bool v);
      DLLEXPORT QoreNode(double d);
      DLLLOCAL QoreNode(class QoreNode *l, class Operator *o, class QoreNode *r);
      DLLLOCAL QoreNode(class QoreTreeNode *t);
      DLLLOCAL QoreNode(class VarRef *v);
      DLLLOCAL QoreNode(char *fn, class QoreListNode *n_args);
      DLLLOCAL QoreNode(class QoreListNode *n_args, char *fn);
      DLLLOCAL QoreNode(class QoreListNode *n_args, class NamedScope *n);
      DLLLOCAL QoreNode(class UserFunction *u, class QoreListNode *n_args);
      DLLLOCAL QoreNode(class BuiltinFunction *b, class QoreListNode *n_args);
      DLLLOCAL QoreNode(class FunctionCall *fc);
      DLLLOCAL QoreNode(class NamedScope *n, class QoreListNode *n_args);
      DLLLOCAL QoreNode(class NamedScope *n);
      DLLLOCAL QoreNode(class ComplexContextRef *ccref);
      DLLLOCAL QoreNode(class LVRef *lvref);
      DLLLOCAL QoreNode(class RegexSubst *rs);
      DLLLOCAL QoreNode(class RegexSubstNode *rs);
      DLLLOCAL QoreNode(class QoreRegex *r);
      DLLLOCAL QoreNode(class QoreRegexNode *r);
      DLLLOCAL QoreNode(class RegexTrans *rt);
      DLLLOCAL QoreNode(class ClassRef *c);
      DLLLOCAL QoreNode(class ClassRefNode *c);
      DLLLOCAL QoreNode(class AbstractParseObjectMethodReference *objmethref);
      DLLLOCAL QoreNode(class AbstractParseObjectMethodReferenceNode *objmethref);
      DLLLOCAL QoreNode(class AbstractFunctionReference *afr);
      DLLLOCAL QoreNode(class AbstractFunctionReferenceNode *afr);

   protected:
      DLLEXPORT virtual ~QoreNode();

   public:
      union node_u val;
      const QoreType *type;

      DLLEXPORT QoreNode(const QoreType *t);

      // get the value of the type in a string context (default implementation = del = false and returns NullString)
      // if del is true, then the returned QoreString * should be deleted, if false, then it must not be
      // use the QoreStringValueHelper class (defined in QoreStringNode.h) instead of using this function directly
      DLLEXPORT virtual QoreString *getStringRepresentation(bool &del) const;
      // concatenate string representation to a QoreString (no action for complex types = default implementation)
      DLLEXPORT virtual void getStringRepresentation(QoreString &str) const;

      // if del is true, then the returned DateTime * should be deleted, if false, then it should not
      DLLEXPORT virtual class DateTime *getDateTimeRepresentation(bool &del) const;
      // assign date representation to a DateTime (no action for complex types = default implementation)
      DLLEXPORT virtual void getDateTimeRepresentation(DateTime &dt) const;

      DLLEXPORT virtual bool getAsBool() const;
      DLLEXPORT virtual int getAsInt() const;
      DLLEXPORT virtual int64 getAsBigInt() const;
      DLLEXPORT virtual double getAsFloat() const;

      // get string representation (for %n and %N), foff is for multi-line formatting offset, -1 = no line breaks
      // the ExceptionSink is only needed for QoreObject where a method may be executed
      // use the QoreNodeAsStringHelper class (defined in QoreStringNode.h) instead of using these functions directly
      // returns -1 for exception raised, 0 = OK
      DLLEXPORT virtual int getAsString(QoreString &str, int foff, class ExceptionSink *xsink) const;
      // if del is true, then the returned QoreString * should be deleted, if false, then it must not be
      DLLEXPORT virtual QoreString *getAsString(bool &del, int foff, class ExceptionSink *xsink) const;

      // default implementation returns false
      DLLEXPORT virtual bool needs_eval() const;
      DLLEXPORT virtual class QoreNode *realCopy() const;

      // performs a lexical compare, return -1, 0, or 1 if the "this" value is less than, equal, or greater than
      // the "val" passed
      //DLLLOCAL virtual int compare(const QoreNode *val) const;
      // the type passed must always be equal to the current type
      DLLEXPORT virtual bool is_equal_soft(const QoreNode *v, ExceptionSink *xsink) const;
      DLLEXPORT virtual bool is_equal_hard(const QoreNode *v, ExceptionSink *xsink) const;

      // returns the data type
      DLLEXPORT virtual const QoreType *getType() const;
      // returns the type name as a c string
      DLLEXPORT virtual const char *getTypeName() const;
      // eval(): return value requires a deref(xsink)
      // default implementation = returns "this" with incremented atomic reference count
      DLLEXPORT virtual class QoreNode *eval(class ExceptionSink *xsink) const;
      // eval(): return value requires a deref(xsink) if needs_deref is true
      // default implementation = needs_deref = false, returns "this"
      // note: do not use this function directly, use the QoreNodeEvalOptionalRefHolder class instead
      DLLEXPORT virtual class QoreNode *eval(bool &needs_deref, class ExceptionSink *xsink) const;
      // default implementation is getAsBigInt()
      DLLEXPORT virtual int64 bigIntEval(class ExceptionSink *xsink) const;
      // default implementation is getAsInt()
      DLLEXPORT virtual int integerEval(class ExceptionSink *xsink) const;
      // default implementation is getAsBool()
      DLLEXPORT virtual bool boolEval(class ExceptionSink *xsink) const;
      // default implementation is getAsFloat()
      DLLEXPORT virtual double floatEval(class ExceptionSink *xsink) const;

      // deletes the object when the reference count = 0
      DLLEXPORT virtual void deref(class ExceptionSink *xsink);
      // returns true if the node represents a value (default implementation)
      DLLEXPORT virtual bool is_value() const;
      
      DLLLOCAL QoreNode(class FunctionReferenceCall *frc);
      
      DLLEXPORT class QoreNode *RefSelf() const;
      DLLEXPORT void ref() const;
};

class SimpleQoreNode : public QoreNode 
{
   private:
      // not implemented
      DLLLOCAL SimpleQoreNode& operator=(const SimpleQoreNode&);

   public:
      DLLEXPORT SimpleQoreNode(const QoreType *t);
      DLLEXPORT SimpleQoreNode(const SimpleQoreNode &) : QoreNode(type)
      {
      }
      DLLEXPORT void deref();
};

class ParseNode : public SimpleQoreNode
{
   private:
      // not implemented
      DLLLOCAL ParseNode& operator=(const ParseNode&);

   public:
      DLLLOCAL ParseNode(const QoreType *t) : SimpleQoreNode(t)
      {
      }
      DLLLOCAL ParseNode(const ParseNode &) : SimpleQoreNode(type)
      {
      }
      // parse types should never be copied
      DLLLOCAL virtual class QoreNode *realCopy() const
      {
	 assert(false);
	 return 0;
      }
      DLLLOCAL virtual bool is_equal_soft(const QoreNode *v, ExceptionSink *xsink) const
      {
	 assert(false);
	 return false;
      }
      DLLLOCAL virtual bool is_equal_hard(const QoreNode *v, ExceptionSink *xsink) const
      {
	 assert(false);
	 return false;
      }
      DLLLOCAL virtual bool needs_eval() const
      {
	 return true;
      }
      DLLLOCAL virtual bool is_value() const
      {
	 return false;
      }
      DLLLOCAL virtual QoreNode *eval(class ExceptionSink *xsink) const = 0;
      DLLLOCAL virtual QoreNode *eval(bool &needs_deref, class ExceptionSink *xsink) const
      {
	 ReferenceHolder<QoreNode> rv(eval(xsink), xsink);
	 if (!rv || *xsink) {
	    needs_deref = 0;
	    return 0;
	 }
	 needs_deref = true;
	 return rv.release();
      }
      DLLLOCAL virtual int64 bigIntEval(class ExceptionSink *xsink) const
      {
	 ReferenceHolder<QoreNode> rv(eval(xsink), xsink);
	 return rv ? rv->getAsBigInt() : 0;
      }
      DLLLOCAL virtual int integerEval(class ExceptionSink *xsink) const
      {
	 ReferenceHolder<QoreNode> rv(eval(xsink), xsink);
	 return rv ? rv->getAsInt() : 0;
      }
      DLLLOCAL virtual bool boolEval(class ExceptionSink *xsink) const
      {
	 ReferenceHolder<QoreNode> rv(eval(xsink), xsink);
	 return rv ? rv->getAsBool() : 0;
      }
      DLLLOCAL virtual double floatEval(class ExceptionSink *xsink) const
      {
	 ReferenceHolder<QoreNode> rv(eval(xsink), xsink);
	 return rv ? rv->getAsFloat() : 0;
      }
};

// these objects will never be copied or referenced therefore they can have 
// public destructors - the deref() functions just call "delete this;"
class ParseNoEvalNode : public ParseNode
{
   private:
      // not implemented
      DLLLOCAL ParseNoEvalNode& operator=(const ParseNoEvalNode&);

   public:
      DLLLOCAL ParseNoEvalNode(const QoreType *t) : ParseNode(t)
      {
      }
      DLLLOCAL ParseNoEvalNode(const ParseNode &) : ParseNode(type)
      {
      }
      DLLLOCAL virtual bool needs_eval() const
      {
	 return false;
      }
      DLLLOCAL virtual QoreNode *eval(class ExceptionSink *xsink) const
      {
	 assert(false);
	 return 0;
      }
      DLLLOCAL virtual QoreNode *eval(bool &needs_deref, class ExceptionSink *xsink) const
      {
	 assert(false);
	 return 0;
      }
      DLLLOCAL virtual int64 bigIntEval(class ExceptionSink *xsink) const
      {
	 assert(false);
	 return 0;
      }
      DLLLOCAL virtual int integerEval(class ExceptionSink *xsink) const
      {
	 assert(false);
	 return 0;
      }
      DLLLOCAL virtual bool boolEval(class ExceptionSink *xsink) const
      {
	 assert(false);
	 return false;
      }
      DLLLOCAL virtual double floatEval(class ExceptionSink *xsink) const
      {
	 assert(false);
	 return 0.0;
      }
      DLLLOCAL virtual void deref()
      {
	 assert(is_unique());
	 delete this;
      }
      DLLLOCAL virtual void deref(class ExceptionSink *xsink)
      {
	 assert(is_unique());
	 delete this;
      }
};

// for getting relative time values or integer values
DLLEXPORT int getSecZeroInt(const QoreNode *a);
DLLEXPORT int64 getSecZeroBigInt(const QoreNode *a);
DLLEXPORT int getSecMinusOneInt(const QoreNode *a);
DLLEXPORT int64 getSecMinusOneBigInt(const QoreNode *a);
DLLEXPORT int getMsZeroInt(const QoreNode *a);
DLLEXPORT int64 getMsZeroBigInt(const QoreNode *a);
DLLEXPORT int getMsMinusOneInt(const QoreNode *a);
DLLEXPORT int64 getMsMinusOneBigInt(const QoreNode *a);
DLLEXPORT int getMicroSecZeroInt(const QoreNode *a);
DLLEXPORT bool is_nothing(const class QoreNode *n);

DLLLOCAL class QoreNode *copy_and_resolve_lvar_refs(const QoreNode *n, ExceptionSink *xsink);

static inline void discard(QoreNode *n, ExceptionSink *xsink)
{
   if (n)
      n->deref(xsink);
}

class QoreNodeEvalOptionalRefHolder {
   private:
      QoreNode *val;
      ExceptionSink *xsink;
      bool needs_deref;

      DLLLOCAL void discard_intern()
      {
	 if (needs_deref && val)
	    val->deref(xsink);
      }

      // not implemented
      DLLLOCAL QoreNodeEvalOptionalRefHolder(const QoreNodeEvalOptionalRefHolder&);
      DLLLOCAL QoreNodeEvalOptionalRefHolder& operator=(const QoreNodeEvalOptionalRefHolder&);
      DLLLOCAL void *operator new(size_t);

   public:
      DLLLOCAL QoreNodeEvalOptionalRefHolder(ExceptionSink *n_xsink) : xsink(n_xsink)
      {
	 needs_deref = false;
	 val = 0;
      }
      DLLLOCAL QoreNodeEvalOptionalRefHolder(const QoreNode *exp, ExceptionSink *n_xsink) : xsink(n_xsink)
      {
	 needs_deref = false;
	 val = exp ? exp->eval(needs_deref, xsink) : 0;
      }
      DLLLOCAL ~QoreNodeEvalOptionalRefHolder()
      {
	 discard_intern();
      }
      DLLLOCAL void discard()
      {
	 discard_intern();
	 needs_deref = false;
	 val = 0;
      }
      DLLLOCAL void assign(bool n_needs_deref, QoreNode *n_val)
      {
	 discard_intern();
	 needs_deref = n_needs_deref;
	 val = n_val;
      }
      // returns a referenced value - the caller will own the reference
      DLLLOCAL QoreNode *getReferencedValue()
      {
	 if (needs_deref)
	    needs_deref = false;
	 else if (val)
	    val->ref();
	 return val;
      }
      // takes the referenced value and leaves this object empty, value is referenced if necessary
      DLLLOCAL QoreNode *takeReferencedValue()
      {
	 QoreNode *rv = val;
	 if (val && !needs_deref)
	    rv->ref();
	 val = 0;
	 needs_deref = false;
	 return rv;
      }
      DLLLOCAL QoreNode *operator->() { return val; }
      DLLLOCAL QoreNode *operator*() { return val; }
      DLLLOCAL operator bool() const { return val != 0; }
};

#endif
