/*
 QC_QRegExpValidator.h
 
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

#ifndef _QORE_QT_QC_QREGEXPVALIDATOR_H

#define _QORE_QT_QC_QREGEXPVALIDATOR_H

#include <QRegExpValidator>
#include "QoreAbstractQValidator.h"
#include "qore-qt-events.h"

DLLLOCAL extern int CID_QREGEXPVALIDATOR;
DLLLOCAL extern class QoreClass *QC_QRegExpValidator;

DLLLOCAL class QoreClass *initQRegExpValidatorClass(QoreClass *);

class myQRegExpValidator : public QRegExpValidator, public QoreQValidatorExtension
{
#define QOREQTYPE QRegExpValidator
#include "qore-qt-metacode.h"
#include "qore-qt-qvalidator-methods.h"
#undef QOREQTYPE

   public:
      DLLLOCAL myQRegExpValidator(Object *obj, QObject* parent) : QRegExpValidator(parent), QoreQValidatorExtension(obj->getClass())
      {
         init(obj);
      }
      DLLLOCAL myQRegExpValidator(Object *obj, const QRegExp& rx, QObject* parent) : QRegExpValidator(rx, parent), QoreQValidatorExtension(obj->getClass())
      {
         init(obj);
      }
};

class QoreQRegExpValidator : public QoreAbstractQValidator
{
   public:
      QPointer<myQRegExpValidator> qobj;

      DLLLOCAL QoreQRegExpValidator(Object *obj, QObject* parent) : qobj(new myQRegExpValidator(obj, parent))
      {
      }
      DLLLOCAL QoreQRegExpValidator(Object *obj, const QRegExp& rx, QObject* parent) : qobj(new myQRegExpValidator(obj, rx, parent))
      {
      }
      DLLLOCAL virtual class QObject *getQObject() const
      {
         return static_cast<QObject *>(&(*qobj));
      }
      DLLLOCAL virtual class QValidator *getQValidator() const
      {
         return static_cast<QValidator *>(&(*qobj));
      }
      QORE_VIRTUAL_QVALIDATOR_METHODS
};

#endif // _QORE_QT_QC_QREGEXPVALIDATOR_H
