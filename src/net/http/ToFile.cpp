/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2021 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "ToFile.hpp"
#include "Request.hxx"
#include "Handler.hxx"
#include "Progress.hpp"
#include "Operation/Operation.hpp"
#include "io/FileOutputStream.hxx"
#include "Crypto/SHA256.hxx"
#include "thread/AsyncWaiter.hxx"
#include "util/ScopeExit.hxx"

#include <cassert>

class DownloadToFileHandler final : public CurlResponseHandler {
  OutputStream &out;

  SHA256State sha256;

  AsyncWaiter waiter;

  const bool do_sha256;

public:
  DownloadToFileHandler(OutputStream &_out, bool _do_sha256) noexcept
    :out(_out), do_sha256(_do_sha256)
  {
  }

  auto GetSHA256() noexcept {
    assert(do_sha256);

    return sha256.Final();
  }

  void Cancel() noexcept {
    waiter.SetDone();
  }

  void Wait() {
    waiter.Wait();
  }

  /* virtual methods from class CurlResponseHandler */
  void OnHeaders(unsigned status,
                 std::multimap<std::string, std::string> &&headers) override {
  }

  void OnData(ConstBuffer<void> data) override {
    if (do_sha256)
      sha256.Update(data);

    try {
      out.Write(data.data, data.size);
    } catch (...) {
      waiter.SetError(std::current_exception());
      throw Pause{};
    }
  }

  void OnEnd() override {
    waiter.SetDone();
  }

  void OnError(std::exception_ptr e) noexcept override {
    waiter.SetError(std::move(e));
  }
};

static void
DownloadToFile(CurlGlobal &curl, const char *url,
               const char *username, const char *password,
               OutputStream &out, std::array<std::byte, 32> *sha256,
               OperationEnvironment &env)
{
  assert(url != nullptr);

  DownloadToFileHandler handler(out, sha256 != nullptr);
  CurlRequest request(curl, url, handler);
  AtScopeExit(&request) { request.StopIndirect(); };

  request.SetFailOnError();

  if (username != nullptr)
    request.SetOption(CURLOPT_USERNAME, username);
  if (password != nullptr)
    request.SetOption(CURLOPT_PASSWORD, password);

  env.SetCancelHandler([&]{
    request.StopIndirect();
    handler.Cancel();
  });

  AtScopeExit(&env) { env.SetCancelHandler({}); };

  const Net::ProgressAdapter progress(request.GetEasy(), env);

  request.StartIndirect();
  handler.Wait();

  if (sha256 != nullptr)
    *sha256 = handler.GetSHA256();
}

void
Net::DownloadToFile(CurlGlobal &curl, const char *url,
                    const char *username, const char *password,
                    Path path, std::array<std::byte, 32> *sha256,
                    OperationEnvironment &env)
{
  assert(url != nullptr);
  assert(path != nullptr);

  FileOutputStream file(path);
  ::DownloadToFile(curl, url, username, password,
                   file, sha256, env);
  file.Commit();
}

void
Net::DownloadToFileJob::Run(OperationEnvironment &env)
{
  DownloadToFile(curl, url, username, password, path, &sha256, env);
}
