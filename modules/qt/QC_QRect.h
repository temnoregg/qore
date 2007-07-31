/*
 QC_QRect.h
 
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

#ifndef _QORE_QC_QRECT_H

#define _QORE_QC_QRECT_H

#include <QRect>

DLLLOCAL extern int CID_QRECT;
DLLLOCAL extern class QoreClass *QC_QRect;

DLLLOCAL class QoreClass *initQRectClass();

class QoreQRect : public AbstractPrivateData, public QRect
{
   public:
      DLLLOCAL QoreQRect() : QRect()
      {
      }
      DLLLOCAL QoreQRect(int x, int y, int width, int height) : QRect(x, y, width, height)
      {
      }
      DLLLOCAL QoreQRect(const QRect &qr) : QRect(qr)
      {
      }
/*
      DLLLOCAL QoreQRect(const QRect &qr) : QRect(qr.x(), qr.y(), qr.width(), qr.height())
      {
      }
*/   
};


#endif
