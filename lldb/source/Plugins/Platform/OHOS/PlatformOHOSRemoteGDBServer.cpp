//===-- PlatformOHOSRemoteGDBServer.cpp ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Host/ConnectionFileDescriptor.h"
#include "lldb/Host/common/TCPSocket.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/UriParser.h"

#include "PlatformOHOSRemoteGDBServer.h"

#include <sstream>

using namespace lldb;
using namespace lldb_private;
using namespace platform_ohos;

static const lldb::pid_t g_remote_platform_pid =
    0; // Alias for the process id of lldb-platform

static uint16_t g_hdc_forward_port_offset = 0;

static Status ForwardPortWithHdc(
    const uint16_t local_port, const uint16_t remote_port,
    llvm::StringRef remote_socket_name,
    const llvm::Optional<HdcClient::UnixSocketNamespace> &socket_namespace,
    std::string &device_id) {
  Log *log = GetLog(LLDBLog::Platform);

  HdcClient hdc;
  auto error = HdcClient::CreateByDeviceID(device_id, hdc);
  if (error.Fail())
    return error;

  device_id = hdc.GetDeviceID();
  LLDB_LOGF(log, "Connected to OHOS device \"%s\"", device_id.c_str());

  if (remote_port != 0) {
    LLDB_LOGF(log, "Forwarding remote TCP port %d to local TCP port %d",
              remote_port, local_port);
    return hdc.SetPortForwarding(local_port, remote_port);
  }

  LLDB_LOGF(log, "Forwarding remote socket \"%s\" to local TCP port %d",
            remote_socket_name.str().c_str(), local_port);

  if (!socket_namespace)
    return Status("Invalid socket namespace");

  return hdc.SetPortForwarding(local_port, remote_socket_name,
                               *socket_namespace);
}

static Status DeleteForwardPortWithHdc(std::pair<uint16_t, uint16_t> ports,
                                       const std::string &device_id) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Delete port forwarding %d -> %d, device=%s", ports.first,
            ports.second, device_id.c_str());

  HdcClient hdc(device_id);
  return hdc.DeletePortForwarding(ports);
}

static Status DeleteForwardPortWithHdc(std::pair<uint16_t, std::string> remote_socket,
                                       const llvm::Optional<HdcClient::UnixSocketNamespace> &socket_namespace,
                                       const std::string &device_id) {

  Log *log = GetLog(LLDBLog::Platform);
  uint16_t local_port = remote_socket.first;
  std::string remote_socket_name = remote_socket.second;
  LLDB_LOGF(log, "Delete port forwarding %d -> %s, device=%s", local_port,
            remote_socket_name.c_str(), device_id.c_str());
  if (!socket_namespace)
    return Status("Invalid socket namespace");

  HdcClient hdc(device_id);
  return hdc.DeletePortForwarding(local_port, remote_socket_name, *socket_namespace);
}

static Status FindUnusedPort(uint16_t &port) {
  Status error;

  if (const char *env_port = std::getenv("HDC_FORWARD_PORT_BASE_FOR_LLDB")) {
    port = std::atoi(env_port) + g_hdc_forward_port_offset;
    g_hdc_forward_port_offset++;
    return error;
  }

  std::unique_ptr<TCPSocket> tcp_socket(new TCPSocket(true, false));
  if (error.Fail())
    return error;

  error = tcp_socket->Listen("127.0.0.1:0", 1);
  if (error.Success())
    port = tcp_socket->GetLocalPortNumber();

  return error;
}

PlatformOHOSRemoteGDBServer::PlatformOHOSRemoteGDBServer() {}

PlatformOHOSRemoteGDBServer::~PlatformOHOSRemoteGDBServer() {
  for (const auto &it : m_port_forwards) {
    DeleteForwardPortWithHdc(it.second, m_device_id);
  }
  for (const auto &it_socket : m_remote_socket_name) {
    DeleteForwardPortWithHdc(it_socket.second, m_socket_namespace, m_device_id);
  }
}

bool PlatformOHOSRemoteGDBServer::LaunchGDBServer(lldb::pid_t &pid,
                                                     std::string &connect_url) {
  uint16_t remote_port = 0;
  std::string socket_name;
  if (!m_gdb_client_up->LaunchGDBServer("127.0.0.1", pid, remote_port, socket_name))
    return false;

  Log *log = GetLog(LLDBLog::Platform);

  auto error =
      MakeConnectURL(pid, remote_port, socket_name.c_str(), connect_url);
  if (error.Success())
    LLDB_LOGF(log, "gdbserver connect URL: %s", connect_url.c_str());

  return error.Success();
}

bool PlatformOHOSRemoteGDBServer::KillSpawnedProcess(lldb::pid_t pid) {
  DeleteForwardPort(pid);
  return m_gdb_client_up->KillSpawnedProcess(pid);
}

Status PlatformOHOSRemoteGDBServer::ConnectRemote(Args &args) {
  m_device_id.clear();

  if (args.GetArgumentCount() != 1)
    return Status(
        "\"platform connect\" takes a single argument: <connect-url>");

  llvm::Optional<URI> uri;
  const char *url = args.GetArgumentAtIndex(0);
  if (!url)
    return Status("URL is null.");
  uri = URI::Parse(url);
  if (!uri)
    return Status("Invalid URL: %s", url);
  if (uri->hostname != "localhost")
    m_device_id = static_cast<std::string>(uri->hostname);

  m_socket_namespace.reset();
  if (uri->scheme == "unix-connect")
    m_socket_namespace = HdcClient::UnixSocketNamespaceFileSystem;
  else if (uri->scheme == "unix-abstract-connect")
    m_socket_namespace = HdcClient::UnixSocketNamespaceAbstract;

  std::string connect_url;
  auto error =
      MakeConnectURL(g_remote_platform_pid, uri->port ? (*uri->port) : 0,
                     uri->path, connect_url);

  if (error.Fail())
    return error;

  args.ReplaceArgumentAtIndex(0, connect_url);

  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Rewritten platform connect URL: %s", connect_url.c_str());
  error = PlatformRemoteGDBServer::ConnectRemote(args);
  if (error.Fail())
    DeleteForwardPort(g_remote_platform_pid);

  return error;
}

Status PlatformOHOSRemoteGDBServer::DisconnectRemote() {
  DeleteForwardPort(g_remote_platform_pid);
  g_hdc_forward_port_offset = 0;
  return PlatformRemoteGDBServer::DisconnectRemote();
}

void PlatformOHOSRemoteGDBServer::DeleteForwardPort(lldb::pid_t pid) {
  Log *log = GetLog(LLDBLog::Platform);

  auto it = m_port_forwards.find(pid);
  auto it_socket = m_remote_socket_name.find(pid);
  if (it != m_port_forwards.end() && it->second.second != 0) {
    const auto error = DeleteForwardPortWithHdc(it->second, m_device_id);
    if (error.Fail()) {
      LLDB_LOGF(log, "Failed to delete port forwarding (pid=%" PRIu64
                ", fwd=(%d -> %d), device=%s): %s",
                pid, it->second.first, it->second.second, m_device_id.c_str(),
                error.AsCString());
    }
    m_port_forwards.erase(it);
  }
  
  if(it_socket != m_remote_socket_name.end()) {
    const auto error_Socket = DeleteForwardPortWithHdc(it_socket->second, m_socket_namespace, m_device_id);
    if (error_Socket.Fail()) {
      LLDB_LOGF(log, "Failed to delete port forwarding (pid=%" PRIu64
                ", fwd=(%d->%s)device=%s): %s", pid, it_socket->second.first, it_socket->second.second.c_str(), m_device_id.c_str(),error_Socket.AsCString());
    }
    m_remote_socket_name.erase(it_socket);
  }
  
  return;
}

Status PlatformOHOSRemoteGDBServer::MakeConnectURL(
    const lldb::pid_t pid, const uint16_t remote_port,
    llvm::StringRef remote_socket_name, std::string &connect_url) {
  static const int kAttempsNum = 5;

  Status error;
  // There is a race possibility that somebody will occupy a port while we're
  // in between FindUnusedPort and ForwardPortWithHdc - adding the loop to
  // mitigate such problem.
  for (auto i = 0; i < kAttempsNum; ++i) {
    uint16_t local_port = 0;
    error = FindUnusedPort(local_port);
    if (error.Fail())
      return error;

    error = ForwardPortWithHdc(local_port, remote_port, remote_socket_name,
                               m_socket_namespace, m_device_id);
    if (error.Success()) {
      if (remote_port != 0){
        m_port_forwards[pid] = {local_port, remote_port};
      }
      else{
        m_remote_socket_name[pid] ={local_port, remote_socket_name.str()};
      }
      std::ostringstream url_str;
      url_str << "connect://localhost:" << local_port;
      connect_url = url_str.str();
      break;
    }
  }

  return error;
}

lldb::ProcessSP PlatformOHOSRemoteGDBServer::ConnectProcess(
    llvm::StringRef connect_url, llvm::StringRef plugin_name,
    lldb_private::Debugger &debugger, lldb_private::Target *target,
    lldb_private::Status &error) {
  // We don't have the pid of the remote gdbserver when it isn't started by us
  // but we still want to store the list of port forwards we set up in our port
  // forward map. Generate a fake pid for these cases what won't collide with
  // any other valid pid on ohos.
  static lldb::pid_t s_remote_gdbserver_fake_pid = 0xffffffffffffffffULL;

  llvm::Optional<URI> uri = URI::Parse(connect_url);
  if (!uri) {
    error.SetErrorStringWithFormat("Invalid URL: %s",
                                   connect_url.str().c_str());
    return nullptr;
  }

  std::string new_connect_url;
  error = MakeConnectURL(s_remote_gdbserver_fake_pid--,
                         (*uri->port) ? (*uri->port) : 0, uri->path,
                         new_connect_url);
  if (error.Fail())
    return nullptr;

  return PlatformRemoteGDBServer::ConnectProcess(new_connect_url, plugin_name,
                                                 debugger, target, error);
}
