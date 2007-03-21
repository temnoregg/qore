/*
  sybase_read_output.h

  Sybase DB layer for QORE
  uses Sybase OpenClient C library

  Qore Programming language

  Copyright (C) 2007

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

#ifndef SYBASE_READ_OUTPUT_H_
#define SYBASE_READ_OUTPUT_H_

class QoreNode;
class QoreEncoding;
class sybase_command_wrapper;
class ExceptionSink;

#include "sybase_query_parser.h"
#include "sybase_low_level_interface.h"

//------------------------------------------------------------------------------
// Read output of a command (SQL or procedure call) and convert it into Qore node (hash or list of hashes)
extern QoreNode* convert_sybase_output_to_Qore(const sybase_command_wrapper& wrapper, const QoreEncoding* encoding, const processed_sybase_query& query_info, std::vector<parameter_info_t>& outputs_info, ExceptionSink* xsink);

#endif

// EOF

