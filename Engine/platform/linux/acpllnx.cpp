//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#if !defined(LINUX_VERSION)
#error This file should only be included on the Linux or BSD build
#endif

// ********* LINUX PLACEHOLDER DRIVER *********

#include <stdio.h>
#include <allegro.h>
#include <xalleg.h>
#include "gfx/ali3d.h"
#include "ac/runtime_defines.h"
#include "debug/out.h"
#include "platform/base/agsplatformdriver.h"
#include "plugin/agsplugin.h"
#include "util/string.h"
#include <libcda.h>

#include <pwd.h>
#include <sys/stat.h>

using namespace AGS::Common;


// Replace the default Allegro icon. The original defintion is in the
// Allegro 4.4 source under "src/x/xwin.c".
#include "icon.xpm"
void* allegro_icon = icon_xpm;
String LinuxOutputDirectory;

struct AGSLinux : AGSPlatformDriver {
  AGSLinux();

  virtual int  CDPlayerCommand(int cmdd, int datt);
  virtual void Delay(int millis);
  virtual void DisplayAlert(const char*, ...);
  virtual const char *GetUserSavedgamesDirectory();
  virtual const char *GetAppOutputDirectory();
  virtual unsigned long GetDiskFreeSpaceMB();
  virtual const char* GetNoMouseErrorString();
  virtual const char* GetAllegroFailUserHint();
  virtual eScriptSystemOSID GetSystemOSID();
  virtual int  InitializeCDPlayer();
  virtual void PlayVideo(const char* name, int skip, int flags);
  virtual void PostAllegroInit(bool windowed);
  virtual void PostAllegroExit();
  virtual void SetGameWindowIcon();
  virtual void ShutdownCDPlayer();
  virtual bool LockMouseToWindow();
  virtual void UnlockMouse();
};

AGSLinux::AGSLinux() {
  strcpy(psp_game_file_name, "agsgame.dat");
  strcpy(psp_translation, "default");

  BrInitError e = BR_INIT_ERROR_DISABLED;
  if (br_init(&e)) {
    char *exedir = br_find_exe_dir(NULL);
    if (exedir) {
      chdir(exedir);
      free(exedir);
    }
  }

  // force private modules
  setenv("ALLEGRO_MODULES", ".", 1);
}

int AGSLinux::CDPlayerCommand(int cmdd, int datt) {
  return cd_player_control(cmdd, datt);
}

void AGSLinux::DisplayAlert(const char *text, ...) {
  char displbuf[2000];
  va_list ap;
  va_start(ap, text);
  vsprintf(displbuf, text, ap);
  va_end(ap);
  printf("%s", displbuf);
}

size_t BuildXDGPath(char *destPath, size_t destSize)
{
  // Check to see if XDG_DATA_HOME is set in the enviroment
  const char* home_dir = getenv("XDG_DATA_HOME");
  size_t l = 0;
  if (home_dir)
  {
    l = snprintf(destPath, destSize, "%s", home_dir);
  }
  else
  {
    // No evironment variable, so we fall back to home dir in /etc/passwd
    struct passwd *p = getpwuid(getuid());
    l = snprintf(destPath, destSize, "%s/.local", p->pw_dir);
    if (mkdir(destPath, 0755) != 0 && errno != EEXIST)
      return 0;
    l += snprintf(destPath + l, destSize - l, "/share");
    if (mkdir(destPath, 0755) != 0 && errno != EEXIST)
      return 0;
  }
  l += snprintf(destPath + l, destSize - l, "/ags");
  if (mkdir(destPath, 0755) != 0 && errno != EEXIST)
    return 0;
  return l;
}

void DetermineAppOutputDirectory()
{
  if (!LinuxOutputDirectory.IsEmpty())
  {
    return;
  }

  char xdg_path[256];
  if (BuildXDGPath(xdg_path, sizeof(xdg_path)) > 0)
    LinuxOutputDirectory = xdg_path;
  else
    LinuxOutputDirectory = "/tmp";
}

const char *AGSLinux::GetUserSavedgamesDirectory()
{
  DetermineAppOutputDirectory();
  return LinuxOutputDirectory;
}

const char *AGSLinux::GetAppOutputDirectory()
{
  DetermineAppOutputDirectory();
  return LinuxOutputDirectory;
}

void AGSLinux::Delay(int millis) {
  while (millis >= 5) {
    usleep(5);
    millis -= 5;

    update_polled_stuff(false);
  }
  if (millis > 0)
    usleep(millis);
}

unsigned long AGSLinux::GetDiskFreeSpaceMB() {
  // placeholder
  return 100;
}

const char* AGSLinux::GetNoMouseErrorString() {
  return "This game requires a mouse. You need to configure and setup your mouse to play this game.\n";
}

// extern int INIreadint (const char *sectn, const char *item, int errornosect = 1);

const char* AGSLinux::GetAllegroFailUserHint()
{
  return "Make sure you have latest version of Allegro 4 libraries installed, and X server is running.";
}

eScriptSystemOSID AGSLinux::GetSystemOSID() {
  // int fake_win =  INIreadint("misc", "fake_os", 0);
  // if (fake_win > 0) {
  //   return (eScriptSystemOSID)fake_win;
  // } else {
  //   return eOS_Linux;
  // }
  return eOS_Linux;
}

int AGSLinux::InitializeCDPlayer() {
  return cd_player_init();
}

void AGSLinux::PlayVideo(const char *name, int skip, int flags) {
  // do nothing
}

//
// Quoting Benoit Pierre:
// When using a high polling rate mouse on Linux with X11, the mouse cursor
// lags. This is due to the fact that Allegro will only process up-to 5
// X11 events at a time, and so mouse motion events will pile up and the
// mouse cursor will lag. It's possible to fix it without patching Allegro
// by setting a custom Allegro X11 input handler that will make sure all
// currently queued X11 events are processed.
//
// NOTE: this refers specifically to Allegro 4.
// TODO: if AGS ever uses patched Allegro version, this custom handler may
// be removed.
//
static void XWinInputHandler(void)
{
  if (!_xwin.display)
    return;

  // Repeat the call to Allegro's internal input handler until all the event
  // queue was processed
  while (XQLength(_xwin.display) > 0)
    _xwin_private_handle_input();
}

void AGSLinux::PostAllegroInit(bool windowed)
{
  _xwin_input_handler = XWinInputHandler;
  Out::FPrint("Set up the custom XWin input handler");
}

void AGSLinux::PostAllegroExit() {
  // do nothing
}

void AGSLinux::SetGameWindowIcon() {
  // do nothing
}

void AGSLinux::ShutdownCDPlayer() {
  cd_exit();
}

AGSPlatformDriver* AGSPlatformDriver::GetDriver() {
  if (instance == NULL)
    instance = new AGSLinux();
  return instance;
}

void AGSLinux::ReplaceSpecialPaths(const char *sourcePath, char *destPath) {
  // MYDOCS is what is used in acplwin.cpp
  if(strncasecmp(sourcePath, "$MYDOCS$", 8) == 0) {
    struct passwd *p = getpwuid(getuid());
    strcpy(destPath, p->pw_dir);
    strcat(destPath, "/.local");
    mkdir(destPath, 0755);
    strcat(destPath, "/share");
    mkdir(destPath, 0755);
    strcat(destPath, &sourcePath[8]);
    mkdir(destPath, 0755);
  // SAVEGAMEDIR is what is actually used in ac.cpp
  } else if(strncasecmp(sourcePath, "$SAVEGAMEDIR$", 13) == 0) {
    struct passwd *p = getpwuid(getuid());
    strcpy(destPath, p->pw_dir);
    strcat(destPath, "/.local");
    mkdir(destPath, 0755);
    strcat(destPath, "/share");
    mkdir(destPath, 0755);
    strcat(destPath, &sourcePath[8]);
    mkdir(destPath, 0755);
  } else if(strncasecmp(sourcePath, "$APPDATADIR$", 12) == 0) {
    struct passwd *p = getpwuid(getuid());
    strcpy(destPath, p->pw_dir);
    strcat(destPath, "/.local");
    mkdir(destPath, 0755);
    strcat(destPath, "/share");
    mkdir(destPath, 0755);
    strcat(destPath, &sourcePath[12]);
    mkdir(destPath, 0755);
  } else {
    strcpy(destPath, sourcePath);
  }
}

bool AGSLinux::LockMouseToWindow()
{
    return XGrabPointer(_xwin.display, _xwin.window, False,
        PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
        GrabModeAsync, GrabModeAsync, _xwin.window, None, CurrentTime) == GrabSuccess;
}

void AGSLinux::UnlockMouse()
{
    XUngrabPointer(_xwin.display, CurrentTime);
}
