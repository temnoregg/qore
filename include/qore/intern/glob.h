/*
  glob.h

  Qore Programming Language

  Copyright (C) 2003 - 2015 David Nichols

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

  Note that the Qore library is released under a choice of three open-source
  licenses: MIT (as above), LGPL 2+, or GPL 2+; see README-LICENSE for more
  information.
*/

#ifndef _QORE_INTERN_GLOB_H
#define _QORE_INTERN_GLOB_H

#include <assert.h>

#include <string>
#include <vector>

#include <qore/intern/QoreRegexNode.h>

typedef int (*glob_error_t)(const char *, int);

#ifdef _WIN32
class QoreGlobWin {
protected:
   typedef std::vector<std::string> names_t;
   names_t names;

public:
   size_t gl_pathc;
   const char **gl_pathv;

   DLLLOCAL QoreGlobWin() : gl_pathc(0), gl_pathv(0) {
   }

   DLLLOCAL ~QoreGlobWin() {
      reset();
   }

   DLLLOCAL int set(const char *pattern, int flags, glob_error_t errfunc) {
      assert(!flags);
      assert(!errfunc);
      reset();

      char *dirp = q_dirname(pattern);
      unsigned len = strlen(dirp);
      QoreString dir(dirp, len, len + 1, QCS_DEFAULT); 

      // set the pattern to get all files in the directory, and then match according to glob() rules
      dir.concat("\\*.*");
      WIN32_FIND_DATA pfd;
      HANDLE h = ::FindFirstFile(dir.getBuffer(), &pfd);
      ON_BLOCK_EXIT(::FindClose, h);

      // make regex pattern
      QoreString str(q_basenameptr(pattern));
      str.replaceAll(".", "\\.");
      str.replaceAll("?", ".");
      str.replaceAll("*", ".*");
      str.prepend("^");
      str.concat("$");

      ExceptionSink xsink;
      SimpleRefHolder<QoreRegexNode> qrn(new QoreRegexNode(&str, PCRE_CASELESS, &xsink));
      if (xsink)
	 return -1;

      while (FindNextFile(h, &pfd)) {
	 if (qrn->exec(pfd.cFileName, strlen(pfd.cFileName))) {
	    names.push_back(pfd.cFileName);
	    //printd(5, "QoreGlobWin::set(pattern='%s') dir='%s' regex='%s' %s MATCHED\n", pattern, dir.getBuffer(), str.getBuffer(), pfd.cFileName);
	 }
      }

      if (names.size()) {
	 gl_pathc = names.size();

	 gl_pathv = (const char **)malloc(sizeof(char *) * names.size());
	 for (unsigned i = 0; i < names.size(); ++i) {
	    gl_pathv[i] = names[i].c_str();
	 }
      }

      return 0;
   }

   DLLLOCAL int reset() {
      names.clear();
      gl_pathc = 0;
      if (gl_pathv) {
	 free(gl_pathv);
	 gl_pathv = 0;
      }
      return 0;
   }
};

typedef QoreGlobWin glob_t;
#else
#error no glob implementation for this platform
#endif

// check prototypes
DLLLOCAL int glob(const char *pattern, int flags, glob_error_t errfunc, glob_t *buf);
DLLLOCAL int globfree(glob_t *buf);

#endif
