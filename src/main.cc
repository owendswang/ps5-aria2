/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "common.h"

#include <unistd.h>

#ifdef ARIA2_PS5
#  include <cerrno>
#  include <cstdio>
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/syscall.h>
#  include <vector>

extern "C" int sceKernelSendNotificationRequest(int, void*, size_t, int);
#endif

#ifdef __MINGW32__
#  include <shellapi.h>
#endif // __MINGW32__

#include <aria2/aria2.h>
#include "Context.h"
#include "MultiUrlRequestInfo.h"
#include "message.h"
#include "Platform.h"
#include "Exception.h"
#include "console.h"
#include "LogFactory.h"

namespace aria2 {

error_code::Value main(int argc, char** argv)
{
#ifdef __MINGW32__
  int winArgc;
  auto winArgv = CommandLineToArgvW(GetCommandLineW(), &winArgc);
  if (winArgv == nullptr) {
    A2_LOG_ERROR("Reading command-line failed");
    return error_code::UNKNOWN_ERROR;
  }
  std::vector<std::unique_ptr<char>> winArgStrs;
  winArgStrs.reserve(winArgc);
  auto pargv = make_unique<char*[]>(winArgc);
  for (int i = 0; i < winArgc; ++i) {
    winArgStrs.emplace_back(strdup(wCharToUtf8(winArgv[i]).c_str()));
    pargv[i] = winArgStrs.back().get();
  }

  Context context(true, winArgc, pargv.get(), KeyVals());
#else  // !__MINGW32__
#  ifdef ARIA2_PS5
  char confPath[] = "--conf-path=/data/aria2/aria2.conf";
  std::vector<char*> ps5Argv;
  ps5Argv.reserve(argc + 1);
  ps5Argv.push_back(argv[0]);
  ps5Argv.push_back(confPath);
  ps5Argv.insert(ps5Argv.end(), argv + 1, argv + argc);
  Context context(true, argc + 1, ps5Argv.data(), KeyVals());
#  else
  Context context(true, argc, argv, KeyVals());
#  endif
#endif

  error_code::Value exitStatus = error_code::FINISHED;
  if (context.reqinfo) {
    exitStatus = context.reqinfo->execute();
  }
  return exitStatus;
}

} // namespace aria2

int main(int argc, char** argv)
{
#ifdef ARIA2_PS5
  syscall(SYS_thr_set_name, -1, "aria2.elf");
#endif
  aria2::error_code::Value r;
  aria2::global::initConsole(false);
#ifdef ARIA2_PS5
  if (mkdir("/data/aria2", 0755) != 0 && errno != EEXIST) {
    aria2::global::cerr()->printf("Cannot create PS5 data directory (errno=%d).\n",
                                  errno);
    return aria2::error_code::UNKNOWN_ERROR;
  }
  int instanceFd = open("/data/aria2/aria2.lock", O_CREAT | O_RDWR | O_CLOEXEC,
                        0600);
  if (instanceFd < 0) {
    aria2::global::cerr()->printf("Cannot open PS5 instance lock (errno=%d).\n",
                                  errno);
    return aria2::error_code::UNKNOWN_ERROR;
  }
  if (flock(instanceFd, LOCK_EX | LOCK_NB) != 0) {
    const int lockError = errno;
    close(instanceFd);
    if (lockError == EWOULDBLOCK) {
      struct NotificationRequest {
        char reserved[45];
        char message[3075];
      } request = {};
      std::snprintf(request.message, sizeof(request.message),
                    "aria2 v%s\nAlready running...", PACKAGE_VERSION);
      if (sceKernelSendNotificationRequest(0, &request, sizeof(request), 0) !=
          0) {
        aria2::global::cerr()->printf(
            "Failed to send PS5 already-running notification.\n");
      }
      aria2::global::cerr()->printf("aria2.elf is already running.\n");
      return aria2::error_code::FINISHED;
    }
    aria2::global::cerr()->printf("Cannot lock PS5 instance file (errno=%d).\n",
                                  lockError);
    return aria2::error_code::UNKNOWN_ERROR;
  }
  int sessionFd = open("/data/aria2/aria2.session",
                       O_CREAT | O_RDWR | O_CLOEXEC, 0644);
  if (sessionFd < 0) {
    aria2::global::cerr()->printf("Cannot open PS5 session file (errno=%d).\n",
                                  errno);
    close(instanceFd);
    return aria2::error_code::UNKNOWN_ERROR;
  }
  close(sessionFd);
#endif
  try {
    aria2::Platform platform;
    r = aria2::main(argc, argv);
  }
  catch (aria2::Exception& ex) {
    aria2::global::cerr()->printf("%s\n%s\n", EX_EXCEPTION_CAUGHT,
                                  ex.stackTrace().c_str());
    r = ex.getErrorCode();
  }
#ifdef ARIA2_PS5
  close(instanceFd);
#endif
  return r;
}
