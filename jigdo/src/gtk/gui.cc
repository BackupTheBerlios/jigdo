/* $Id: gui.cc,v 1.10 2004/06/26 11:28:46 atterer Exp $ -*- C++ -*-
  __   _
  |_) /|  Copyright (C) 2001-2003  |  richard@
  | \/�|  Richard Atterer          |  atterer.net
  � '` �
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2. See
  the file COPYING for details.

  Interface to GTK GUI. Sets up global variables pointing to the main
  GUI elements.

  "Glue code" between the interface as generated by glade (in
  gtk-interface.*, gtk-support.*) and jigdo's functionality.

*/

#include <config.h>

#include <fstream>
#include <ctype.h>

#include <gtk-single-url.hh>
#include <gtk-makeimage.hh>
#include <gui.hh>
#include <joblist.hh>
#include <messagebox.hh>
#include <string-utf.hh>
//______________________________________________________________________

DEBUG_UNIT("gui")

/* Not all of these are set up during create() - we rely on the vars
   being initialized to 0 in that case. */
GUI::Window GUI::window;
GUI::Filesel GUI::filesel;
GUI::License GUI::license;
//______________________________________________________________________

// File selection dialogue - helper functions
namespace {
  using namespace GUI;

  GtkEntry* filesel_state = 0;

  // Private function for filesel_open below
  void on_filesel_okButton(GtkWidget*, gpointer) {
    if (filesel_state != 0) {
      // Copy over filename to the right GtkEntry
      GtkFileSelection* fileselObj = GTK_FILE_SELECTION(filesel.filesel);
      gtk_entry_set_text(filesel_state,
                         gtk_file_selection_get_filename(fileselObj));
    }
    gtk_widget_hide(filesel.filesel); // Close file selection window
  }

  /* Called to open filesel on e.g. a button click. When filesel's OK
     button is clicked, the selected filename is copied to "entry". */
  void filesel_open(GtkWidget*, gpointer entry) {
    gtk_widget_show(filesel.filesel);
    filesel_state = GTK_ENTRY(entry);
  }
}
//______________________________________________________________________

/* Initialisation of the GUI. Mostly does some post-processing of the
   glade-generated data structures, connecting button press events to
   the corresponding function etc. */
void GUI::create() {
  window.create();
  filesel.create();
  GUI::jobList.postGtkInit();

  /* Remove tabs from main window notebook, since the user is only
     allowed to switch between the pages using clicks on buttons/list
     entries. */
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK(window.invisibleNotebook),FALSE);

  // Set background white for jigdo welcome screen
  GdkColor white;
  gdk_color_parse("white", &white);
  gtk_widget_modify_bg(window.aboutBgnd, GTK_STATE_NORMAL, &white);

//   GtkRcStyle* aboutStyle = gtk_widget_get_modifier_style(window.aboutBgnd);
//   gdk_color_parse("white", &aboutStyle->bg[0]);
//   ... modify style - how?
//   gtk_widget_modify_style(window.aboutBgnd, aboutStyle);

  gtk_widget_set_name(window.aboutBgnd, "aboutBgnd");
  gtk_rc_parse_string("style \"whitebg\" { bg[NORMAL] = \"white\" }");
  // base[NORMAL] = \"white\"
  gtk_rc_parse_string("widget \"aboutBgnd\" style \"whitebg\"");

  // Set string "Jigsaw Download x.y.z" on welcome screen
  GtkLabel* aboutJigdoLabel = GTK_LABEL(GUI::window.aboutJigdoLabel);
  string banner = subst(_(
    "<span weight=\"bold\" foreground=\"black\">"
    "<span size=\"x-large\">Jigsaw Download %F1</span>\n"
    "Copyright 2001-2004 Richard Atterer\n"
    "http://atterer.net/jigdo</span>"), JIGDO_VERSION);
  gtk_label_set_markup(aboutJigdoLabel, banner.c_str());
  gtk_label_set_justify(aboutJigdoLabel, GTK_JUSTIFY_CENTER);
  //gtk_label_set_justify(GTK_LABEL(GUI::window.aboutJigdoButtonLabel),
  //                      GTK_JUSTIFY_CENTER);
  gtk_label_set_markup(GTK_LABEL(GUI::window.aboutJigdoButtonLabel), _(
    "<span size=\"x-small\">This is Free Software, distributable under the "
    "terms of the GNU GPL v2.</span>"));

  // Set width of image_Info label to something bigger than the default
  GtkRequisition infoReq;
  gtk_widget_size_request(GUI::window.window, &infoReq);
  //gtk_widget_get_size_request(GUI::window.jigdo_InfoVbox, &infoReq.width,
  //&infoReq.height);
  debug("updateWindow width=%1", infoReq.width);
  int width = infoReq.width - 100;
  if (width < 200) width = 450;
  gtk_widget_set_size_request(GUI::window.jigdo_Info, width, -1);

  /* Handler to copy filename to right GtkEntry when OK is pressed
     in filesel */
  g_signal_connect(G_OBJECT(filesel.okButton), "clicked",
                   G_CALLBACK(on_filesel_okButton), NULL);

  /* Open filesel window when buttons are clicked, enter result in
     appropriate GtkEntry */
  g_signal_connect(G_OBJECT(window.open_URLSel), "clicked",
                   G_CALLBACK(filesel_open),
                   G_OBJECT(window.open_URL));
  g_signal_connect(G_OBJECT(window.open_destSel), "clicked",
                   G_CALLBACK(filesel_open),
                   G_OBJECT(window.open_dest));

  gtk_widget_hide_all(window.toolbarReuse);

# if WINDOWS
  gtk_entry_set_text(GTK_ENTRY(window.open_dest), g_get_home_dir());
# else
  char* cwd = g_get_current_dir();
  gtk_entry_set_text(GTK_ENTRY(window.open_dest), cwd);
  g_free(cwd);
# endif

# if DEBUG
  gtk_entry_set_text(GTK_ENTRY(window.open_URL),
                     //"ftp://localhost/image"
                     //"http://localhost:8000/~richard/ironmaiden/part32"
#                    if WINDOWS
                     "http://10.0.0.5:8000/~richard/ironmaiden/image.jigdo"
#                    else
                     "http://127.0.0.1:8000/~richard/ironmaiden/image.jigdo"
#                    endif
                     );
# endif
}
//______________________________________________________________________

// Callback handlers

void on_toolbarExit_clicked(GtkButton*, gpointer) {
  gtk_main_quit();
}

gboolean on_window_delete_event(GtkWidget*, GdkEvent*, gpointer) {
  gtk_main_quit();
  return TRUE;
}

void setNotebookPage(GtkWidget* pageObject) {
  GUI::jobList.setWindowOwner(0);
  GtkNotebook* notebook = GTK_NOTEBOOK(pageObject->parent);
  gboolean sensitive = (pageObject == GUI::window.pageOpen ? FALSE : TRUE);
  gtk_widget_set_sensitive(GUI::window.toolbarOpen, sensitive);
  gtk_notebook_set_current_page(notebook,
                                gtk_notebook_page_num(notebook, pageObject));
}

void on_openButton_clicked(GtkButton*, gpointer) {
  /* Create either a JobLine to download a single file, or one to process a
     .jigdo file, and GUI::jobList.append() it. */
  const char* uri = gtk_entry_get_text(GTK_ENTRY(window.open_URL));
  const char* dest = gtk_entry_get_text(GTK_ENTRY(window.open_dest));
  JobLine::create(uri, dest);
}
//________________________________________

namespace {
  // Set up a text buffer which displays the jigdo license
  void setLicenseText(GtkTextBuffer* textBuf) {
    GtkTextTag* large = gtk_text_buffer_create_tag(
      textBuf, NULL, "scale", 2.0, NULL);
    GtkTextTag* center = gtk_text_buffer_create_tag(
      textBuf, NULL, "justification", GTK_JUSTIFY_CENTER, NULL);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(textBuf, &iter);
    gtk_text_buffer_insert_with_tags(textBuf, &iter,
      _("\nJigsaw Download License\n"), -1, large, center, NULL);
    string copy = subst(_("\n"
    "\tCopyright © 2001-2004 Richard Atterer <richard%1atterer.net>\n"
    "\tJigsaw Download homepage: http://atterer.net/jigdo\n"
    "\n"), '@');
    gtk_text_buffer_insert_with_tags(textBuf, &iter, copy.c_str(), -1,
                                     center, NULL);
    gtk_text_buffer_insert(textBuf, &iter, _(
    "Jigsaw Download is free software; you can redistribute it and/or "
    "modify it under the terms of the GNU General Public License, version "
    "2, as published by the Free Software Foundation.\n"
    "\n"
    "In addition, as a special exception, the author gives permission "
    "to link the jigdo code with the OpenSSL project's \"OpenSSL\" library "
    "(or with modified versions of it that use the same license as the "
    "\"OpenSSL\" library), and to distribute the linked executables.\n"),
     -1);
    gtk_text_buffer_insert_with_tags(textBuf, &iter,
                                     "______________________________\n\n",
                                     -1, center, NULL);
    gtk_text_buffer_insert(textBuf, &iter, _(
    "Jigsaw Download uses the World Wide Web Consortium's \"libwww\" "
    "library (see <http://www.w3.org/Library/>), to which the "
    "following license applies:\n"
    "\n"
    "Copyright © 1995-1998 World Wide Web Consortium, (Massachusetts "
    "Institute of Technology, Institut National de Recherche en "
    "Informatique et en Automatique, Keio University). All Rights Reserved. "
    "This program is distributed under the W3C's Intellectual Property "
    "License. This program is distributed in the hope that it will be "
    "useful, but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See W3C License "
    "<http://www.w3.org/Consortium/Legal/> for more details.\n"
    "\n"
    "Copyright © 1995 CERN. \"This product includes computer software "
    "created and made available by CERN. This acknowledgment shall be "
    "mentioned in full in any product which includes the CERN computer "
    "software included herein or parts thereof.\"\n"),
     -1);
    gtk_text_buffer_insert_with_tags(textBuf, &iter,
                                     "______________________________\n\n",
                                     -1, center, NULL);
    gtk_text_buffer_insert(textBuf, &iter, _(
    "Please note: The copyright notice below only applies to the text of "
    "the GNU General Public License; the copyright of the program is as "
    "specified above. Also note that jigdo is licensed under GPL version "
    "_2_ and no other version.\n"
    "\n\n"), -1);
    string copying = packageDataDir; copying += "COPYING";
    static const int BUF_SIZE = 1024;
    char buf[BUF_SIZE];
    ifstream file(copying.c_str());
    while (file) {
      file.read(buf, BUF_SIZE);
      size_t n = file.gcount();
      for (size_t i = 1; i < n - 2; ++i) { // Very simple reformatting
        if (buf[i] == '\f')
          buf[i] = '\n';
        else if (buf[i] == '\n' && buf[i - 1] != '\n' && isgraph(buf[i + 1]))
          buf[i] = ' ';
        else if (buf[i] == '\n')
          { }
        // Whoa, we're supposed to be outputting UTF8 - just delete non-ASCII
        else if ((buf[i] < 32 || buf[i] >= 127) && buf[i] != '\t')
          buf[i] = '?';
      }
      gtk_text_buffer_insert(textBuf, &iter, buf, file.gcount());
    }
    if (!file.eof() && !file) {
      string err = subst(_("Error loading `%LE1': %LE2"),
                         copying, strerror(errno));
      gtk_text_buffer_insert(textBuf, &iter, err.c_str(), -1);
    }
  }

} // namespace
//________________________________________

// Display jigdo license
void on_aboutJigdoButton_clicked(GtkButton*, gpointer) {
  if (GUI::license.license == 0) {
    GUI::license.create();
    GtkTextView* view = GTK_TEXT_VIEW(GUI::license.licenseText);
    GtkTextBuffer* textBuf = gtk_text_view_get_buffer(view);
    setLicenseText(textBuf);

    GtkTextIter iter;
    gtk_text_buffer_get_start_iter(textBuf, &iter);
    gtk_text_buffer_place_cursor(textBuf, &iter);

    GdkGeometry geom;
    geom.min_width = geom.min_height = 1;
    gtk_window_set_geometry_hints(GTK_WINDOW(GUI::license.license), GUI::license.licenseText, &geom, GDK_HINT_MIN_SIZE);
  }
  gtk_window_present(GTK_WINDOW(GUI::license.license));
}
//________________________________________

void on_download_startButton_clicked(GtkButton*, gpointer) {
  GtkSingleUrl* j = dynamic_cast<GtkSingleUrl*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_startButton_clicked();
}
void on_download_pauseButton_clicked(GtkButton*, gpointer) {
  GtkSingleUrl* j = dynamic_cast<GtkSingleUrl*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_pauseButton_clicked();
}
void on_download_stopButton_clicked(GtkButton*, gpointer) {
  GtkSingleUrl* j = dynamic_cast<GtkSingleUrl*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_stopButton_clicked();
}
void on_download_restartButton_clicked(GtkButton*, gpointer) {
  GtkSingleUrl* j = dynamic_cast<GtkSingleUrl*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_restartButton_clicked();
}
void on_download_closeButton_clicked(GtkButton*, gpointer) {
  GtkSingleUrl* j = dynamic_cast<GtkSingleUrl*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_closeButton_clicked();
}

void on_download_startButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.download_buttonInfo),
                       _("<b>Continue</b> download (after Pause) or "
                         "<b>Resume</b> it (after Stop)"));
}
void on_download_pauseButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.download_buttonInfo),
                       _("<b>Pause</b> download, but leave connection "
                         "to server open"));
}
void on_download_stopButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.download_buttonInfo),
                       _("<b>Stop</b> download by closing the connection"));
}
void on_download_restartButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.download_buttonInfo),
                       _("<b>Restart</b> download - the data downloaded so "
                         "far is discarded"));
}
void on_download_closeButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.download_buttonInfo),
                       _("<b>Close</b> this download and remove it from the "
                         "list below"));
}
void on_download_button_leave(GtkButton*, gpointer) {
  gtk_label_set_text(GTK_LABEL(GUI::window.download_buttonInfo), "");
}
//________________________________________

void on_jigdo_startButton_clicked(GtkButton*, gpointer) {
  GtkMakeImage* j = dynamic_cast<GtkMakeImage*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_startButton_clicked();
}
void on_jigdo_pauseButton_clicked(GtkButton*, gpointer) {
  GtkMakeImage* j = dynamic_cast<GtkMakeImage*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_pauseButton_clicked();
}
void on_jigdo_stopButton_clicked(GtkButton*, gpointer) {
  GtkMakeImage* j = dynamic_cast<GtkMakeImage*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_stopButton_clicked();
}
void on_jigdo_restartButton_clicked(GtkButton*, gpointer) {
  GtkMakeImage* j = dynamic_cast<GtkMakeImage*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_restartButton_clicked();
}
void on_jigdo_closeButton_clicked(GtkButton*, gpointer) {
  GtkMakeImage* j = dynamic_cast<GtkMakeImage*>(GUI::jobList.windowOwner());
  if (j != 0) j->on_closeButton_clicked();
}

void on_jigdo_startButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.jigdo_buttonInfo),
                       _("<b>Continue</b> all child downloads (after Pause) "
                         "or <b>Resume</b> them (after Stop or error)"));
}
void on_jigdo_pauseButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.jigdo_buttonInfo),
                       _("<b>Pause</b> all child downloads, but leave "
                         "connections to servers open"));
}
void on_jigdo_stopButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.jigdo_buttonInfo),
                       _("<b>Stop</b> download by closing connections of "
                         "all children"));
}
void on_jigdo_restartButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.jigdo_buttonInfo),
                       _("<b>Restart</b> download and processing of "
                         "<tt>.jigdo</tt> - the <tt>.jigdo</tt> data "
                         "downloaded so far (and only it) is discarded"));
}
void on_jigdo_closeButton_enter(GtkButton*, gpointer) {
  gtk_label_set_markup(GTK_LABEL(GUI::window.jigdo_buttonInfo),
                       _("<b>Close</b> this download and all its children, "
                         "and remove them from the list below"));
}
void on_jigdo_button_leave(GtkButton*, gpointer) {
  gtk_label_set_text(GTK_LABEL(GUI::window.jigdo_buttonInfo), "");
}
