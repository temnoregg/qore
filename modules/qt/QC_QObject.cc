/*
 QC_QObject.cc
 
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

#include <qore/Qore.h>

#include "QC_QObject.h"
#include "QC_QFont.h"

int CID_QOBJECT;

static void QOBJECT_constructor(class Object *self, class QoreNode *params, ExceptionSink *xsink)
{
   QoreQObject *qo;
   int np = num_params(params);
   if (!np)
      qo = new QoreQObject(self);
   else 
   {
      QoreNode *p = test_param(params, NT_OBJECT, 0);
      QoreAbstractQObject *parent = p ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
      if (!parent)
      {
	 if (!xsink->isException())
	    xsink->raiseException("QOBJECT-CONSTRUCTOR-ERROR", "expecting an object derived from QObject as parameter to QObject::constructor() in first argument if passed, argument is either not an argument or not derived from QObject (type passed: %s)", p ? p->type->getName() : "NOTHING");
	 return;
      }
      ReferenceHolder<QoreAbstractQObject> holder(parent, xsink);
      qo = new QoreQObject(self, parent->getQObject());
   }

   self->setPrivate(CID_QOBJECT, qo);
}

static void QOBJECT_copy(class Object *self, class Object *old, class QoreQObject *qo, ExceptionSink *xsink)
{
   xsink->raiseException("QOBJECT-COPY-ERROR", "objects of this class cannot be copied");
}

//bool blockSignals ( bool block )
static QoreNode *QOBJECT_blockSignals(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   bool block = p ? p->getAsBool() : 0;
   return new QoreNode(qo->getQObject()->blockSignals(block));
}

//const QObjectList & children () const
//static QoreNode *QOBJECT_children(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)qo->getQObject()->children());
//}

//bool connect ( const QObject * sender, const char * signal, const char * method, Qt::ConnectionType type = Qt::AutoCompatConnection ) const
static QoreNode *QOBJECT_connect(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);

   QoreAbstractQObject *sender = (p && p->type == NT_OBJECT) ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
   if (!p || !sender)
   {
      if (!xsink->isException())
         xsink->raiseException("QOBJECT-CONNECT-PARAM-ERROR", "expecting a QObject object as first argument to QObject::connect()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQObject> holder(sender, xsink);
   p = get_param(params, 1);
   if (!p || p->type != NT_STRING) {
      xsink->raiseException("QOBJECT-CONNECT-PARAM-ERROR", "expecting a string as second argument to QObject::connect()");
      return 0;
   }
   const char *signal = p->val.String->getBuffer();
   p = get_param(params, 2);
   if (!p || p->type != NT_STRING) {
      xsink->raiseException("QOBJECT-CONNECT-PARAM-ERROR", "expecting a string as third argument to QObject::connect()");
      return 0;
   }
   const char *method = p->val.String->getBuffer();
   //p = get_param(params, 3);
   //Qt::ConnectionType type = (Qt::ConnectionType)(p ? p->getAsInt() : 0);
   //return new QoreNode(qo->getQObject()->connect(sender->getQObject(), signal, method, type));

   qo->connectDynamic(sender, signal, method, xsink);
   return 0;
}

//bool disconnect ( const char * signal = 0, const QObject * receiver = 0, const char * method = 0 )
//bool disconnect ( const QObject * receiver, const char * method = 0 )
static QoreNode *QOBJECT_disconnect(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   int offset = 0;

   QoreNode *p = get_param(params, 0);
   if (!p || (p->type != NT_STRING && p->type != NT_OBJECT)) {
      xsink->raiseException("QOBJECT-DISCONNECT-PARAM-ERROR", "expecting a string or QObject as first argument to QObject::disconnect()");
      return 0;
   }

   const char *signal = 0;
   if (p && p->type == NT_STRING) {
      signal = p->val.String->getBuffer();
      offset = 1;
   }

   p = get_param(params, offset++);
   QoreAbstractQObject *receiver = (p && p->type == NT_OBJECT) ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
   if (!p || !receiver)
   {
      if (!xsink->isException())
         xsink->raiseException("QOBJECT-DISCONNECT-PARAM-ERROR", "expecting a QObject object argument to QObject::disconnect()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQObject> holder(receiver, xsink);
   p = get_param(params, offset);
   if (!p || p->type != NT_STRING) {
      xsink->raiseException("QOBJECT-DISCONNECT-PARAM-ERROR", "expecting a string as third argument to QObject::disconnect()");
      return 0;
   }
   const char *method = p->val.String->getBuffer();
   if (signal)
      return new QoreNode(qo->getQObject()->disconnect(signal, receiver->getQObject(), method));
   return new QoreNode(qo->getQObject()->disconnect(receiver->getQObject(), method));
}

//void dumpObjectInfo ()
static QoreNode *QOBJECT_dumpObjectInfo(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   qo->getQObject()->dumpObjectInfo();
   return 0;
}

//void dumpObjectTree ()
static QoreNode *QOBJECT_dumpObjectTree(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   qo->getQObject()->dumpObjectTree();
   return 0;
}

//QList<QByteArray> dynamicPropertyNames () const
//static QoreNode *QOBJECT_dynamicPropertyNames(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)qo->getQObject()->dynamicPropertyNames());
//}

//virtual bool event ( QEvent * e )
//static QoreNode *QOBJECT_event(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QEvent* e = p;
//   return new QoreNode(qo->getQObject()->event(e));
//}

//virtual bool eventFilter ( QObject * watched, QEvent * event )
//static QoreNode *QOBJECT_eventFilter(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   QoreAbstractQObject *watched = (p && p->type == NT_OBJECT) ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
//   if (!p || !watched)
//   {
//      if (!xsink->isException())
//         xsink->raiseException("QOBJECT-EVENTFILTER-PARAM-ERROR", "expecting a QObject object as first argument to QObject::eventFilter()");
//      return 0;
//   }
//   ReferenceHolder<QoreAbstractQObject> holder(watched, xsink);
//   p = get_param(params, 1);
//   ??? QEvent* event = p;
//   return new QoreNode(qo->getQObject()->eventFilter(watched->getQObject(), event));
//}

//T findChild ( const QString & name = QString() ) const
//static QoreNode *QOBJECT_findChild(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   if (!p || p->type != NT_STRING) {
//      xsink->raiseException("QOBJECT-FINDCHILD-PARAM-ERROR", "expecting a string as first argument to QObject::findChild()");
//      return 0;
//   }
//   const char *name = p->val.String->getBuffer();
//   ??? return new QoreNode((int64)qo->getQObject()->findChild(name));
//}

//QList<T> findChildren ( const QString & name = QString() ) const
//static QoreNode *QOBJECT_findChildren(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   if (!p || p->type != NT_STRING) {
//      xsink->raiseException("QOBJECT-FINDCHILDREN-PARAM-ERROR", "expecting a string as first argument to QObject::findChildren()");
//      return 0;
//   }
//   const char *name = p->val.String->getBuffer();
//   ??? return new QoreNode((int64)qo->getQObject()->findChildren(name));
//}
//QList<T> findChildren ( const QRegExp & regExp ) const
//static QoreNode *QOBJECT_findChildren(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QRegExp& regExp = p;
//   ??? return new QoreNode((int64)qo->getQObject()->findChildren(regExp));
//}

//bool inherits ( const char * className ) const
static QoreNode *QOBJECT_inherits(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (!p || p->type != NT_STRING) {
      xsink->raiseException("QOBJECT-INHERITS-PARAM-ERROR", "expecting a string as first argument to QObject::inherits()");
      return 0;
   }
   const char *className = p->val.String->getBuffer();
   return new QoreNode(qo->getQObject()->inherits(className));
}

//void installEventFilter ( QObject * filterObj )
static QoreNode *QOBJECT_installEventFilter(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQObject *filterObj = (p && p->type == NT_OBJECT) ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
   if (!p || !filterObj)
   {
      if (!xsink->isException())
         xsink->raiseException("QOBJECT-INSTALLEVENTFILTER-PARAM-ERROR", "expecting a QObject object as first argument to QObject::installEventFilter()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQObject> holder(filterObj, xsink);
   qo->getQObject()->installEventFilter(filterObj->getQObject());
   return 0;
}

//bool isWidgetType () const
static QoreNode *QOBJECT_isWidgetType(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode(qo->getQObject()->isWidgetType());
}

//void killTimer ( int id )
static QoreNode *QOBJECT_killTimer(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int id = p ? p->getAsInt() : 0;
   qo->getQObject()->killTimer(id);
   return 0;
}

//virtual const QMetaObject * metaObject () const
//static QoreNode *QOBJECT_metaObject(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return qo->getQObject()->metaObject();
//}

//void moveToThread ( QThread * targetThread )
//static QoreNode *QOBJECT_moveToThread(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QThread* targetThread = p;
//   qo->getQObject()->moveToThread(targetThread);
//   return 0;
//}

//QString objectName () const
static QoreNode *QOBJECT_objectName(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode(new QoreString(qo->getQObject()->objectName().toUtf8().data(), QCS_UTF8));
}

//QObject * parent () const
//static QoreNode *QOBJECT_parent(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return qo->getQObject()->parent();
//}

//QVariant property ( const char * name ) const
//static QoreNode *QOBJECT_property(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   if (!p || p->type != NT_STRING) {
//      xsink->raiseException("QOBJECT-PROPERTY-PARAM-ERROR", "expecting a string as first argument to QObject::property()");
//      return 0;
//   }
//   const char *name = p->val.String->getBuffer();
//   ??? return new QoreNode((int64)qo->getQObject()->property(name));
//}

//void removeEventFilter ( QObject * obj )
static QoreNode *QOBJECT_removeEventFilter(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQObject *obj = (p && p->type == NT_OBJECT) ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
   if (!p || !obj)
   {
      if (!xsink->isException())
         xsink->raiseException("QOBJECT-REMOVEEVENTFILTER-PARAM-ERROR", "expecting a QObject object as first argument to QObject::removeEventFilter()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQObject> holder(obj, xsink);
   qo->getQObject()->removeEventFilter(obj->getQObject());
   return 0;
}

//void setObjectName ( const QString & name )
static QoreNode *QOBJECT_setObjectName(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (!p || p->type != NT_STRING) {
      xsink->raiseException("QOBJECT-SETOBJECTNAME-PARAM-ERROR", "expecting a string as first argument to QObject::setObjectName()");
      return 0;
   }
   const char *name = p->val.String->getBuffer();
   qo->getQObject()->setObjectName(name);
   return 0;
}

//void setParent ( QObject * parent )
static QoreNode *QOBJECT_setParent(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQObject *parent = (p && p->type == NT_OBJECT) ? (QoreAbstractQObject *)p->val.object->getReferencedPrivateData(CID_QOBJECT, xsink) : 0;
   if (!p || !parent)
   {
      if (!xsink->isException())
         xsink->raiseException("QOBJECT-SETPARENT-PARAM-ERROR", "expecting a QObject object as first argument to QObject::setParent()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQObject> holder(parent, xsink);
   qo->getQObject()->setParent(parent->getQObject());
   return 0;
}

//bool setProperty ( const char * name, const QVariant & value )
//static QoreNode *QOBJECT_setProperty(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   if (!p || p->type != NT_STRING) {
//      xsink->raiseException("QOBJECT-SETPROPERTY-PARAM-ERROR", "expecting a string as first argument to QObject::setProperty()");
//      return 0;
//   }
//   const char *name = p->val.String->getBuffer();
//   p = get_param(params, 1);
//   ??? QVariant& value = p;
//   return new QoreNode(qo->getQObject()->setProperty(name, value));
//}

//bool signalsBlocked () const
static QoreNode *QOBJECT_signalsBlocked(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode(qo->getQObject()->signalsBlocked());
}

//int startTimer ( int interval )
static QoreNode *QOBJECT_startTimer(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int interval = p ? p->getAsInt() : 0;
   return new QoreNode((int64)qo->getQObject()->startTimer(interval));
}

//QThread * thread () const
//static QoreNode *QOBJECT_thread(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return qo->getQObject()->thread();
//}


// custom methods
static QoreNode *QOBJECT_createSignal(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = test_param(params, NT_STRING, 0);
   if (!p) {
      xsink->raiseException("QOBJECT-CREATE-SIGNAL-ERROR", "no string argument passed to QObject::createSignal()");
      return 0;
   }

   qo->createSignal(p->val.String->getBuffer(), xsink);
   return 0;
}

static QoreNode *QOBJECT_emit(Object *self, QoreAbstractQObject *qo, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = test_param(params, NT_STRING, 0);
   if (!p) {
      xsink->raiseException("QOBJECT-EMIT-ERROR", "no string argument passed to QObject::emit()");
      return 0;
   }

   qo->emit_signal(p->val.String->getBuffer(), params->val.list);
   return 0;
}


class QoreClass *initQObjectClass()
{
   tracein("initQObjectClass()");
   
   class QoreClass *QC_QObject = new QoreClass("QObject", QDOM_GUI);
   CID_QOBJECT = QC_QObject->getID();

   QC_QObject->setConstructor(QOBJECT_constructor);
   QC_QObject->setCopy((q_copy_t)QOBJECT_copy);
  
   QC_QObject->addMethod("blockSignals",                (q_method_t)QOBJECT_blockSignals);
   //QC_QObject->addMethod("children",                    (q_method_t)QOBJECT_children);
   QC_QObject->addMethod("connect",                     (q_method_t)QOBJECT_connect);
   QC_QObject->addMethod("disconnect",                  (q_method_t)QOBJECT_disconnect);
   QC_QObject->addMethod("dumpObjectInfo",              (q_method_t)QOBJECT_dumpObjectInfo);
   QC_QObject->addMethod("dumpObjectTree",              (q_method_t)QOBJECT_dumpObjectTree);
   //QC_QObject->addMethod("dynamicPropertyNames",        (q_method_t)QOBJECT_dynamicPropertyNames);
   //QC_QObject->addMethod("event",                       (q_method_t)QOBJECT_event);
   //QC_QObject->addMethod("eventFilter",                 (q_method_t)QOBJECT_eventFilter);
   //QC_QObject->addMethod("findChild",                   (q_method_t)QOBJECT_findChild);
   //QC_QObject->addMethod("findChildren",                (q_method_t)QOBJECT_findChildren);
   QC_QObject->addMethod("inherits",                    (q_method_t)QOBJECT_inherits);
   QC_QObject->addMethod("installEventFilter",          (q_method_t)QOBJECT_installEventFilter);
   QC_QObject->addMethod("isWidgetType",                (q_method_t)QOBJECT_isWidgetType);
   QC_QObject->addMethod("killTimer",                   (q_method_t)QOBJECT_killTimer);
   //QC_QObject->addMethod("metaObject",                  (q_method_t)QOBJECT_metaObject);
   //QC_QObject->addMethod("moveToThread",                (q_method_t)QOBJECT_moveToThread);
   QC_QObject->addMethod("objectName",                  (q_method_t)QOBJECT_objectName);
   //QC_QObject->addMethod("parent",                      (q_method_t)QOBJECT_parent);
   //QC_QObject->addMethod("property",                    (q_method_t)QOBJECT_property);
   QC_QObject->addMethod("removeEventFilter",           (q_method_t)QOBJECT_removeEventFilter);
   QC_QObject->addMethod("setObjectName",               (q_method_t)QOBJECT_setObjectName);
   QC_QObject->addMethod("setParent",                   (q_method_t)QOBJECT_setParent);
   //QC_QObject->addMethod("setProperty",                 (q_method_t)QOBJECT_setProperty);
   QC_QObject->addMethod("signalsBlocked",              (q_method_t)QOBJECT_signalsBlocked);
   QC_QObject->addMethod("startTimer",                  (q_method_t)QOBJECT_startTimer);
   //QC_QObject->addMethod("thread",                      (q_method_t)QOBJECT_thread);

   // custom methods
   QC_QObject->addMethod("createSignal",                (q_method_t)QOBJECT_createSignal);
   QC_QObject->addMethod("emit",                        (q_method_t)QOBJECT_emit);


   traceout("initQObjectClass()");
   return QC_QObject;
}
