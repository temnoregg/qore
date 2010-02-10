/*
 QoreTypeInfo.cc
 
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

#include <qore/Qore.h>

// static reference types
static QoreTypeInfo staticBigIntTypeInfo(NT_INT), 
   staticFloatTypeInfo(NT_FLOAT),
   staticBoolTypeInfo(NT_BOOLEAN),
   staticStringTypeInfo(NT_STRING),
   staticBinaryTypeInfo(NT_BINARY),
   staticObjectTypeInfo(NT_OBJECT),
   staticDateTypeInfo(NT_DATE),
   staticHashTypeInfo(NT_HASH),
   staticListTypeInfo(NT_LIST),
   staticNothingTypeInfo(NT_NOTHING),
   staticNullTypeInfo(NT_NULL),
   staticRunTimeClosureTypeInfo(NT_RUNTIME_CLOSURE),
   staticCallReferenceTypeInfo(NT_FUNCREF)
   ;

// const pointers to static reference types
const QoreTypeInfo *bigIntTypeInfo = &staticBigIntTypeInfo,
   *floatTypeInfo = &staticFloatTypeInfo,
   *boolTypeInfo = &staticBoolTypeInfo,
   *stringTypeInfo = &staticStringTypeInfo,
   *binaryTypeInfo = &staticBinaryTypeInfo,
   *objectTypeInfo = &staticObjectTypeInfo,
   *dateTypeInfo = &staticDateTypeInfo,
   *hashTypeInfo = &staticHashTypeInfo,
   *listTypeInfo = &staticListTypeInfo,
   *nothingTypeInfo = &staticNothingTypeInfo,
   *nullTypeInfo = &staticNullTypeInfo,
   *runTimeClosureTypeInfo = &staticRunTimeClosureTypeInfo,
   *callReferenceTypeInfo = &staticCallReferenceTypeInfo
   ;

const QoreTypeInfo *getTypeInfoForValue(const AbstractQoreNode *n) {
   qore_type_t t = n ? n->getType() : NT_NOTHING;
   if (t != NT_OBJECT) {
      switch (t) {
	 case NT_INT:
	    return bigIntTypeInfo;
	 case NT_STRING:
	    return stringTypeInfo;
	 case NT_BOOLEAN:
	    return boolTypeInfo;
	 case NT_FLOAT:
	    return floatTypeInfo;
	 case NT_BINARY:
	    return binaryTypeInfo;
	 case NT_LIST:
	    return listTypeInfo;
	 case NT_HASH:
	    return hashTypeInfo;
	 case NT_DATE:
	    return dateTypeInfo;
	 case NT_NULL:
	    return nullTypeInfo;
	 case NT_NOTHING:
	    return nothingTypeInfo;
	 default:
	    //printd(5, "getTypeInfoForValue() %s %d not found\n", n->getTypeName(), n->getType());
	    return 0;
      }
   }
   return reinterpret_cast<const QoreObject *>(n)->getClass()->getTypeInfo();
}

AbstractQoreNode *getDefaultValueForBuiltinValueType(qore_type_t t) {
   switch (t) {
      case NT_INT:
	 return new QoreBigIntNode(0);
      case NT_STRING:
	 return new QoreStringNode;
      case NT_BOOLEAN:
	 return &False;
      case NT_DATE:
	 return new DateTimeNode((int64)0);
      case NT_FLOAT:
	 return new QoreFloatNode(0.0);
      case NT_LIST:
	 return new QoreListNode;
      case NT_HASH:
	 return new QoreHashNode;
      case NT_BINARY:
	 return new BinaryNode;
      case NT_NULL:
	 return &Null;
      case NT_NOTHING:
      case NT_OBJECT:
	 return &Nothing;
   }

   assert(false);
   return 0;
}

qore_type_t getBuiltinType(const char *str) {
   if (!strcmp(str, "int"))
      return NT_INT;
   if (!strcmp(str, "string"))
      return NT_STRING;
   if (!strcmp(str, "bool"))
      return NT_BOOLEAN;
   if (!strcmp(str, "date"))
      return NT_DATE;
   if (!strcmp(str, "float"))
      return NT_FLOAT;
   if (!strcmp(str, "list"))
      return NT_LIST;
   if (!strcmp(str, "hash"))
      return NT_HASH;
   if (!strcmp(str, "object"))
      return NT_OBJECT;
   if (!strcmp(str, "binary"))
      return NT_BINARY;
   // these last two don't make much sense to use, but...
   if (!strcmp(str, "null"))
      return NT_NULL;
   if (!strcmp(str, "nothing"))
      return NT_NOTHING;

   return -1;
}

const char *getBuiltinTypeName(qore_type_t type) {
   switch (type) {
      case NT_INT:
	 return "int";

      case NT_STRING:
	 return "string";

      case NT_BOOLEAN:
	 return "bool";

      case NT_DATE:
	 return "date";

      case NT_FLOAT:
	 return "float";

      case NT_LIST:
	 return "list";

      case NT_HASH:
	 return "hash";

      case NT_OBJECT:
	 return "object";

      case NT_BINARY:
	 return "binary";

	 // these last two don't make much sense to use, but...
      case NT_NULL:
	 return "null";

      case NT_NOTHING:
	 return "nothing";
   }

   assert(false);
   return "<unknown>";
}

void QoreParseTypeInfo::resolve() {
   if (cscope) {
      // resolve class
      qc = getRootNS()->parseFindScopedClass(cscope);
      delete cscope;
      cscope = 0;
   }
}

AbstractQoreNode *QoreTypeInfo::checkTypeInstantiationIntern(bool obj, const char *param_name, AbstractQoreNode *n, ExceptionSink *xsink) const {
   //printd(0, "QoreTypeInfo::checkTypeInstantiationIntern() this=%p has_type=%d (%s) n=%p (%s)\n", this, this ? has_type : 0, getName(), n, n ? n->getTypeName() : "NOTHING");
   if (!this || !has_type) return n;
   if (qt == NT_NOTHING && is_nothing(n)) return n;
   if (is_nothing(n)) {
      if (obj) doObjectTypeException(param_name, n, xsink);
      else doTypeException(param_name, n, xsink);
      return n;
   }
      
   // from here on we know n != 0
   if (qt == NT_OBJECT) {
      if (n->getType() != NT_OBJECT) {
	 if (obj) doObjectTypeException(param_name, n, xsink);
	 else doTypeException(param_name, n, xsink);
	 return n;
      }

      if (!qc)
	 return n;

      bool priv;
      if (reinterpret_cast<const QoreObject *>(n)->getClass(qc->getID(), priv)) {
	 if (!priv)
	    return n;

	 // check private access
	 if (!runtimeCheckPrivateClassAccess(qc))
	    return n;

	 if (obj) doObjectPrivateClassException(param_name, n, xsink);
	 else doPrivateClassException(param_name, n, xsink);
	 return n;
      }

      if (obj) doObjectTypeException(param_name, n, xsink);
      else doTypeException(param_name, n, xsink);
   }
   qore_type_t t = n->getType();

   if (t == qt)
      return n;

   // exceptions here
   if (qt == NT_FLOAT && t == NT_INT) {
      QoreFloatNode *rv = new QoreFloatNode(n->getAsFloat());
      n->deref(xsink);
      return rv;
   }

   if (obj) doObjectTypeException(param_name, n, xsink);
   else doTypeException(param_name, n, xsink);

   return n;
}

int QoreTypeInfo::testTypeCompatibility(const AbstractQoreNode *n) const {
   if (!this || !has_type) return QTI_IDENT;
   if (qt == NT_NOTHING && is_nothing(n)) return QTI_IDENT;
   if (is_nothing(n))
      return QTI_NOT_EQUAL;

   qore_type_t t = n->getType();

   // from here on we know n != 0
   if (qt == NT_OBJECT) {
      if (t != qt)
	 return QTI_NOT_EQUAL;
      if (!qc)
	 return QTI_AMBIGUOUS;

      bool priv;
      if (reinterpret_cast<const QoreObject *>(n)->getClass(qc->getID(), priv)) {
	 if (!priv)
	    return QTI_IDENT;

	 // check private access
	 if (!runtimeCheckPrivateClassAccess(qc))
	    return QTI_IDENT;
      }

      return QTI_NOT_EQUAL;
   }

   // exceptions here
   if (qt == NT_FLOAT && t == NT_INT)
      return QTI_AMBIGUOUS;

   if (t == qt)
      return QTI_IDENT;

   return QTI_NOT_EQUAL;
}

int QoreTypeInfo::parseEqual(const QoreTypeInfo *typeInfo) const {
   if (!this || !has_type || !typeInfo || !typeInfo->has_type)
      return QTI_IDENT;

   if (qt == NT_OBJECT) {
      if (typeInfo->qt != NT_OBJECT)
	 return QTI_NOT_EQUAL;

      if (!qc)
	 return QTI_AMBIGUOUS;

      return parseCheckCompatibleClass(qc, typeInfo->qc) ? QTI_IDENT : QTI_NOT_EQUAL;
   }

   // exceptions here
   if (qt == NT_FLOAT && typeInfo->qt == NT_INT)
      return QTI_AMBIGUOUS;

   if (typeInfo->qt == qt)
      return QTI_IDENT;

   return QTI_NOT_EQUAL;
}
