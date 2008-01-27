/*
 QC_QApplication.cc
 
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
#include "QC_QApplication.h"
#include "QC_QObject.h"
#include "QC_QClipboard.h"
#include "QC_QStyle.h"
#include "QC_QWidget.h"
#include "QC_QDesktopWidget.h"
#include "QC_QFont.h"
#include "QC_QLocale.h"

DLLLOCAL int CID_QAPPLICATION;

DLLLOCAL int static_argc    = 0;
DLLLOCAL char **static_argv = 0;

static LockedObject qapp_lock;
static QoreObject *qore_qapp = 0;

void qapp_dec()
{
   AutoLocker al(&qapp_lock);
   qore_qapp = 0;
}

class QoreNode *get_qore_qapp()
{
   AutoLocker al(&qapp_lock);
   if (!qore_qapp)
      return 0;

   qore_qapp->ref();
   return qore_qapp;
}

static void QA_constructor(class QoreObject *self, const QoreList *params, ExceptionSink *xsink)
{
   AutoLocker al(&qapp_lock);
   if (qore_qapp) {
      xsink->raiseException("QAPPLICATION-ERROR", "only one QApplication can exist at one time");
      return;
   }

   QoreQApplication *qa = new QoreQApplication(self);
   self->setPrivate(CID_QAPPLICATION, qa);
   qore_qapp = self;
}

static void QA_copy(class QoreObject *self, class QoreObject *old, class QoreQApplication *qa, ExceptionSink *xsink)
{
   xsink->raiseException("QAPPLICATION-COPY-ERROR", "objects of this class cannot be copied");
}

static class QoreNode *QA_exec(class QoreObject *self, class QoreQApplication *qa, const QoreList *params, ExceptionSink *xsink)
{
   qa->qobj->exec();
   return 0;
}

class QoreClass *initQApplicationClass(class QoreClass *qcoreapplication)
{
   tracein("initQApplicationClass()");
   
   class QoreClass *QC_QApplication = new QoreClass("QApplication", QDOM_GUI);
   CID_QAPPLICATION = QC_QApplication->getID();

   QC_QApplication->addBuiltinVirtualBaseClass(qcoreapplication);

   QC_QApplication->setConstructor(QA_constructor);
   QC_QApplication->setCopy((q_copy_t)QA_copy);

   QC_QApplication->addMethod("exec",    (q_method_t)QA_exec);

   traceout("initQApplicationClass()");
   return QC_QApplication;
}

//QWidget * activeModalWidget ()
static QoreNode *f_QApplication_activeModalWidget(const QoreList *params, ExceptionSink *xsink)
{
   QWidget *qt_qobj = QApplication::activeModalWidget();
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   assert(rv_obj);
   rv_obj->ref();
   return rv_obj;
}

//QWidget * activePopupWidget ()
static QoreNode *f_QApplication_activePopupWidget(const QoreList *params, ExceptionSink *xsink)
{
   QWidget *qt_qobj = QApplication::activePopupWidget();
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   assert(rv_obj);
   rv_obj->ref();
   return rv_obj;
}

//QWidget * activeWindow ()
static QoreNode *f_QApplication_activeWindow(const QoreList *params, ExceptionSink *xsink)
{
   QWidget *qt_qobj = QApplication::activeWindow();
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   assert(rv_obj);
   rv_obj->ref();
   return rv_obj;
}

//void alert ( QWidget * widget, int msec = 0 )
static QoreNode *f_QApplication_alert(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreQWidget *widget = (p && p->type == NT_OBJECT) ? (QoreQWidget *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
   if (!widget) {
      if (!xsink->isException())
         xsink->raiseException("QAPPLICATION-ALERT-PARAM-ERROR", "expecting a QWidget object as first argument to QApplication::alert()");
      return 0;
   }
   ReferenceHolder<AbstractPrivateData> widgetHolder(static_cast<AbstractPrivateData *>(widget), xsink);
   p = get_param(params, 1);
   int msec = !is_nothing(p) ? p->getAsInt() : 0;
   QApplication::alert(static_cast<QWidget *>(widget->getQWidget()), msec);
   return 0;
}

////QWidgetQoreList allWidgets ()
//static QoreNode *f_QApplication_allWidgets(const QoreList *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)QApplication::allWidgets());
//}

//void beep ()
static QoreNode *f_QApplication_beep(const QoreList *params, ExceptionSink *xsink)
{
   QApplication::beep();
   return 0;
}

////void changeOverrideCursor ( const QCursor & cursor )
//static QoreNode *f_QApplication_changeOverrideCursor(const QoreList *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QCursor cursor = p;
//   QApplication::changeOverrideCursor(cursor);
//   return 0;
//}

static QoreNode *f_QApplication_clipboard(const QoreList *params, class ExceptionSink *xsink)
{
   static LockedObject lClipboard;

   AutoLocker al(&lClipboard);

   if (!C_Clipboard) {
      QoreObject *o = new QoreObject(QC_QClipboard, getProgram());
      QoreQClipboard *qcb = new QoreQClipboard(o, QApplication::clipboard());
      o->setPrivate(CID_QCLIPBOARD, qcb);
      C_Clipboard = o;
   }
   return C_Clipboard->RefSelf();
}

//int colorSpec ()
static QoreNode *f_QApplication_colorSpec(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::colorSpec());
}

//int cursorFlashTime ()
static QoreNode *f_QApplication_cursorFlashTime(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::cursorFlashTime());
}

//QDesktopWidget * desktop ()
static QoreNode *f_QApplication_desktop(const QoreList *params, ExceptionSink *xsink)
{
   QDesktopWidget *qt_qobj = QApplication::desktop();
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   if (rv_obj)
      rv_obj->ref();
   else {
      rv_obj = new QoreObject(QC_QDesktopWidget, getProgram());
      QoreQtQDesktopWidget *t_qobj = new QoreQtQDesktopWidget(rv_obj, qt_qobj);
      rv_obj->setPrivate(CID_QDESKTOPWIDGET, t_qobj);
   }
   return rv_obj;
}

//bool desktopSettingsAware ()
static QoreNode *f_QApplication_desktopSettingsAware(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode(QApplication::desktopSettingsAware());
}

//int doubleClickInterval ()
static QoreNode *f_QApplication_doubleClickInterval(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::doubleClickInterval());
}

//int exec ()
static QoreNode *f_QApplication_exec(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::exec());
}

//QWidget * focusWidget ()
static QoreNode *f_QApplication_focusWidget(const QoreList *params, ExceptionSink *xsink)
{
   QWidget *qt_qobj = QApplication::focusWidget();
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   assert(rv_obj);
   rv_obj->ref();
   return rv_obj;
}

//QFont font ()
//QFont font ( const QWidget * widget )
//QFont font ( const char * className )
static QoreNode *f_QApplication_font(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (is_nothing(p)) {
      QoreObject *o_qf = new QoreObject(QC_QFont, getProgram());
      QoreQFont *q_qf = new QoreQFont(QApplication::font());
      o_qf->setPrivate(CID_QFONT, q_qf);
      return o_qf;
   }
   if (p->type == NT_OBJECT) {
      QoreQWidget *widget = (QoreQWidget *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QWIDGET, xsink);
      if (!widget) {
         if (!xsink->isException())
            xsink->raiseException("QAPPLICATION-FONT-PARAM-ERROR", "QApplication::font() does not know how to handle arguments of class '%s' as passed as the first argument", (reinterpret_cast<QoreObject *>(p))->getClass()->getName());
         return 0;
      }
      ReferenceHolder<AbstractPrivateData> widgetHolder(static_cast<AbstractPrivateData *>(widget), xsink);
      QoreObject *o_qf = new QoreObject(QC_QFont, getProgram());
      QoreQFont *q_qf = new QoreQFont(QApplication::font(static_cast<QWidget *>(widget->getQWidget())));
      o_qf->setPrivate(CID_QFONT, q_qf);
      return o_qf;
   }
   QoreStringNode *pstr = dynamic_cast<QoreStringNode *>(p);
   if (!pstr) {
      xsink->raiseException("QAPPLICATION-FONT-PARAM-ERROR", "QApplication::font() expects either a string, a QWidget as the sole argument or no arguments at all");
      return 0;
   }
   const char *className = pstr->getBuffer();
   QoreObject *o_qf = new QoreObject(QC_QFont, getProgram());
   QoreQFont *q_qf = new QoreQFont(QApplication::font(className));
   o_qf->setPrivate(CID_QFONT, q_qf);
   return o_qf;
}

//QFontMetrics fontMetrics ()
static QoreNode *f_QApplication_fontMetrics(const QoreList *params, ExceptionSink *xsink)
{
   QoreObject *o_qfm = new QoreObject(QC_QFontMetrics, getProgram());
   QoreQFontMetrics *q_qfm = new QoreQFontMetrics(QApplication::fontMetrics());
   o_qfm->setPrivate(CID_QFONTMETRICS, q_qfm);
   return o_qfm;
}

//QSize globalStrut ()
static QoreNode *f_QApplication_globalStrut(const QoreList *params, ExceptionSink *xsink)
{
   QoreObject *o_qs = new QoreObject(QC_QSize, getProgram());
   QoreQSize *q_qs = new QoreQSize(QApplication::globalStrut());
   o_qs->setPrivate(CID_QSIZE, q_qs);
   return o_qs;
}

//bool isEffectEnabled ( Qt::UIEffect effect )
static QoreNode *f_QApplication_isEffectEnabled(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   Qt::UIEffect effect = (Qt::UIEffect)(p ? p->getAsInt() : 0);
   return new QoreNode(QApplication::isEffectEnabled(effect));
}

//bool isLeftToRight ()
static QoreNode *f_QApplication_isLeftToRight(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode(QApplication::isLeftToRight());
}

//bool isRightToLeft ()
static QoreNode *f_QApplication_isRightToLeft(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode(QApplication::isRightToLeft());
}

//Qt::LayoutDirection keyboardInputDirection ()
static QoreNode *f_QApplication_keyboardInputDirection(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::keyboardInputDirection());
}

//int keyboardInputInterval ()
static QoreNode *f_QApplication_keyboardInputInterval(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::keyboardInputInterval());
}

//QLocale keyboardInputLocale ()
static QoreNode *f_QApplication_keyboardInputLocale(const QoreList *params, ExceptionSink *xsink)
{
   QoreObject *o_ql = new QoreObject(QC_QLocale, getProgram());
   QoreQLocale *q_ql = new QoreQLocale(QApplication::keyboardInputLocale());
   o_ql->setPrivate(CID_QLOCALE, q_ql);
   return o_ql;
}

//Qt::KeyboardModifiers keyboardModifiers ()
static QoreNode *f_QApplication_keyboardModifiers(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::keyboardModifiers());
}

#ifdef QT_KEYPAD_NAVIGATION
//bool keypadNavigationEnabled ()
static QoreNode *f_QApplication_keypadNavigationEnabled(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode(QApplication::keypadNavigationEnabled());
}
#endif

//Qt::LayoutDirection layoutDirection ()
static QoreNode *f_QApplication_layoutDirection(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::layoutDirection());
}

//Qt::MouseButtons mouseButtons ()
static QoreNode *f_QApplication_mouseButtons(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::mouseButtons());
}

////QCursor * overrideCursor ()
//static QoreNode *f_QApplication_overrideCursor(const QoreList *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)QApplication::overrideCursor());
//}

//QPalette palette ()
//QPalette palette ( const QWidget * widget )
//QPalette palette ( const char * className )
static QoreNode *f_QApplication_palette(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (is_nothing(p)) {
      QoreObject *o_qp = new QoreObject(QC_QPalette, getProgram());
      QoreQPalette *q_qp = new QoreQPalette(QApplication::palette());
      o_qp->setPrivate(CID_QPALETTE, q_qp);
      return o_qp;
   }
   if (p && p->type == NT_OBJECT) {
      QoreQWidget *widget = (QoreQWidget *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QWIDGET, xsink);
      if (!widget) {
         if (!xsink->isException())
            xsink->raiseException("QAPPLICATION-PALETTE-PARAM-ERROR", "QApplication::palette() does not know how to handle arguments of class '%s' as passed as the first argument", (reinterpret_cast<QoreObject *>(p))->getClass()->getName());
         return 0;
      }
      ReferenceHolder<AbstractPrivateData> widgetHolder(static_cast<AbstractPrivateData *>(widget), xsink);
      QoreObject *o_qp = new QoreObject(QC_QPalette, getProgram());
      QoreQPalette *q_qp = new QoreQPalette(QApplication::palette(static_cast<QWidget *>(widget->getQWidget())));
      o_qp->setPrivate(CID_QPALETTE, q_qp);
      return o_qp;
   }
   QoreStringNode *pstr = dynamic_cast<QoreStringNode *>(p);
   if (!pstr) {
      xsink->raiseException("QAPPLICATION-PALETTE-PARAM-ERROR", "expecting a string as first argument to QApplication::palette()");
      return 0;
   }
   const char *className = pstr->getBuffer();
   QoreObject *o_qp = new QoreObject(QC_QPalette, getProgram());
   QoreQPalette *q_qp = new QoreQPalette(QApplication::palette(className));
   o_qp->setPrivate(CID_QPALETTE, q_qp);
   return o_qp;
}

//bool quitOnLastWindowClosed ()
static QoreNode *f_QApplication_quitOnLastWindowClosed(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode(QApplication::quitOnLastWindowClosed());
}

////QDecoration & qwsDecoration ()
//static QoreNode *f_QApplication_qwsDecoration(const QoreList *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)QApplication::qwsDecoration());
//}

////void qwsSetDecoration ( QDecoration * decoration )
////QDecoration * qwsSetDecoration ( const QString & decoration )
//static QoreNode *f_QApplication_qwsSetDecoration(const QoreList *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QDecoration* decoration = p;
//   QApplication::qwsSetDecoration(decoration);
//   return 0;
//}

//void restoreOverrideCursor ()
static QoreNode *f_QApplication_restoreOverrideCursor(const QoreList *params, ExceptionSink *xsink)
{
   QApplication::restoreOverrideCursor();
   return 0;
}

//void setActiveWindow ( QWidget * active )
static QoreNode *f_QApplication_setActiveWindow(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreQWidget *active = (p && p->type == NT_OBJECT) ? (QoreQWidget *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QWIDGET, xsink) : 0;
   if (!active) {
      if (!xsink->isException())
         xsink->raiseException("QAPPLICATION-SETACTIVEWINDOW-PARAM-ERROR", "expecting a QWidget object as first argument to QApplication::setActiveWindow()");
      return 0;
   }
   ReferenceHolder<AbstractPrivateData> activeHolder(static_cast<AbstractPrivateData *>(active), xsink);
   QApplication::setActiveWindow(static_cast<QWidget *>(active->getQWidget()));
   return 0;
}

//void setColorSpec ( int spec )
static QoreNode *f_QApplication_setColorSpec(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int spec = p ? p->getAsInt() : 0;
   QApplication::setColorSpec(spec);
   return 0;
}

//void setCursorFlashTime ( int )
static QoreNode *f_QApplication_setCursorFlashTime(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int x = p ? p->getAsInt() : 0;
   QApplication::setCursorFlashTime(x);
   return 0;
}

//void setDesktopSettingsAware ( bool on )
static QoreNode *f_QApplication_setDesktopSettingsAware(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   bool on = p ? p->getAsBool() : false;
   QApplication::setDesktopSettingsAware(on);
   return 0;
}

//void setDoubleClickInterval ( int )
static QoreNode *f_QApplication_setDoubleClickInterval(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int x = p ? p->getAsInt() : 0;
   QApplication::setDoubleClickInterval(x);
   return 0;
}

//void setEffectEnabled ( Qt::UIEffect effect, bool enable = true )
static QoreNode *f_QApplication_setEffectEnabled(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   Qt::UIEffect effect = (Qt::UIEffect)(p ? p->getAsInt() : 0);
   p = get_param(params, 1);
   bool enable = !is_nothing(p) ? p->getAsBool() : true;
   QApplication::setEffectEnabled(effect, enable);
   return 0;
}

//void setFont ( const QFont & font, const char * className = 0 )
static QoreNode *f_QApplication_setFont(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreQFont *font = (p && p->type == NT_OBJECT) ? (QoreQFont *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QFONT, xsink) : 0;
   if (!font) {
      if (!xsink->isException())
         xsink->raiseException("QAPPLICATION-SETFONT-PARAM-ERROR", "expecting a QFont object as first argument to QApplication::setFont()");
      return 0;
   }
   ReferenceHolder<AbstractPrivateData> fontHolder(static_cast<AbstractPrivateData *>(font), xsink);
   QoreStringNode *pstr = test_string_param(params, 1);
   const char *className = pstr ? pstr->getBuffer() : 0;
   QApplication::setFont(*(static_cast<QFont *>(font)), className);
   return 0;
}

////void setGlobalStrut ( const QSize & )
//static QoreNode *f_QApplication_setGlobalStrut(const QoreList *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? const^QSize const^qsize = p;
//   QApplication::setGlobalStrut(const^qsize);
//   return 0;
//}

//void setKeyboardInputInterval ( int )
static QoreNode *f_QApplication_setKeyboardInputInterval(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int x = p ? p->getAsInt() : 0;
   QApplication::setKeyboardInputInterval(x);
   return 0;
}

#ifdef QT_KEYPAD_NAVIGATION
//void setKeypadNavigationEnabled ( bool enable )
static QoreNode *f_QApplication_setKeypadNavigationEnabled(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   bool enable = p ? p->getAsBool() : false;
   QApplication::setKeypadNavigationEnabled(enable);
   return 0;
}
#endif

//void setLayoutDirection ( Qt::LayoutDirection direction )
static QoreNode *f_QApplication_setLayoutDirection(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   Qt::LayoutDirection direction = (Qt::LayoutDirection)(p ? p->getAsInt() : 0);
   QApplication::setLayoutDirection(direction);
   return 0;
}

////void setOverrideCursor ( const QCursor & cursor )
//static QoreNode *f_QApplication_setOverrideCursor(const QoreList *params, ExceptionSink *xsink)
//{
//   QoreNode *p = get_param(params, 0);
//   ??? QCursor cursor = p;
//   QApplication::setOverrideCursor(cursor);
//   return 0;
//}

//void setPalette ( const QPalette & palette, const char * className = 0 )
static QoreNode *f_QApplication_setPalette(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreQPalette *palette = (p && p->type == NT_OBJECT) ? (QoreQPalette *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QPALETTE, xsink) : 0;
   if (!palette) {
      if (!xsink->isException())
         xsink->raiseException("QAPPLICATION-SETPALETTE-PARAM-ERROR", "expecting a QPalette object as first argument to QApplication::setPalette()");
      return 0;
   }
   ReferenceHolder<AbstractPrivateData> paletteHolder(static_cast<AbstractPrivateData *>(palette), xsink);
   QoreStringNode *pstr = test_string_param(params, 1);
   const char *className = pstr ? pstr->getBuffer() : 0;
   QApplication::setPalette(*(palette->getQPalette()), className);
   return 0;
}

//void setQuitOnLastWindowClosed ( bool quit )
static QoreNode *f_QApplication_setQuitOnLastWindowClosed(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   bool quit = p ? p->getAsBool() : false;
   QApplication::setQuitOnLastWindowClosed(quit);
   return 0;
}

//void setStartDragDistance ( int l )
static QoreNode *f_QApplication_setStartDragDistance(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int l = p ? p->getAsInt() : 0;
   QApplication::setStartDragDistance(l);
   return 0;
}

//void setStartDragTime ( int ms )
static QoreNode *f_QApplication_setStartDragTime(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int ms = p ? p->getAsInt() : 0;
   QApplication::setStartDragTime(ms);
   return 0;
}

//void setStyle ( QStyle * style )
//QStyle * setStyle ( const QString & style )
static QoreNode *f_QApplication_setStyle(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (p && p->type == NT_OBJECT) {
      QoreAbstractQStyle *style = (QoreAbstractQStyle *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QSTYLE, xsink);
      if (!style) {
         if (!xsink->isException())
            xsink->raiseException("QAPPLICATION-SETSTYLE-PARAM-ERROR", "QApplication::setStyle() does not know how to handle arguments of class '%s' as passed as the first argument", (reinterpret_cast<QoreObject *>(p))->getClass()->getName());
         return 0;
      }
      ReferenceHolder<AbstractPrivateData> styleHolder(static_cast<AbstractPrivateData *>(style), xsink);
      QApplication::setStyle(style->getQStyle());
      return 0;
   }
   QString style;
   if (get_qstring(p, style, xsink))
      return 0;
   return return_qstyle(style, QApplication::setStyle(style), xsink);
}

//void setWheelScrollLines ( int )
static QoreNode *f_QApplication_setWheelScrollLines(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   int x = p ? p->getAsInt() : 0;
   QApplication::setWheelScrollLines(x);
   return 0;
}

//void setWindowIcon ( const QIcon & icon )
static QoreNode *f_QApplication_setWindowIcon(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   QoreQIcon *icon = (p && p->type == NT_OBJECT) ? (QoreQIcon *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QICON, xsink) : 0;
   if (!icon) {
      if (!xsink->isException())
         xsink->raiseException("QAPPLICATION-SETWINDOWICON-PARAM-ERROR", "expecting a QIcon object as first argument to QApplication::setWindowIcon()");
      return 0;
   }
   ReferenceHolder<AbstractPrivateData> iconHolder(static_cast<AbstractPrivateData *>(icon), xsink);
   QApplication::setWindowIcon(*(static_cast<QIcon *>(icon)));
   return 0;
}

//int startDragDistance ()
static QoreNode *f_QApplication_startDragDistance(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::startDragDistance());
}

//int startDragTime ()
static QoreNode *f_QApplication_startDragTime(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::startDragTime());
}

//QStyle * style ()
static QoreNode *f_QApplication_style(const QoreList *params, ExceptionSink *xsink)
{
   QStyle *qt_qobj = QApplication::style();
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   if (rv_obj)
      rv_obj->ref();
   else
   {
      rv_obj = new QoreObject(QC_QStyle, getProgram());
      QoreQtQStyle *qs = new QoreQtQStyle(rv_obj, qt_qobj);
      rv_obj->setPrivate(CID_QSTYLE, qs);
   }
   return rv_obj;
}

//void syncX ()
static QoreNode *f_QApplication_syncX(const QoreList *params, ExceptionSink *xsink)
{
   QApplication::syncX();
   return 0;
}

//QWidget * topLevelAt ( const QPoint & point )
//QWidget * topLevelAt ( int x, int y )
static QoreNode *f_QApplication_topLevelAt(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (p && p->type == NT_OBJECT) {
      QoreQPoint *point = (QoreQPoint *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QPOINT, xsink);
      if (!point) {
         if (!xsink->isException())
            xsink->raiseException("QAPPLICATION-TOPLEVELAT-PARAM-ERROR", "QApplication::topLevelAt() does not know how to handle arguments of class '%s' as passed as the first argument", (reinterpret_cast<QoreObject *>(p))->getClass()->getName());
         return 0;
      }
      ReferenceHolder<AbstractPrivateData> pointHolder(static_cast<AbstractPrivateData *>(point), xsink);
      QWidget *qt_qobj = QApplication::topLevelAt(*(static_cast<QPoint *>(point)));
      if (!qt_qobj)
         return 0;
      QVariant qv_ptr = qt_qobj->property("qobject");
      QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
      assert(rv_obj);
      rv_obj->ref();
      return rv_obj;
   }
   int x = p ? p->getAsInt() : 0;
   p = get_param(params, 1);
   int y = p ? p->getAsInt() : 0;
   QWidget *qt_qobj = QApplication::topLevelAt(x, y);
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   assert(rv_obj);
   rv_obj->ref();
   return rv_obj;
}

////QWidgetQoreList topLevelWidgets ()
//static QoreNode *f_QApplication_topLevelWidgets(const QoreList *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)QApplication::topLevelWidgets());
//}

////Type type ()
//static QoreNode *f_QApplication_type(const QoreList *params, ExceptionSink *xsink)
//{
//   ??? return new QoreNode((int64)QApplication::type());
//}

//int wheelScrollLines ()
static QoreNode *f_QApplication_wheelScrollLines(const QoreList *params, ExceptionSink *xsink)
{
   return new QoreNode((int64)QApplication::wheelScrollLines());
}

//QWidget * widgetAt ( const QPoint & point )
//QWidget * widgetAt ( int x, int y )
static QoreNode *f_QApplication_widgetAt(const QoreList *params, ExceptionSink *xsink)
{
   QoreNode *p = get_param(params, 0);
   if (p && p->type == NT_OBJECT) {
      QoreQPoint *point = (QoreQPoint *)(reinterpret_cast<QoreObject *>(p))->getReferencedPrivateData(CID_QPOINT, xsink);
      if (!point) {
         if (!xsink->isException())
            xsink->raiseException("QAPPLICATION-WIDGETAT-PARAM-ERROR", "QApplication::widgetAt() does not know how to handle arguments of class '%s' as passed as the first argument", (reinterpret_cast<QoreObject *>(p))->getClass()->getName());
         return 0;
      }
      ReferenceHolder<AbstractPrivateData> pointHolder(static_cast<AbstractPrivateData *>(point), xsink);
      QWidget *qt_qobj = QApplication::widgetAt(*(static_cast<QPoint *>(point)));
      if (!qt_qobj)
         return 0;
      QVariant qv_ptr = qt_qobj->property("qobject");
      QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
      assert(rv_obj);
      rv_obj->ref();
      return rv_obj;
   }
   int x = p ? p->getAsInt() : 0;
   p = get_param(params, 1);
   int y = p ? p->getAsInt() : 0;
   QWidget *qt_qobj = QApplication::widgetAt(x, y);
   if (!qt_qobj)
      return 0;
   QVariant qv_ptr = qt_qobj->property("qobject");
   QoreObject *rv_obj = reinterpret_cast<QoreObject *>(qv_ptr.toULongLong());
   assert(rv_obj);
   rv_obj->ref();
   return rv_obj;
}

//QIcon windowIcon ()
static QoreNode *f_QApplication_windowIcon(const QoreList *params, ExceptionSink *xsink)
{
   QoreObject *o_qi = new QoreObject(QC_QIcon, getProgram());
   QoreQIcon *q_qi = new QoreQIcon(QApplication::windowIcon());
   o_qi->setPrivate(CID_QICON, q_qi);
   return o_qi;
}



void initQApplicationStaticFunctions()
{
   // add builtin functions (static member functions in C++)
   builtinFunctions.add("QApplication_activeModalWidget",            f_QApplication_activeModalWidget);
   builtinFunctions.add("QApplication_activePopupWidget",            f_QApplication_activePopupWidget);
   builtinFunctions.add("QApplication_activeWindow",                 f_QApplication_activeWindow);
   builtinFunctions.add("QApplication_alert",                        f_QApplication_alert);
   //builtinFunctions.add("QApplication_allWidgets",                   f_QApplication_allWidgets);
   builtinFunctions.add("QApplication_beep",                         f_QApplication_beep);
   //builtinFunctions.add("QApplication_changeOverrideCursor",         f_QApplication_changeOverrideCursor);
   builtinFunctions.add("QApplication_clipboard",                    f_QApplication_clipboard);
   builtinFunctions.add("QApplication_colorSpec",                    f_QApplication_colorSpec);
   builtinFunctions.add("QApplication_cursorFlashTime",              f_QApplication_cursorFlashTime);
   builtinFunctions.add("QApplication_desktop",                      f_QApplication_desktop);
   builtinFunctions.add("QApplication_desktopSettingsAware",         f_QApplication_desktopSettingsAware);
   builtinFunctions.add("QApplication_doubleClickInterval",          f_QApplication_doubleClickInterval);
   builtinFunctions.add("QApplication_exec",                         f_QApplication_exec);
   builtinFunctions.add("QApplication_focusWidget",                  f_QApplication_focusWidget);
   builtinFunctions.add("QApplication_font",                         f_QApplication_font);
   builtinFunctions.add("QApplication_fontMetrics",                  f_QApplication_fontMetrics);
   builtinFunctions.add("QApplication_globalStrut",                  f_QApplication_globalStrut);
   builtinFunctions.add("QApplication_isEffectEnabled",              f_QApplication_isEffectEnabled);
   builtinFunctions.add("QApplication_isLeftToRight",                f_QApplication_isLeftToRight);
   builtinFunctions.add("QApplication_isRightToLeft",                f_QApplication_isRightToLeft);
   builtinFunctions.add("QApplication_keyboardInputDirection",       f_QApplication_keyboardInputDirection);
   builtinFunctions.add("QApplication_keyboardInputInterval",        f_QApplication_keyboardInputInterval);
   builtinFunctions.add("QApplication_keyboardInputLocale",          f_QApplication_keyboardInputLocale);
   builtinFunctions.add("QApplication_keyboardModifiers",            f_QApplication_keyboardModifiers);
#ifdef QT_KEYPAD_NAVIGATION
   builtinFunctions.add("QApplication_keypadNavigationEnabled",      f_QApplication_keypadNavigationEnabled);
#endif
   builtinFunctions.add("QApplication_layoutDirection",              f_QApplication_layoutDirection);
   builtinFunctions.add("QApplication_mouseButtons",                 f_QApplication_mouseButtons);
   //builtinFunctions.add("QApplication_overrideCursor",               f_QApplication_overrideCursor);
   builtinFunctions.add("QApplication_palette",                      f_QApplication_palette);
   builtinFunctions.add("QApplication_quitOnLastWindowClosed",       f_QApplication_quitOnLastWindowClosed);
   //builtinFunctions.add("QApplication_qwsDecoration",                f_QApplication_qwsDecoration);
   //builtinFunctions.add("QApplication_qwsSetDecoration",             f_QApplication_qwsSetDecoration);
   builtinFunctions.add("QApplication_restoreOverrideCursor",        f_QApplication_restoreOverrideCursor);
   builtinFunctions.add("QApplication_setActiveWindow",              f_QApplication_setActiveWindow);
   builtinFunctions.add("QApplication_setColorSpec",                 f_QApplication_setColorSpec);
   builtinFunctions.add("QApplication_setCursorFlashTime",           f_QApplication_setCursorFlashTime);
   builtinFunctions.add("QApplication_setDesktopSettingsAware",      f_QApplication_setDesktopSettingsAware);
   builtinFunctions.add("QApplication_setDoubleClickInterval",       f_QApplication_setDoubleClickInterval);
   builtinFunctions.add("QApplication_setEffectEnabled",             f_QApplication_setEffectEnabled);
   builtinFunctions.add("QApplication_setFont",                      f_QApplication_setFont);
   //builtinFunctions.add("QApplication_setGlobalStrut",               f_QApplication_setGlobalStrut);
   builtinFunctions.add("QApplication_setKeyboardInputInterval",     f_QApplication_setKeyboardInputInterval);
#ifdef QT_KEYPAD_NAVIGATION
   builtinFunctions.add("QApplication_setKeypadNavigationEnabled",   f_QApplication_setKeypadNavigationEnabled);
#endif
   builtinFunctions.add("QApplication_setLayoutDirection",           f_QApplication_setLayoutDirection);
   //builtinFunctions.add("QApplication_setOverrideCursor",            f_QApplication_setOverrideCursor);
   builtinFunctions.add("QApplication_setPalette",                   f_QApplication_setPalette);
   builtinFunctions.add("QApplication_setQuitOnLastWindowClosed",    f_QApplication_setQuitOnLastWindowClosed);
   builtinFunctions.add("QApplication_setStartDragDistance",         f_QApplication_setStartDragDistance);
   builtinFunctions.add("QApplication_setStartDragTime",             f_QApplication_setStartDragTime);
   builtinFunctions.add("QApplication_setStyle",                     f_QApplication_setStyle);
   builtinFunctions.add("QApplication_setWheelScrollLines",          f_QApplication_setWheelScrollLines);
   builtinFunctions.add("QApplication_setWindowIcon",                f_QApplication_setWindowIcon);
   builtinFunctions.add("QApplication_startDragDistance",            f_QApplication_startDragDistance);
   builtinFunctions.add("QApplication_startDragTime",                f_QApplication_startDragTime);
   builtinFunctions.add("QApplication_style",                        f_QApplication_style);
   builtinFunctions.add("QApplication_syncX",                        f_QApplication_syncX);
   builtinFunctions.add("QApplication_topLevelAt",                   f_QApplication_topLevelAt);
   //builtinFunctions.add("QApplication_topLevelWidgets",              f_QApplication_topLevelWidgets);
   //builtinFunctions.add("QApplication_type",                         f_QApplication_type);
   builtinFunctions.add("QApplication_wheelScrollLines",             f_QApplication_wheelScrollLines);
   builtinFunctions.add("QApplication_widgetAt",                     f_QApplication_widgetAt);
   builtinFunctions.add("QApplication_windowIcon",                   f_QApplication_windowIcon);
}

