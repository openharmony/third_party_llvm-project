add_lldb_library(lldbPluginPlatformHOS PLUGIN HdcClient.cpp PlatformHOS.cpp
	                 PlatformHOSRemoteGDBServer.cpp LINK_LIBS lldbCore
                         lldbHost lldbPluginPlatformLinux lldbPluginPlatformGDB
                             LINK_COMPONENTS Support)
