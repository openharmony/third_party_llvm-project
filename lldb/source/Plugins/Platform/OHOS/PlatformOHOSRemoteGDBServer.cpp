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
#include "llvm/Support/Regex.h"

#include "PlatformOHOSRemoteGDBServer.h"

#include <sstream>

using namespace lldb;
using namespace lldb_private;
using namespace platform_ohos;

static const lldb::pid_t g_remote_platform_pid =
    0; // Alias for the process id of lldb-platform

static uint16_t g_hdc_forward_port_offset = 0;

static Status ForwardPortWithHdc(
    const std::string &connect_addr, const uint16_t local_port,
    const uint16_t remote_port, llvm::StringRef remote_socket_name,
    const llvm::Optional<HdcClient::UnixSocketNamespace> &socket_namespace,
    std::string &device_id) {
  Log *log = GetLog(LLDBLog::Platform);

  HdcClient hdc(connect_addr);
  auto error = HdcClient::CreateByDeviceID(device_id, hdc);
  if (error.Fail())
    return error;

  device_id = hdc.GetDeviceID();
  LLDB_LOGF(log, "Connected to OHOS device \"%s\"", device_id.c_str());

  if (socket_namespace) {
    LLDB_LOGF(log, "Forwarding remote socket \"%s\" to local TCP port %d",
              remote_socket_name.str().c_str(), local_port);
    return hdc.SetPortForwarding(local_port, remote_socket_name,
                                 *socket_namespace);
  }

  LLDB_LOGF(log, "Forwarding remote TCP port %d to local TCP port %d",
            remote_port, local_port);

  if (remote_port == 0)
    return Status("Invalid remote_port");
  
  return hdc.SetPortForwarding(local_port, remote_port);
}

static Status DeleteForwardPortWithHdc(const std::string &connect_addr,
                                       std::pair<uint16_t, uint16_t> ports,
                                       const std::string &device_id) {
  Log *log = GetLog(LLDBLog::Platform);
  LLDB_LOGF(log, "Delete port forwarding %d -> %d, device=%s", ports.first,
            ports.second, device_id.c_str());

  HdcClient hdc(connect_addr, device_id);
  return hdc.DeletePortForwarding(ports);
}

static Status DeleteForwardPortWithHdc(
    const std::string &connect_addr,
    std::pair<uint16_t, std::string> remote_socket,
    const llvm::Optional<HdcClient::UnixSocketNamespace> &socket_namespace,
    const std::string &device_id) {

  Log *log = GetLog(LLDBLog::Platform);
  uint16_t local_port = remote_socket.first;
  std::string remote_socket_name = remote_socket.second;
  LLDB_LOGF(log, "Delete port forwarding %d -> %s, device=%s", local_port,
            remote_socket_name.c_str(), device_id.c_str());
  if (!socket_namespace)
    return Status("Invalid socket namespace");

  HdcClient hdc(connect_addr, device_id);
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

PlatformOHOSRemoteGDBServer::PlatformOHOSRemoteGDBServer() = default;

PlatformOHOSRemoteGDBServer::~PlatformOHOSRemoteGDBServer() {
  for (const auto &it : m_port_forwards) {
    DeleteForwardPortWithHdc(m_connect_addr, it.second, m_device_id);
  }
  for (const auto &it_socket : m_remote_socket_name) {
    DeleteForwardPortWithHdc(m_connect_addr, it_socket.second, m_socket_namespace, m_device_id);
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

bool IsValidIPv4(llvm::StringRef ip) {
  std::string pattern = "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";
  llvm::Regex regex(pattern);
  return regex.match(ip);
}

bool IsValidIPv6(llvm::StringRef ip) {
  std::string pattern = "^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$";
  llvm::Regex regex(pattern);
  return regex.match(ip);
}

bool PlatformOHOSRemoteGDBServer::IsHostnameDeviceID(llvm::StringRef hostname) {
  return hostname != "localhost" && !IsValidIPv4(hostname) &&
         !IsValidIPv6(hostname);
}

Status PlatformOHOSRemoteGDBServer::ConnectRemote(Args &args) {
  m_device_id.clear();
  m_connect_addr = "localhost";

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

  Log *log = GetLog(LLDBLog::Platform);
  if (IsHostnameDeviceID(uri->hostname)) { // accepts no (empty) hostname too
    m_device_id = uri->hostname.str();
    LLDB_LOG(log, "Treating hostname as device id: \"{0}\"", m_device_id);
  } else {
    m_connect_addr = uri->hostname.str();
    LLDB_LOG(log, "Treating hostname as remote HDC server address: \"{0}\"",
             m_connect_addr);
  }

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
    const auto error =
        DeleteForwardPortWithHdc(m_connect_addr, it->second, m_device_id);
    if (error.Fail()) {
      LLDB_LOGF(log, "Failed to delete port forwarding (pid=%" PRIu64
                ", fwd=(%d -> %d), device=%s): %s",
                pid, it->second.first, it->second.second, m_device_id.c_str(),
                error.AsCString());
    }
    m_port_forwards.erase(it);
  }
  
  if(it_socket != m_remote_socket_name.end()) {
    const auto error_Socket = DeleteForwardPortWithHdc(
        m_connect_addr, it_socket->second, m_socket_namespace, m_device_id);
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

    error =
        ForwardPortWithHdc(m_connect_addr, local_port, remote_port,
                           remote_socket_name, m_socket_namespace, m_device_id);
    if (error.Success()) {
      if (!m_socket_namespace){
        m_port_forwards[pid] = {local_port, remote_port};
      }
      else{
        m_remote_socket_name[pid] ={local_port, remote_socket_name.str()};
      }
      // Connect to local_port on a potentially remote machine with running HDC
      // server
      std::ostringstream url_str;
      url_str << "connect://" << m_connect_addr << ":" << local_port;
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

  // If m_connect_addr is remote, this connects to a remote HDC server, assuming
  // that all of the needed ports are open
  std::string new_connect_url;
  error = MakeConnectURL(s_remote_gdbserver_fake_pid--,
                         (*uri->port) ? (*uri->port) : 0, uri->path,
                         new_connect_url);
  if (error.Fail())
    return nullptr;

  return PlatformRemoteGDBServer::ConnectProcess(new_connect_url, plugin_name,
                                                 debugger, target, error);
}
