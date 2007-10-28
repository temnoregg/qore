/*
 QC_QLayout.cc
 
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

#include "QC_QLayout.h"

int CID_QLAYOUT;
QoreClass *QC_QLayout = 0;

static void QLAYOUT_constructor(class Object *self, class QoreNode *params, ExceptionSink *xsink)
{
   xsink->raiseException("ABSTRACT-CLASS-ERROR", "QLayout is an abstract builtin class and cannot be directly instantiated or referenced by user code");
}

//bool activate ()
static QoreNode *QLAYOUT_activate(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode(ql->getQLayout()->activate());
}

////virtual void addItem ( QLayoutItem * item ) = 0
//static QoreNode *QLAYOUT_addItem(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QLayoutItem* item = p;
//   ql->getQLayout()->addItem(item);
//   return 0;
//}

//void addWidget ( QWidget * w )
static QoreNode *QLAYOUT_addWidget(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQWidget *w = (p && p->type == NT_OBJECT) ? (QoreAbstractQWidget *)p->val.object->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
   if (!w) {
      if (!xsink->isException())
         xsink->raiseException("QLAYOUT-ADDWIDGET-PARAM-ERROR", "expecting a QWidget object as first argument to QLayout::addWidget()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQWidget> wHolder(w, xsink);
   ql->getQLayout()->addWidget(w->getQWidget());
   return 0;
}

//QRect contentsRect () const
static QoreNode *QLAYOUT_contentsRect(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   Object *o_qr = new Object(QC_QRect, getProgram());
   QoreQRect *q_qr = new QoreQRect(ql->getQLayout()->contentsRect());
   o_qr->setPrivate(CID_QRECT, q_qr);
   return new QoreNode(o_qr);
}

//virtual int count () const = 0
static QoreNode *QLAYOUT_count(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)ql->getQLayout()->count());
}

//virtual Qt::Orientations expandingDirections () const
static QoreNode *QLAYOUT_expandingDirections(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)ql->getQLayout()->expandingDirections());
}

////void getContentsMargins ( int * left, int * top, int * right, int * bottom ) const
//static QoreNode *QLAYOUT_getContentsMargins(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? int* left = p;
//   p = get_param(params, 1);
//   ??? int* top = p;
//   p = get_param(params, 2);
//   ??? int* right = p;
//   p = get_param(params, 3);
//   ??? int* bottom = p;
//   ql->getQLayout()->getContentsMargins(left, top, right, bottom);
//   return 0;
//}

//virtual int indexOf ( QWidget * widget ) const
static QoreNode *QLAYOUT_indexOf(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQWidget *widget = (p && p->type == NT_OBJECT) ? (QoreAbstractQWidget *)p->val.object->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
   if (!widget) {
      if (!xsink->isException())
         xsink->raiseException("QLAYOUT-INDEXOF-PARAM-ERROR", "expecting a QWidget object as first argument to QLayout::indexOf()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQWidget> widgetHolder(widget, xsink);
   return new QoreNode((int64)ql->getQLayout()->indexOf(widget->getQWidget()));
}

//bool isEnabled () const
static QoreNode *QLAYOUT_isEnabled(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode(ql->getQLayout()->isEnabled());
}

////virtual QLayoutItem * itemAt ( int index ) const = 0
//static QoreNode *QLAYOUT_itemAt(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   int index = p ? p->getAsInt() : 0;
//   ??? return ql->getQLayout()->itemAt(index);
//}

//virtual QSize maximumSize () const
static QoreNode *QLAYOUT_maximumSize(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   Object *o_qs = new Object(QC_QSize, getProgram());
   QoreQSize *q_qs = new QoreQSize(ql->getQLayout()->maximumSize());
   o_qs->setPrivate(CID_QSIZE, q_qs);
   return new QoreNode(o_qs);
}

////QWidget * menuBar () const
//static QoreNode *QLAYOUT_menuBar(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return ql->getQLayout()->menuBar();
//}

//virtual QSize minimumSize () const
static QoreNode *QLAYOUT_minimumSize(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   Object *o_qs = new Object(QC_QSize, getProgram());
   QoreQSize *q_qs = new QoreQSize(ql->getQLayout()->minimumSize());
   o_qs->setPrivate(CID_QSIZE, q_qs);
   return new QoreNode(o_qs);
}

////QWidget * parentWidget () const
//static QoreNode *QLAYOUT_parentWidget(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   ??? return ql->getQLayout()->parentWidget();
//}

////void removeItem ( QLayoutItem * item )
//static QoreNode *QLAYOUT_removeItem(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QLayoutItem* item = p;
//   ql->getQLayout()->removeItem(item);
//   return 0;
//}

//void removeWidget ( QWidget * widget )
static QoreNode *QLAYOUT_removeWidget(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQWidget *widget = (p && p->type == NT_OBJECT) ? (QoreAbstractQWidget *)p->val.object->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
   if (!widget) {
      if (!xsink->isException())
         xsink->raiseException("QLAYOUT-REMOVEWIDGET-PARAM-ERROR", "expecting a QWidget object as first argument to QLayout::removeWidget()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQWidget> widgetHolder(widget, xsink);
   ql->getQLayout()->removeWidget(widget->getQWidget());
   return 0;
}

//bool setAlignment ( QWidget * w, Qt::Alignment alignment )
//bool setAlignment ( QLayout * l, Qt::Alignment alignment )
static QoreNode *QLAYOUT_setAlignment(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = test_param(params, NT_OBJECT, 0);
   QoreAbstractQLayout *l = p ? (QoreAbstractQLayout *)p->val.object->getReferencedPrivateData(CID_QLAYOUT, xsink) : 0;
   if (!l) {
      QoreAbstractQWidget *w = p ? (QoreAbstractQWidget *)p->val.object->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
      if (!w) {
	 if (!xsink->isException())
	    xsink->raiseException("QLAYOUT-SETALIGNMENT-PARAM-ERROR", "QLayout::setAlignment() expecting a QWidget or QLayout as first argument");
	 return 0;
      }
      ReferenceHolder<QoreAbstractQWidget> wHolder(w, xsink);
      p = get_param(params, 1);
      Qt::Alignment alignment = (Qt::Alignment)(p ? p->getAsInt() : 0);
      return new QoreNode(ql->getQLayout()->setAlignment(w->getQWidget(), alignment));
   }
   ReferenceHolder<QoreAbstractQLayout> lHolder(l, xsink);
   p = get_param(params, 1);
   Qt::Alignment alignment = (Qt::Alignment)(p ? p->getAsInt() : 0);
   return new QoreNode(ql->getQLayout()->setAlignment(l->getQLayout(), alignment));
}

//void setContentsMargins ( int left, int top, int right, int bottom )
static QoreNode *QLAYOUT_setContentsMargins(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int left = p ? p->getAsInt() : 0;
   p = get_param(params, 1);
   int top = p ? p->getAsInt() : 0;
   p = get_param(params, 2);
   int right = p ? p->getAsInt() : 0;
   p = get_param(params, 3);
   int bottom = p ? p->getAsInt() : 0;
   ql->getQLayout()->setContentsMargins(left, top, right, bottom);
   return 0;
}

//void setEnabled ( bool enable )
static QoreNode *QLAYOUT_setEnabled(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   bool enable = p ? p->getAsBool() : false;
   ql->getQLayout()->setEnabled(enable);
   return 0;
}

//void setMenuBar ( QWidget * widget )
static QoreNode *QLAYOUT_setMenuBar(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreAbstractQWidget *widget = (p && p->type == NT_OBJECT) ? (QoreAbstractQWidget *)p->val.object->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
   if (!widget) {
      if (!xsink->isException())
         xsink->raiseException("QLAYOUT-SETMENUBAR-PARAM-ERROR", "expecting a QWidget object as first argument to QLayout::setMenuBar()");
      return 0;
   }
   ReferenceHolder<QoreAbstractQWidget> widgetHolder(widget, xsink);
   ql->getQLayout()->setMenuBar(widget->getQWidget());
   return 0;
}

//void setSizeConstraint ( SizeConstraint )
static QoreNode *QLAYOUT_setSizeConstraint(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QLayout::SizeConstraint sizeconstraint = (QLayout::SizeConstraint)(p ? p->getAsInt() : 0);
   ql->getQLayout()->setSizeConstraint(sizeconstraint);
   return 0;
}

//void setSpacing ( int )
static QoreNode *QLAYOUT_setSpacing(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int x = p ? p->getAsInt() : 0;
   ql->getQLayout()->setSpacing(x);
   return 0;
}

//SizeConstraint sizeConstraint () const
static QoreNode *QLAYOUT_sizeConstraint(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)ql->getQLayout()->sizeConstraint());
}

//int spacing () const
static QoreNode *QLAYOUT_spacing(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)ql->getQLayout()->spacing());
}

////virtual QLayoutItem * takeAt ( int index ) = 0
//static QoreNode *QLAYOUT_takeAt(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   int index = p ? p->getAsInt() : 0;
//   ??? return ql->getQLayout()->takeAt(index);
//}

//void update ()
static QoreNode *QLAYOUT_update(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   ql->getQLayout()->update();
   return 0;
}

//void setMargin (int);
static QoreNode *QLAYOUT_setMargin(Object *self, QoreAbstractQLayout *ql, QoreNode *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int margin = p ? p->getAsInt() : 0;
   ql->getQLayout()->setMargin(margin);

   return 0;
}

class QoreClass *initQLayoutClass(class QoreClass *qobject)
{
   tracein("initQLayoutClass()");
   
   QC_QLayout = new QoreClass("QLayout", QDOM_GUI);
   CID_QLAYOUT = QC_QLayout->getID();

   QC_QLayout->addBuiltinVirtualBaseClass(qobject);
   QC_QLayout->setConstructor(QLAYOUT_constructor);

   QC_QLayout->addMethod("activate",                    (q_method_t)QLAYOUT_activate);
   //QC_QLayout->addMethod("addItem",                     (q_method_t)QLAYOUT_addItem);
   QC_QLayout->addMethod("addWidget",                   (q_method_t)QLAYOUT_addWidget);
   QC_QLayout->addMethod("contentsRect",                (q_method_t)QLAYOUT_contentsRect);
   QC_QLayout->addMethod("count",                       (q_method_t)QLAYOUT_count);
   QC_QLayout->addMethod("expandingDirections",         (q_method_t)QLAYOUT_expandingDirections);
   //QC_QLayout->addMethod("getContentsMargins",          (q_method_t)QLAYOUT_getContentsMargins);
   QC_QLayout->addMethod("indexOf",                     (q_method_t)QLAYOUT_indexOf);
   QC_QLayout->addMethod("isEnabled",                   (q_method_t)QLAYOUT_isEnabled);
   //QC_QLayout->addMethod("itemAt",                      (q_method_t)QLAYOUT_itemAt);
   QC_QLayout->addMethod("maximumSize",                 (q_method_t)QLAYOUT_maximumSize);
   //QC_QLayout->addMethod("menuBar",                     (q_method_t)QLAYOUT_menuBar);
   QC_QLayout->addMethod("minimumSize",                 (q_method_t)QLAYOUT_minimumSize);
   //QC_QLayout->addMethod("parentWidget",                (q_method_t)QLAYOUT_parentWidget);
   //QC_QLayout->addMethod("removeItem",                  (q_method_t)QLAYOUT_removeItem);
   QC_QLayout->addMethod("removeWidget",                (q_method_t)QLAYOUT_removeWidget);
   QC_QLayout->addMethod("setAlignment",                (q_method_t)QLAYOUT_setAlignment);
   QC_QLayout->addMethod("setContentsMargins",          (q_method_t)QLAYOUT_setContentsMargins);
   QC_QLayout->addMethod("setEnabled",                  (q_method_t)QLAYOUT_setEnabled);
   QC_QLayout->addMethod("setMargin",                   (q_method_t)QLAYOUT_setMargin);
   QC_QLayout->addMethod("setMenuBar",                  (q_method_t)QLAYOUT_setMenuBar);
   QC_QLayout->addMethod("setSizeConstraint",           (q_method_t)QLAYOUT_setSizeConstraint);
   QC_QLayout->addMethod("setSpacing",                  (q_method_t)QLAYOUT_setSpacing);
   QC_QLayout->addMethod("sizeConstraint",              (q_method_t)QLAYOUT_sizeConstraint);
   QC_QLayout->addMethod("spacing",                     (q_method_t)QLAYOUT_spacing);
   //QC_QLayout->addMethod("takeAt",                      (q_method_t)QLAYOUT_takeAt);
   QC_QLayout->addMethod("update",                      (q_method_t)QLAYOUT_update);


   traceout("initQLayoutClass()");
   return QC_QLayout;
}
