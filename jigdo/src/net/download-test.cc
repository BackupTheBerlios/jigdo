/* $Id: download-test.cc,v 1.1 2004/08/03 16:31:53 atterer Exp $ -*- C++ -*-
  __   _
  |_) /|  Copyright (C) 2004  |  richard@
  | \/�|  Richard Atterer     |  atterer.net
  � '` �
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2. See
  the file COPYING for details.

  #test-deps net/download.o glibcurl/glibcurl.o

*/

#define DEBUG 1
#include <config.h>

#include <download.hh>
#include <log.hh>

namespace {

  void testUriJoin(const char* base, const char* rel, const char* expected) {
    string s = "anything", b = base, r = rel;
    Download::uriJoin(&s, b, r);
    msg("base=%1, rel=%2, result=%3, expected=%4", b, r, s, expected);
    Assert(s == expected);
  }

}

int main(int argc, char* argv[]) {
  if (argc == 2) Logger::scanOptions(argv[1], argv[0]);

  testUriJoin("foo", "bar", "foobar");
  testUriJoin("http://base/", "rel", "http://base/rel");
  testUriJoin("http://base/boo.html", "rel", "http://base/rel");
  testUriJoin("http://base/", "http://wah/", "http://wah/");
  testUriJoin("http://base/", "telnet:", "telnet:");

  /* Should ideally eliminate ../ if possible. */
  testUriJoin("http://base/", "../../x", "http://base/../../x");
  testUriJoin("http://base/a/b/", "../x/y", "http://base/a/b/../x/y");

  msg("Exit");
  return 0;
}
