/* $Id: uri.hh,v 1.2 2004/08/09 08:35:05 atterer Exp $ -*- C++ -*-
  __   _
  |_) /|  Copyright (C) 2001-2003  |  richard@
  | \/�|  Richard Atterer          |  atterer.net
  � '` �
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2. See
  the file COPYING for details.

*/

#ifndef URI_HH
#define URI_HH

#include <config.h>

#include <string>

/** Create a new URI from an absolute base URI and a relative URI. (rel can
    also be absolute, in this case, the result in dest equals rel.) */
void uriJoin(string* dest, const string& base, const string& rel);

//______________________________________________________________________

/** Return offset of first ':' in string if it is preceded by characters
    other than '/', space or control characters, otherwise return 0. */
unsigned findLabelColon(const string& s);
//______________________________________________________________________

/** Return true iff the absolute URL is a "real" HTTP/FTP/.. url, as opposed
    to a label name followed by a path. */
bool isRealUrl(const string& s);

#endif
