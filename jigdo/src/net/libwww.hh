/* $Id: libwww.hh,v 1.6 2003/08/13 21:25:13 atterer Exp $ -*- C++ -*-
  __   _
  |_) /|  Copyright (C) 2001-2002  |  richard@
  | \/�|  Richard Atterer          |  atterer.net
  � '` �
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2. See
  the file COPYING for details.

  Include libwww headers, clean up namespace afterwards.

*/

#ifndef LIBWWW_HH
#define LIBWWW_HH

#include <dirent.hh>

#undef PACKAGE

extern "C" {
#include <HTInit.h>
#include <HTNet.h>
#include <HTReqMan.h>
#include <WWWCore.h>
#include <WWWStream.h>
#include <WWWTrans.h>
//#include "WWWUtil.h"

// HTReqMan.h incorrectly defines the name as HTRequest_deleteRangeAll
extern BOOL HTRequest_deleteRange (HTRequest * request);
}

#include <config.h>
#undef PACKAGE
#define PACKAGE "jigdo"
#undef _
#if ENABLE_NLS
#  define _(String) dgettext (PACKAGE, String)
#  else
#  define _(String) (String)
#endif

#endif
