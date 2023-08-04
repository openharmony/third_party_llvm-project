

# 1. Introduction to LLDB


Low Lever Debugger (LLDB) is a next-generation high-performance debugger. It is developed based on [LLVM15.0.4](https://github.com/llvm/llvm-project/releases/tag/llvmorg-15.0.4) and supports debugging on the home screen and OpenHarmony devices or emulators.

> **NOTE**
>
> You can use LLDB and [OpenHarmony Device Connector (hdc)](https://gitee.com/openharmony/docs/blob/master/en/device-dev/subsystems/subsys-toolchain-hdc-guide.md) to remotely debug your application.

# 2. Obtaining LLDB
**Obtaining method:**

Obtain the OpenHarmony SDK from [Daily Builds](http://ci.openharmony.cn/workbench/cicd/dailybuild).

LLDB is in the following path of the SDK: **\ohos-sdk\\\[system]\native\llvm.

**Usage example:**

The following uses the Windows platform as an example:

- Method 1

  Obtain the SDK for Windows, place `lldb.exe`, `liblldb.dll`, and `libpython3.10.dll` in the `**\ohos-sdk\windows\native\llvm\bin` directory on a disk, and run `lldb.exe`.

- Method 2

  Decompress the SDK and run `lldb.exe` in the `**\ohos-sdk\windows\native\llvm\bin` directory.

During remote debugging, `lldb-server` and `lldb` must be used together.

**Table 1** lldb-server architecture

| Path of lldb-server                                         | Description                                       |
| ------------------------------------------------------------ | ------------------------------------------- |
| **\ohos-sdk\\[system]\native\llvm\lib\clang\\\[version]\bin\aarch64-linux-ohos\lldb-server | Applicable when the target device uses the OHOS platform and AArch64 architecture.|
| **\ohos-sdk\\\[system]\native\llvm\lib\clang\\\[version]\bin\arm-linux-ohos\lldb-server | Applicable when the target device uses the OHOS platform and ARM architecture.    |
| **\ohos-sdk\\\[system]\native\llvm\lib\clang\\\[version]\bin\x86_64-linux-ohos\lldb-server | Applicable when the target device uses the OHOS platform and x86_64 architecture. |

> **NOTE**
>
> **Table 2** Values of the system parameter
>
> | system Parameter Value | Description |
> | -------------- | ----------------------- |
> | windows | Tools for the Windows platform. |
> | linux | Tools for the Linux platform. |
> | darwin | Tools for the macOS platform. |
>
> The tools for the macOS platform is further classified into the x86_64 tools and ARM64 tools. `mac-sdk-full` and `mac-sdk-public` apply to the x86_64 architecture. `mac-sdk-m1-full` and `mac-sdk-m1-public` apply to the ARM64 architecture.
>
> The path of `lldb-server` varies according to the LLVM version. For example, for LLVM 12.0.1, the path of `lldb-server` is `*\native\llvm\lib\clang\12.0.1\bin`; for LLVM 15.0.4, the path is `*\native\llvm\lib\clang\15.0.4\bin`.


> **NOTE**
>
> - You are advised to use LLDB and hdc in the same SDK.
> - Ensure that the LLDB comes from the same version of the toolchain as the compiler used for building the target ELF file or application.

# 3. LLDB Commands
>  **NOTE**
>
> For details on original commands, see [GDB to LLDB command map](https://lldb.llvm.org/use/map.html).
>
> To obtain more commands, run the `help` command in the LLDB CLI.
>
> ```
> (lldb) help
> Debugger commands:
>   apropos           -- List debugger commands related to a word or subject.
>   breakpoint        -- Commands for operating on breakpoints (see 'help b' for shorthand.)
>   command           -- Commands for managing custom LLDB commands.
>   disassemble       -- Disassemble specified instructions in the current target.  Defaults to the current function for the current thread and stack frame.
>   expression        -- Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.
>   frame             -- Commands for selecting and examing the current thread's stack frames.
> ...
> ```
>

## 3.1 Option-related Commands

- -`v/--version`: displays the LLDB version.

  **Table 3** Command description

  |    Return Value   |               Description              |
  | :----------: | :------------------------------------: |
  | Required information| Version information. `revision` is followed by `commitID`.|

  Usage:

  ```
  lldb -v / lldb --version
  ```

- -`h/--help`: displays help information of LLDB.

  **Table 4** Command description

  |    Return Value   | Description|
  | :----------: | :--------: |
  | Required information|  Help information. |

  Usage:
  ```
  E:\ohos-sdk\windows\native\llvm\bin>lldb.exe --help
  OVERVIEW: LLDB
  
  USAGE: lldb.exe [options]
  
  ATTACHING:
    --attach-name <name> Tells the debugger to attach to a process with the given name.
    --attach-pid <pid>   Tells the debugger to attach to a process with the given pid.
    -n <value>           Alias for --attach-name
    -p <value>           Alias for --attach-pid
    --wait-for           Tells the debugger to wait for a process with the given pid or name to launch before attaching.
    -w                   Alias for --wait-for
  ...
  ```

- --`arch <architecture>/-arch <architecture>/-a <value>`: uses the specified architecture during launch and runtime.
  
  Usage:
  
  ```
  E:\ohos-sdk\windows\native\llvm\bin>lldb.exe --arch arm
  ```
  
- --`debug/-d`: prints more information for debugging.
  
  Usage:
  ```
  E:\ohos-sdk\windows\native\llvm\bin>lldb.exe --debug
  ```
  
- --`no-use-colors/-X`: does not use colors.
  
  Usage:
  ```
  E:\ohos-sdk\windows\native\llvm\bin>lldb.exe --no-use-colors
  ```

## 3.2 Logs

To facilitate fault locating, run `log <subcommand> [<command-options>]` to record LLDB logs.

- Run `help log` to display the help information about the `log` command.

  ```
  (lldb) help log
  Commands controlling LLDB internal logging.
  
  Syntax: log <subcommand> [<command-options>]
  
  The following subcommands are supported:
  
        disable -- Disable one or more log channel categories.
        enable  -- Enable logging for a single log channel.
        list    -- List the log categories for one or more log channels.  If none specified, lists them all.
        timers  -- Enable, disable, dump, and reset LLDB internal performance timers.
  
  For more help on any particular subcommand, type 'help <command> <subcommand>'.
  ```

- Run `log list` to display the list of supported logs.

  ```
  (lldb) log list
  ```

- Run the following command to record all logs in `D:\lldb.log`. In the command, `-T` means to carry the timestamp, and `-S` means to append stack backtracking to each log line.
  ```
  (lldb) log enable lldb all -T -S -f D:\lldb.log
  ```

- Run the following command to record platform events and activities in `D:\platform.log`:

  ```
  (lldb) log enable lldb platform -f D:\platform.log
  ```

- Run the following command to record module activities, such as creating, destroying, and replacing modules, in `D:\module.log`:

  ```
  (lldb) log enable lldb module -f D:\module.log
  ```

- Run the following command to record all logs of gdb-remote in `D:\gdb-remote.log`.

  ```
  (lldb) log enable gdb-remote all -f D:\gdb-remote.log
  ```

  You can use the help command to obtain other commands. For example, you can run `help log enable` to view information about the `log enable` command.

  ```
  (lldb) help log enable
  Enable logging for a single log channel.
  
  Syntax: log enable <cmd-options> <log-channel> <log-category> [<log-category> [...]]
  
  Command Options Usage:
    log enable [-FSTanpstv] [-f <filename>] <log-channel> <log-category> [<log-category> [...]]
  
         -F ( --file-function )
              Prepend the names of files and function that generate the logs.
  
         -S ( --stack )
              Append a stack backtrace to each log line.
  
         -T ( --timestamp )
              Prepend all log lines with a timestamp.
  
         -a ( --append )
              Append to the log file instead of overwriting.
  
         -f <filename> ( --file <filename> )
              Set the destination file to log to.
  
         -n ( --thread-name )
              Prepend all log lines with the thread name for the thread that generates the log line.
  
         -p ( --pid-tid )
              Prepend all log lines with the process and thread ID that generates the log line.
  
         -s ( --sequence )
              Prepend all log lines with an increasing integer sequence id.
  
         -t ( --threadsafe )
              Enable thread safe logging to avoid interweaved log lines.
  
         -v ( --verbose )
              Enable verbose logging.
  
       This command takes options and free-form arguments.  If your arguments resemble option specifiers (i.e., they start with a - or --), you must use ' -- '
       between the end of the command options and the beginning of the arguments.
  ```
## 3.3 Platforms
LLDB provides the `platform [connect|disconnect|info|list|status|select] ...` command for you to manage and create a platform.

### 3.3.1 Platform-related Information

To obtain the platform help information and commands, run the `help platform` and `apropos platform` commands.

- View the platform help information on Windows.

  ```
  (lldb) help platform
  Commands to manage and create platforms.
  
  Syntax: platform [connect|disconnect|info|list|status|select] ...
  
  The following subcommands are supported:
  
        connect        -- Select the current platform by providing a connection URL.
        disconnect     -- Disconnect from the current platform.
        file           -- Commands to access files on the current platform.
        get-file       -- Transfer a file from the remote end to the local host.
        get-size       -- Get the file size from the remote end.
        list           -- List all platforms that are available.
        mkdir          -- Make a new directory on the remote end.
        process        -- Commands to query, launch and attach to processes on the current platform.
        put-file       -- Transfer a file from this system to the remote end.
        select         -- Create a platform if needed and select it as the current platform.
        settings       -- Set settings for the current target's platform, or for a platform by name.
        shell          -- Run a shell command on the current platform.  Expects 'raw' input (see 'help raw-input'.)
        status         -- Display status for the current platform.
        target-install -- Install a target (bundle or executable file) to the remote end.
  
  For more help on any particular subcommand, type 'help <command> <subcommand>'.
  (lldb)
  ```

- View platform-related commands on Windows.

  ```
  (lldb) apropos platform
  The following commands may relate to 'platform':
    platform                -- Commands to manage and create platforms.
    platform connect        -- Select the current platform by providing a connection URL.
    platform disconnect     -- Disconnect from the current platform.
    platform file           -- Commands to access files on the current platform.
    platform list           -- List all platforms that are available.
    platform process        -- Commands to query, launch and attach to processes on the current platform.
    platform process launch -- Launch a new process on a remote platform.
    platform process list   -- List processes on a remote platform by name, pid, or many other matching attributes.
    platform select         -- Create a platform if needed and select it as the current platform.
    platform settings       -- Set settings for the current target's platform, or for a platform by name.
    platform shell          -- Run a shell command on the current platform.
    platform status         -- Display status for the current platform.
    process                 -- Commands for interacting with processes on the current platform.
  
  The following settings variables may relate to 'platform':
    target.auto-install-main-executable -- Always install the main executable when connected to a remote platform.
    target.breakpoints-use-platform-avoid-list -- Consult the platform module avoid list when setting non-module specific breakpoints.
    symbols.enable-external-lookup -- Control the use of external tools and repositories to locate symbol files. Directories listed in
                                      target.debug-file-search-paths and directory of the executable are always checked first for separate debug info files.
                                      Then depending on this setting: On macOS, Spotlight would be also used to locate a matching .dSYM bundle based on the
                                      UUID of the executable. On NetBSD, directory /usr/libdata/debug would be also searched. On platforms other than NetBSD
                                      directory /usr/lib/debug would be also searched.
    plugin.jit-loader.gdb.enable -- Enable GDB's JIT compilation interface (default: enabled on all platforms except macOS)
  ```

### 3.3.2 Viewing Available Platforms

To view information about available platforms, run the `platform list` command.

- View available platforms on Windows.

  ```
  (lldb) platform list
  Available platforms:
  host: Local Windows user platform plug-in.
  remote-linux: Remote Linux user platform plug-in.
  remote-android: Remote Android user platform plug-in.
  remote-freebsd: Remote FreeBSD user platform plug-in.
  remote-gdb-server: A platform that uses the GDB remote protocol as the communication transport.
  darwin: Darwin platform plug-in.
  remote-ios: Remote iOS platform plug-in.
  remote-macosx: Remote Mac OS X user platform plug-in.
  host: Local Mac OS X user platform plug-in.
  remote-netbsd: Remote NetBSD user platform plug-in.
  remote-openbsd: Remote OpenBSD user platform plug-in.
  qemu-user: Platform for debugging binaries under user mode qemu
  remote-windows: Remote Windows user platform plug-in.
  remote-hos: Remote HarmonyOS user platform plug-in.
  remote-ohos: Remote Open HarmonyOS user platform plug-in.
  (lldb)
  ```
  
- View available platforms on Linux.

  ```
  (lldb) platform list
  Available platforms:
  host: Local Linux user platform plug-in.
  remote-linux: Remote Linux user platform plug-in.
  remote-android: Remote Android user platform plug-in.
  remote-freebsd: Remote FreeBSD user platform plug-in.
  remote-gdb-server: A platform that uses the GDB remote protocol as the communication transport.
  darwin: Darwin platform plug-in.
  remote-ios: Remote iOS platform plug-in.
  remote-macosx: Remote Mac OS X user platform plug-in.
  host: Local Mac OS X user platform plug-in.
  remote-netbsd: Remote NetBSD user platform plug-in.
  remote-openbsd: Remote OpenBSD user platform plug-in.
  qemu-user: Platform for debugging binaries under user mode qemu
  remote-windows: Remote Windows user platform plug-in.
  remote-hos: Remote HarmonyOS user platform plug-in.
  remote-ohos: Remote Open HarmonyOS user platform plug-in.
  ```

### 3.3.3 Selecting a Platform

To select a remote platform for connection, run the `platform select <platform-name>` command.

- Select a platform.

  **Example**: Debug an application on an OHOS platform device, such as the RK3568 development board and some phones.

  ```
  platform select remote-ohos
  ```

  > **NOTE**
  >
  > If LLDB 10.0.1 is in use, `remote-hos` means that the OHOS platform is selected.

- When selecting a platform, specify the SDK root directory that contains the root directories of all remote system files.

  ```
  platform select --sysroot <filename> <platform-name>
  ```

### 3.3.4 Viewing Platform Status

To view the status of the remote platform, run the `platform status` command.

- Status information when the remote platform is not connected:

  ```
  (lldb) platform select remote-ohos
    Platform: remote-ohos
   Connected: no
   Container: no
  (lldb) platform status
    Platform: remote-ohos
   Connected: no
   Container: no
  ```
  
- Status information when the remote platform is connected:
  
  ```
  ./lldb-server platform --server --listen "*:8080"
  
  (lldb) platform select remote-ohos
    Platform: remote-ohos
   Connected: no
   Container: no
  (lldb) platform connect connect://localhost:8080
    Platform: remote-ohos
      Triple: arm-unknown-linux-unknown
    Hostname: localhost
   Connected: yes
   Container: no
  WorkingDir: /data/local/tmp
      Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
  (lldb) platform status
    Platform: remote-ohos
      Triple: arm-unknown-linux-unknown
  OS Version: 4294967295 (5.10.97)
    Hostname: localhost
   Connected: yes
   Container: no
  WorkingDir: /data/local/tmp
      Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
  (lldb)
  ```
  
### 3.3.5 Connecting to a Platform

To connect to a platform, run the `platform connect <connect-url>` command.

- Set up a connection through the port number.

  **Example**

  Step 1: Listen for a port on the LLDB server.

  ```
  # ./lldb-server platform --server --listen "*:8080"
  ```

  Step 2: Set up a connection on the LLDB client.

  ```
  (lldb) platform select remote-ohos
    Platform: remote-ohos
   Connected: no
   Container: no
  (lldb) platform connect connect://localhost:8080
    Platform: remote-ohos
      Triple: arm-unknown-linux-unknown
    Hostname: localhost
   Connected: yes
   Container: no
  WorkingDir: /data/local/tmp
      Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
  ```

  The LLDB server displays a connection success response.

  ```
  # ./lldb-server platform --server --listen "*:8080"
  Connection established.
  ```

- Set up a connection in UNIX socket mode.

  **Example**

  Step 1: Listen for a UNIX socket on the LLDB server.
  
  ```
  ./lldb-server platform --server --listen unix-abstract:///com.example.myapplication/platform-1667463465318.sock
  ```

  Step 2: Set up a connection on the LLDB client.
  
  ```
  (lldb) platform select remote-ohos
    Platform: remote-ohos
   Connected: no
   Container: no
  (lldb) platform connect unix-abstract-connect:///com.example.myapplication/platform-1667463465318.sock
    Platform: remote-ohos
      Triple: arm-unknown-linux-unknown
    Hostname: localhost
   Connected: yes
   Container: no
  WorkingDir: /data/local/tmp
      Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
  ```
  
  The LLDB server displays a connection success response.
  
  ```
  Connection established.
  ```
  
### 3.3.6 Disconnecting from a Platform
To disconnect from the current platform, run the `platform disconnect` command.

- Disconnect from the current platform.

  **Example**

  The LLDB client disconnects from the LLDB server.

  ```
  (lldb) platform disconnect
  Disconnected from "localhost"
  ```
  
  The LLDB server displays a disconnection response.
  
  ```
  error: lost connection
  lldb-server exiting..
  ```
  
### 3.3.7 Viewing Platform Processes

To view the process information on the remote platform, run the `platform process list` command.

- List the processes on the platform.
  
  ```
  (lldb) platform process list
  ```

- List the information about a specific process on the platform.

  ```
  (lldb) platform process list --pid <pid>
  (lldb) platform process list -p <pid>
  ```

## 3.4 Functions

### 3.4.1 Stepping in or Skipping a Function with Debugging Information

To step in a function that has debugging information, run the `thread step-in <cmd-options> [<thread-id>]` command. (In the command, `thread step-in` can be abbreviated as `step` or `s`.)

- Step in a function with debugging information.

  ```
  (lldb) thread step-in
  (lldb) step
  (lldb) s
  ```

To execute the next line of code without stepping in a function, run the `thread step-over <cmd-options> [<thread-id>]` command. (In the command, `thread step-over` can be abbreviated as `next` or `n`.)

- Do not step in a function and perform single-step execution at the source level.
  ```
  (lldb) thread step-over
  (lldb) next
  (lldb) n
  ```

### 3.4.2 Stepping in a Function Without Debugging Information

By default, LLDB does not step in a function without debugging information even when the `thread step-in` command is executed.

- Skip a function without debugging information. The effect is similar to `thread step-over`.

  ```
  (lldb) thread step-in
  (lldb) step
  (lldb) s
  ```

### 3.4.3 Exiting a Function Being Debugged

To finish the execution of the current stack frame and stop after returning, run the `finish <cmd-options> [<thread-id>]` command. If you want to finish tracing when debugging a function step by step, run the `finish` command. You can also use the `thread step-out <cmd-options> [<thread-id>]` command.

- Exit the function that is being debugged.

  ```
  (lldb) finish
  (lldb) thread step-out
  ```

### 3.4.4 Directly Executing a Function

To directly call a function, run the `call`, `expr`, or `print` command.

- Run `expr` to call a function.
  ```
  (lldb) expr (int) printf ("Print nine: %d.", 4 + 5)
  ```
- Run `call` to call a function.
  ```
  (lldb) call (int) printf ("Print nine: %d.", 4 + 5)
  ```
- Run `print` to call a function.
  ```
  (lldb) print (int) printf ("Print nine: %d.", 4 + 5)
  ```



## 3.5 Breakpoints

To set a breakpoint, run the `breakpoint <subcommand> [<command-options>]` or `_regexp-break` command.

### 3.5.1 Setting a Line Breakpoint

**Example**: Add a breakpoint to a line in a file.

- Method 1 (applies only when the source file is a separate file)

  ```
  (lldb) b 8
  ```

- Method 2

  ```
  (lldb) breakpoint set --file hello.cpp --line 9
  (lldb) br s -f hello.cpp -l 9
  ```

### 3.5.2 Setting a Function Breakpoint

- Set a breakpoint for a function.

  ```
  (lldb) b main
  (lldb) breakpoint set --method main
  (lldb) br s -M main
  ```

- Run `_regexp-break <module> <name>` to set breakpoints for a function in a specific library.

  ```
  (lldb) _regexp-break libc.so`malloc
  ```

### 3.5.3 Setting a Conditional Breakpoint

To set a conditional breakpoint, use the `-c` parameter. A breakpoint. A conditional breakpoint is triggered only when the condition is met.

- Set a conditional breakpoint when line 10 in `hello.cpp` is `i ==5`.

  ```
  (lldb) breakpoint set --file hello.cpp --line 10 --condition 'i==5'
  (lldb) breakpoint set -f hello.cpp -l 10 -c 'i==5'
  ```

### 3.5.4 Setting a Temporary Breakpoint

To make a breakpoint take effect only once, run the `_regexp-tbreak` (`tb` for short) command.

- Set a temporary function breakpoint.

  ```
  (lldb) _regexp-tbreak test
  ```

- Set a temporary line breakpoint.

  ```
  (lldb) _regexp-tbreak hello.cpp:10
  ```

### 3.5.5 Listing All Breakpoints

To list all breakpoints, run the `breakpoint list <cmd-options> [<breakpt-id>]` command. (In the command, `breakpoint list` can be abbreviated as `br list`.)

- List all breakpoints.

  ```
  (lldb) breakpoint list
  (lldb) br list
  ```
- List internal breakpoints.

  ```
  (lldb) breakpoint list --internal
  (lldb) br list -i
  ```
- List all breakpoints and provide a complete description of the breakpoints and their locations.

  ```
  (lldb) breakpoint list --full
  (lldb) br list -f
  ```
- Parse information about breakpoints.

  ```
  (lldb) breakpoint list --verbose
  (lldb) br list -v
  ```

### 3.5.6 Deleting Breakpoints

To delete a breakpoint, run the `breakpoint delete <cmd-options> [<breakpt-id | breakpt-id-list>]` command. (In the command, `breakpoint delete` can be abbreviated as `br delete`.)

- Delete a breakpoint.

  ```
  (lldb) breakpoint delete 2
  ```

- Delete all breakpoints

  ```
  (lldb) breakpoint delete
  ```

- Delete a line breakpoint of a file.

  ```
  (lldb) breakpoint clear -l 20 -f hello.cpp
  (lldb) breakpoint clear --line 20 --file hello.cpp
  ```

### 3.5.7 Disabling Breakpoints

To disable a breakpoint instead of deleting it, run the `breakpoint disable [<breakpt-id | breakpt-id-list>]` command. (In the command, `breakpoint disable` can be abbreviated as `br dis`).

- Disable a breakpoint.

  ```
  (lldb) breakpoint disable 1
  1 breakpoints disabled.
  ```

- Disable all breakpoints.
  ```
  (lldb) breakpoint disable
  ```

### 3.5.8 Enabling Breakpoints

To enable a breakpoint that has been disabled, run the `breakpoint enable [<breakpt-id | breakpt-id-list>]` command. (In the command, `breakpoint enable` can be abbreviated as `br en`).

- Enable a disabled breakpoint.
  ```
  (lldb) breakpoint enable 3
  1 breakpoints enabled.
  ```
- Enable all disabled breakpoints.
  ```
  (lldb) breakpoint enable
  ```

## 3.6 Watchpoints

To set a watchpoint, run the `watchpoint <subcommand> [<command-options>]` command. A watchpoint stops when a variable or expression changes.

### 3.6.1 Setting a Watchpoint

To set a watchpoint, run the `watchpoint set <subcommand> [<subcommand-options>]` command. (In the command, `watchpoint set` can be abbreviated as `wa s`.)

- Set a watchpoint for a variable.

  ```
  (lldb) watchpoint set variable global_var     
  (lldb) wa s v global_var
  ```

- Set a watchpoint for an expression.

  ```
  (lldb) watchpoint set expression -- my_ptr                            
  (lldb) wa s e -- my_ptr
  ```

### 3.6.2 Listing All Watchpoints

To list all watchpoints, run the `watchpoint list <cmd-options> [<watchpt-id | watchpt-id-list>]` command. (In the command, `watchpoint list` can be abbreviated as `watch l`).

- List all watchpoints.

  ```
  (lldb) watchpoint list
  (lldb) watch l
  ```

- Parse information about all watchpoints.
  ```
  (lldb) watchpoint list -v
  (lldb) watch l -v
  ```
- Lists the information about the locations of all watchpoints.
  ```
  (lldb) watchpoint list -f
  (lldb) watch l -f
  ```
### 3.6.3 Deleting Watchpoints

To delete a watchpoint, run the `watchpoint delete <cmd-options> [<watchpt-id | watchpt-id-list>]` command. (In the command, `watchpoint delete` can be abbreviated as `watch del`.)

- Delete a watchpoint.
  ```
  (lldb) watchpoint delete 2                                             
  1 watchpoints deleted.
  ```

- Delete all watchpoints.
  ```
  (lldb) watchpoint delete
  About to delete all watchpoints, do you want to do that?: [Y/n] Y
  All watchpoints removed. (2 watchpoints)
  ```

### 3.6.4 Disabling Watchpoints

To disable a watchpoint, run the `watchpoint disable` (`watch dis` for short) command.

- Disable a watchpoint.
  ```
  (lldb) watchpoint disable 2                                             
  1 watchpoints disabled.
  ```
- Disable all watchpoints.

  ```
  (lldb) watchpoint disable                                             
  (lldb) watch dis
  ```

### 3.6.5 Enabling Watchpoints

To enable a watchpoint that has been disabled, run the `watchpoint enable` (`watch en` for short) command.

- Enable a watchpoint.

  ```
  (lldb) watchpoint enable 2                                            
  1 watchpoints enabled.
  ```

- Enable all watchpoints.

  ```
  (lldb) watchpoint enable  
  (lldb) watch en
  ```

## 3.7 Expressions

To evaluate an expression, run the `expression <cmd-options> -- <expr>` command. (In the command, `expression` can be abbreviated as `expr`.)

- Create and assign a value to a convenience variable.
  ```
  (lldb) print int $value1 = 7
  (lldb) expression int $value2 = 7
  ```

- Print a variable value.
  ```
  (lldb) print $value1
  (lldb) expression $value2
  ```

- Print a global variable global_var.
  ```
  (lldb) expression global_var
  ```
- Calculate a variable.
  ```
  (lldb) expression global_var * 3
  ```

- Print a variable (decimal by default).

  ```
  (lldb) print value
  (lldb) expression value
  (lldb) p value
  ```

- Print a variable (hexadecimal).

  ```
  (lldb) print/x value
  (lldb) expression/x value
  (lldb) p/x value
  ```

- Print a variable (octal).

  ```
  (lldb) print/o value
  (lldb) expression/o value
  (lldb) p/o value
  ```

- Print a variable (binary).

  ```
  (lldb) print/t value
  (lldb) expression/t value
  (lldb) p/t value
  ```

## 3.8 Viewing Variables

To view variable information, run the `frame variable <cmd-options> [<variable-name> [<variable-name> [...]]]` and `target variable <cmd-options> <variable-name> [<variable-name> [...]]` commands.

- Show the arguments and local variables of the current frame.

  ```
  (lldb) frame variable
  (lldb) fr v
  ```

- Show the global variables and static variables of the source file of the current frame.

  ```
  (lldb) frame variable -g
  (lldb) fr v -g
  ```

- Show the global variables of the current target before or during the running of the process.

  ```
  (lldb) target variable
  (lldb) ta v
  ```

- Show the global variables of a file.

  ```
  (lldb) target variable --file hello.cpp
  (lldb) ta v --file hello.cpp
  ```

## 3.9 Processes and Threads

### 3.9.1 Stack Backtrace

To show the stack backtrace for the current thread, run the `thread backtrace <cmd-options>` command. (In the command, `thread backtrace` can be abbreviated as `bt`.)

- Show the stack backtrace for the current thread.
  ```
  (lldb) thread backtrace
  (lldb) bt
  ```
- Backtrace the first two frames of the current thread.
  ```
  (lldb) thread backtrace -c 2
  ```
- Show the stack backtraces for all threads.
  ```
  (lldb) thread backtrace all
  (lldb) bt all
  ```

- Exit the current stack frame.

  ```
  (lldb) thread return
  ```

### 3.9.2 Showing Threads in the Current Target Process

To show the threads in the current target process, run the `thread list` command.
- List the threads in your program.
  ``` 
  (lldb) thread list
  ```

- Show the thread information.

  ```
  (lldb) thread info
  ```

### 3.9.3 Stack Frame in a Thread

To select a stack frame in the current thread, run the `frame select <cmd-options> [<frame-index>]` command.

- Select the stack frame whose index is 2 in the current thread.

  ```
  (lldb) frame select 2
  ```

- Select the stack frame that calls the current stack frame.

  ```
  (lldb) up
  (lldb) frame select --relative=1
  ```

- Select the stack frame called by the current stack frame.

  ```
  (lldb) down
  (lldb) frame select --relative=-1
  (lldb) fr s -r-1
  ```

- List information about the currently selected frame in the current thread.

  ```
  (lldb) frame info
  ```


- Show the current frame and source code.

  ```
  (lldb) frame select
  (lldb) f
  (lldb) process status
  ```

### 3.9.4 Registers

To access the register of the current thread and stack frame, run the `register [read|write] ...` command.

- Show the general purpose registers for the current thread.

  ```
   (lldb) register read
  ```

- Change the register value.

  ```
   (lldb) register write rax 0x0000000000401470
  ```

## 3.10 Executable Files and Shared Libraries

If the search path of the executable file is different from that of the local file system, you can run the `settings append target.exec-search-paths` command to set the search path.

- Set `E:\DevEcoStudioProjects\MyApplication\entry\build\default\intermediates\cmake\default\obj\armeabi-v7a`, the path where `libentry.so` and `libc++_shared.so` are stored, to the search path.

  ```
  (lldb) settings append target.exec-search-paths "E:\DevEcoStudioProjects\MyApplication\entry\build\default\intermediates\cmake\default\obj\armeabi-v7a"
  ```

- Add a symbol file.

  ```
  (lldb) target symbols add a.out
  ```

To access information about one or more target modules, run the `target modules <sub-command>` or `target` command.

- List information about the current executable file and related shared libraries.

  ```
  (lldb) target modules list
  (lldb) image list
  ```
  
- Search for functions or symbols in the executable files and related shared libraries.

  ```
  (lldb) target modules lookup --name main
  (lldb) image lookup -n main
  ```

- Search for address information in the `a.out` file.

  ```
  (lldb) target modules lookup --address 0x4012df a.out
  (lldb) image lookup -a 0x4012df a.out
  ```


## 3.11 Source Files

To obtain source code information, run the `source <subcommand> [<subcommand-options>]` command.

- Display source line information for the current target process.
  ```
  (lldb) source info
  ```
  
- Display source line information for the current target process about a file.
  ```
  (lldb) source info -f hello.cpp
  ```
  
- Display source code for the current target process.
  ```
  (lldb) source list
  ```

- List the source code.
  ```
  (lldb) _regexp-list
  (lldb) list
  ```
  

If the source files are no longer in the same location as when the program was built (for example, if the program was built on a different computer), you need to tell the debugger how to find the sources at their local file path instead of the build system's file path. To do so, run the `settings set target.source-map /buildbot/path /my/path` command to remap the source file path names for the debug session.

- If the source code path of the executable file is `D:\demo`, but the source code directory is moved to `E:\demo`, remap the source file path names for debugging.

  ```
  (lldb) settings set target.source-map D:\demo E:\demo
  ```

## 3.12 Disassembly

Run the `disassemble [<cmd-options>]` command for disassembling specified instructions. (In the command, `disassemble` can be abbreviated as `dis` or `di`.)

- Disassemble the current function for the current frame.
  ```
  (lldb) disassemble --frame
  (lldb) dis -f
  (lldb) di -f
  ```
- Disassemble any functions named `main`.
  ```
  (lldb) disassemble --name main
  (lldb) dis -n main
  (lldb) di -n main
  ```
- Show a specific number of disassembly instructions.
  ```
  (lldb) disassemble --count 10
  (lldb) dis -c 10
  (lldb) di -c 10
  ```
- Disassemble the current source line for the current frame.
  ```
  (lldb) disassemble --line
  (lldb) dis -l
  (lldb) di -l
  ```

## 3.13 Changing the Program Execution

### 3.13.1 Jumping to a Specific Location

To go to a specific location, run the `jump` command.

- Go to line 27 of the current function.

  ```
  (lldb) jump 27
  ```
  > **NOTE**
  >
  > Exercise caution when running the `jump` command, which may cause unexpected results.
  >
  > The `jump` command cannot enable the program to jump to another function.
  >

### 3.13.2 Changing a Register Value

The program counter (PC) register stores the next instruction to be executed by the program. You can change the value of this register to change the program execution process.

- Change the register value.

  ```
  (lldb) register write pc `$pc+8`
  ```

## 3.14 Signals

To manage the policy that LLDB uses to process the operating system signals of the current directory process, run the `process handle <cmd-options> [<unix-signal> [<unix-signal> [...]]]` command.

- Show the LLDB handling policy of OS signals for the current target process.

  ```
  (lldb) process handle
  ```

- Set a policy for handling the OS signal SIGRTMIN+1 for the current target process.

  ```
  (lldb) process handle -p false -s false -n false SIGRTMIN+1
  ```

- Set a policy for handling the OS signal SIGRTMIN+1 for the current target process.

  ```
  (lldb) process handle -p true -s true -n false SIGRTMIN+1
  ```
  > **NOTE**
  >
  > `-n <boolean> (--notify <boolean>)`: specifies whether the debugger should notify the user if a signal is received. The value `true` means that the debugger should notify the user, and `false` means the opposite.
  >
  > `-p <boolean> (--pass <boolean>)`: specifies whether the signal should be passed to the process. The value `true` means that the value should be passed to the process, and `false` means the opposite.
  >
  > `-s <boolean> (--stop <boolean>)`: specifies whether the debugger should stop processing if a signal is received. The value `true` means that the debugger should stop processing, and `false` means the opposite.

## 3.15 Core Dump

To generate a core dump file, run the `process save-core [-s corefile-style -p plugin-name] FILE` command. The file records the status of the current process for future analysis.

- Generate the `core.dump` file for the current process.

  ```
  (lldb) process save-core --plugin-name=minidump --style=stack core.dump
  ```

- Use the `core.dump` file to locate the crash.

  ```
  lldb --core /path/to/core
  lldb <project-name> -c /path/to/core
  ```

## 3.16 Others

### 3.16.1 Searching by Keyword

To search for debugger commands by keyword, run the `apropos <search-word>` command.

- List debugger commands related to the platform.

  ```
  (lldb) apropos platform
  ```

- List debugger commands related to info.

  ```
  (lldb) apropos info
  ```

### 3.16.2 Command Aliases

LLDB supports creation of aliases for common commands. To create a command alias, run the `command alias <cmd-options> -- <alias-name> <cmd-name> [<options-for-aliased-command>]` command.

- Alias of the command for creating a breakpoint
  Original command
  ```
  (lldb) breakpoint set --file hello.cpp --line 12
  ```
  Alias
  ```
  (lldb) command alias bfl breakpoint set --file hello.cpp --line %2
  ```

### 3.16.3 Configurations

- Show matching debugger settings and their current values.

  ```
  (lldb) settings show
  ```

- Show the environment information being passed to the process when launched.

  ```
  (lldb) target show-launch-environment
  ```
- Set the environment variables and their values to be passed to the executable file.
  ```
  (lldb) settings set target.env-vars MYPATH=~/.:/usr/bin SOME_ENV_VAR=12345 
  ```
- View the environment variables and their values passed to the executable file.
  ```
  (lldb) settings show target.env-vars
  (lldb) _regexp-env
  ```
- Set whether to inherit environment information from the running process. The value `true` means to inherit the information, and `false` means the opposite.
  ```
  (lldb) settings set target.inherit-env true
  ```
- View the environment information inheritance policy.
  ```
  (lldb) settings show target.inherit-env
  ```
- Obtain more debugger commands related to the environment.

  ```
  (lldb) apropos environment
  ```

- Set the module cache directory of the platform.

  ```
  (lldb) setting set platform.module-cache-directory "D:\.lldb\module_cache"
  ```

- View the module cache directory of the platform.

  The default module cache directory is `C:\Users\UserName\\.lldb\module_cache` for Windows.

  ```
  (lldb) setting show platform.module-cache-directory
  ```

- Set whether to enable module cache of the platform. The value `true` means to enable module cache, and `false` means the opposite.

  ```
  setting set platform.use-module-cache true
  ```

> **NOTE**
>
> When module cache is enabled, the `setting set platform.module-cache-directory <value>` command is valid.
>
> If the version of the debugging device changes, you are advised to clear the historical module cache.

### 3.16.4 Historical Commands

To process historical commands in a session, run the `history <cmd-options>` command. You can use `"! <INDEX>"` to run the historical commands again.

- View historical commands.

  ```
  (lldb) session history
  (lldb) history
  ```

- Clear historical commands.

  ```
  (lldb) session history --clear
  (lldb) history --clear
  (lldb) history -C
  ```

- List historical commands whose start index is 2 and end index is 3.

  ```
  (lldb) session history --start-index 2 --end-index 3
  (lldb) history -s 2 -e 3
  ```

### 3.16.5 Autocomplete

In LLDB, you can enter the start character or characters of a command and press `Tab` to show all possible choices.

> **NOTE**
>
> - The automcomplete feature is available only when the `--build-libedit` option is added during the build of the LLVM project. You are advised to use this feature together with `--build-ncurses`.
> - The autocomplete features applies to Linux, macOS x86_64, and M1.

- Enter `pl` and press `Tab` to show all possible choices. (There are multiple commands starting with `pl` in LLDB.)

  ```
  (lldb) pl
  Available completions:
          platform -- Commands to manage and create platforms.
          plugin   -- Commands for managing LLDB plugins.
  (lldb) pl
  ```

- Enter `his` and press `Tab`. The `history` command is autocompleted. (There is only one command starting with `his` in LLDB.)

  ```
  (lldb) history
     0: history
  ```

### 3.16.6 GUI

To enter the graphical debugging interface, run the `gui` command.

> **NOTE**
>
> - The GUI feature is available only when the `--build-ncurses` option is added during the build of the LLVM project.
>
> - The GUI feature applies to Linux, macOS x86_64, and M1.

- Enter the graphical debugging interface.

  ```
  (lldb) gui
  ```

# 4. Remote Debugging on the Windows Platform

## 4.1 Debugging Executable Files

**Scenario 1**: Customize source code. Use `target create <cmd-options> <filename>` or `file <cmd-options> <filename>` to specify the executable file (built by using the SDK) to be debugged.

**Example**

Device side

Run `lldb-server`.

```
./lldb-server platform --server --listen "*:8080"
```

LLDB client

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
Platform: remote-ohos
Connected: no
Container: no
(lldb) platform connect connect://localhost:8080
Platform: remote-ohos
  Triple: arm-unknown-linux-unknown
Hostname: localhost
Connected: yes
Container: no
...
(lldb) target create <cmd-options> <filename>                
...
(lldb) breakpoint set --name main
...
(lldb) platform disconnect                                            
Disconnected from "localhost"
(lldb) quit   
```

**Scenario 2**: Customize source code. Debug the executable file (built by the SDK) in attach mode.

**Example**

Device side

1)Run the executable file and run the `ps` command to view the PID of the process.

```
./process-name
```

2)Run `lldb-server`.

```
./lldb-server p --server --listen unix-abstract:///com.example.myapplication/platform-1648104534646.sock
```

LLDB client

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
  Platform: remote-ohos
 Connected: no
 Container: no
(lldb) platform connect unix-abstract-connect:///com.example.myapplication/platform-1648104534646.sock
  Platform: remote-ohos
    Triple: arm-unknown-linux-unknown
  Hostname: localhost
 Connected: yes
 Container: no
WorkingDir: /data/local/tmp
    Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
(lldb) breakpoint set --file <filename> --line <linenum>
...
(lldb) process attach --name process-name         // Or 'attach <pid> | <process-name>'
...
(lldb) process detach                             // Or 'detach'
Process ... detached
(lldb) platform disconnect                                            
  Disconnected from "localhost"
(lldb) quit                                        // Or 'q'
```
> **NOTE**
>
> For details on how to use the SDK to build the source code, see [HOW TO USE NDK (windows)](https://gitee.com/openharmony/third_party_musl/wikis/HOW%20TO%20USE%20NDK%20(windows)) or [HOW TO USE NDK (linux)](https://gitee.com/openharmony/third_party_musl/wikis/HOW%20TO%20USE%20NDK%20(linux)). You can also use the Clang or Clang++ compiler in the SDK to build the source code.
>
> For example, to build the executable file for the OHOS platform and ARM architecture, use `<clang distribution> /bin/clang++ --target=arm-linux-ohos--gcc-toolchain=<toolchain distribution> --sysroot=<sysroot distribution> -O0 -g <file> -o <outfile>`.
>
> If the source files are no longer in the same location as when the program was built (for example, if the program was built on a different computer), you need to tell the debugger how to find the sources at their local file path instead of the build system's file path. To do so, run the `settings set target.source-map /buildbot/path /my/path` command to remap the source file path names for the debug session.
>
> In the preceding command, `/buildbot/path` is the build path of the source code, and `/my/path` is the path where the source code is stored currently.
>
> It is assumed that the source codes are stored in D:\demo\hello_world.cpp during the build. To debug the program on another Windows host where the source code is stored in `F:\Test\hello_world.cpp`, run the `settings set target.source-map "D:\demo" "F:\Test"` command.
>
> If the option `OHOS_STL=c++_shared` is used and the debugging device uses the 32-bit ARM architecture, upload `libc++_shared.so` to the `/system/lib` directory. If the device uses the AArch64 architecture, upload `libc++_shared.so` to the `/system/lib64` directory. If the option `OHOS_STL=c++_static` is used, you do not need to upload `libc++_shared.so`.
> 
> `libc++_shared.so` is in the `native` directory of the SDK.
> 
> If the message "[Fail]Error opening file: read-only file system..." is displayed when you upload `libc++_shared.so`, run the `hdc.exe shell mount -o rw,remount /` command.

## 4.2 Debugging HAPs

**Scenario 1**: Build a Debug HAP on DevEco Studio.

**Example**

Device side

1)Install and run the HAP, and run the `ps` command to view the PID of the application.

```
hdc shell bm install -p <file-path>
hdc shell aa start  -a <ability-name> -b <bundle-name> && hdc shell pidof <bundle-name>
```

2)Run `lldb-server`.

```
./lldb-server platform --server --listen "*:8080"
```

LLDB client

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
  Platform: remote-ohos
 Connected: no
 Container: no
(lldb) platform connect connect://localhost:8080
  Platform: remote-ohos
    Triple: arm-unknown-linux-unknown
  Hostname: localhost
 Connected: yes
 Container: no
WorkingDir: /data/local/tmp
    Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
(lldb) settings append target.exec-search-paths <value>
(lldb) breakpoint set --file <filename> --line <linenum>
...
(lldb) process attach --pid <pid>                // Or 'attach <pid> | <process-name>'
...
(lldb) continue
...
(lldb) process detach                           // Or 'detach'
(lldb) platform disconnect                                            
  Disconnected from "localhost"
(lldb) quit                                     // Or 'q'
```

**Scenario 2**: Start application debugging.

**Example**

Device side

1)Install and run the Debug HAP, and check the PID of the application.

```
hdc shell bm install -p <file-path>
hdc shell aa start -a <ability-name> -b <bundle-name> -N && hdc shell pidof <bundle-name>
```

> **NOTE**
>
> The `-N` parameter in the ability assistant indicates that the application starts debugging. The application stops in the appspawn phase and waits for the debugger to connect.
>
> For details about how to use the ability assistant, see [Ability Assistant](https://gitee.com/openharmony/docs/blob/master/en/application-dev/tools/aa-tool.md).

2)Run `lldb-server`.

```
./lldb-server platform --server --listen "*:8080"
```

LLDB client

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
  Platform: remote-ohos
 Connected: no
 Container: no
(lldb) platform connect connect://localhost:8080
  Platform: remote-ohos
    Triple: arm-unknown-linux-unknown
  Hostname: localhost
 Connected: yes
 Container: no
WorkingDir: /data/local/tmp
    Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
(lldb) breakpoint set --file <filename> --line <linenum>
...
(lldb) process attach --pid <pid>                // Or 'attach <pid> | <process-name>'
...
(lldb) continue
...
(lldb) process detach                           // Or 'detach'
(lldb) platform disconnect                                            
  Disconnected from "localhost"
(lldb) quit                                     // Or 'q'
```

> **NOTE**
>
> You can refer to this section to debug the .so file of the debug version.

## 4.3 Debugging OpenHarmony Services

> **NOTE**
>
> This type of debugging can be used in the root version, but not the user version.

> **NOTE**
>
> Run the `ps -elf` command. The process with PPID 1 is the system ability service.

**Scenario 1**: Debug a single system ability.

**Example**: Debug netmanager.

Device side

1)Set the service to debug and obtain its PID.

- Enter the interactive command environment.

  ```
  hdc shell
  ```

- Specify the name of the service to debug, which is netmanager in this example.

  ```
  # param set llvm.debug.service.name netmanager
  Set parameter llvm.debug.service.name netmanager success
  ```

- Stop the service.

  ````
  service_control stop netmanager
  ````

- Start the service so that the service process stops in the initialization phase and waits for the debugger to connect.

  ```
  service_control start netmanager
  ```

- Obtain the PID of the service.

  ```
  # ps -elf | grep init
  root             1     0 0 17:02:55 ?     00:00:03 init --second-stage
  root          1720     1 0 17:19:32 ?     00:00:00 init --second-stage
  root          1723  1624 5 17:19:39 pts/0 00:00:00 grep init
  ```

  Process 1 is the init process, and process 1720 is the one to be debugged.

2)Run `lldb-server`.

```
./lldb-server platform --server --listen "*:8080"
```

LLDB client

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
  Platform: remote-ohos
 Connected: no
 Container: no
(lldb) platform connect connect://localhost:8080
  Platform: remote-ohos
    Triple: arm-unknown-linux-unknown
  Hostname: localhost
 Connected: yes
 Container: no
WorkingDir: /data/local/tmp
    Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
(lldb) breakpoint set --file <filename> --line <linenum>
...
(lldb) process attach --pid <pid>                // Or 'attach <pid>'
...
(lldb) continue
...
(lldb) process detach                           // Or 'detach'
(lldb) platform disconnect                                            
  Disconnected from "localhost"
(lldb) quit                                     // Or 'q'
```

**Scenario 2**: Debug multiple system abilities.

**Example**

Device side

1)Set the services to debug and obtain their PID.

- Enter the interactive command environment.

  ```
  hdc shell
  ```

- Specify the services to debug.

  ```
  # param set llvm.debug.service.all 1
  Set parameter llvm.debug.service.all 1 success
  ```

- Stop service 1 to be debugged.

  ````
  service_control stop <servicename1>
  ````

- Start service 1 so that the service process stops in the initialization phase and waits for the debugger to connect.

  ```
  service_control start <servicename1>
  ```

- Obtain the PID of service 1.

  ```
  ps -elf | grep init
  ```

2)Run `lldb-server`.

```
./lldb-server platform --server --listen "*:8080"
```

LLDB client 1

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
  Platform: remote-ohos
 Connected: no
 Container: no
(lldb) platform connect connect://localhost:8080
  Platform: remote-ohos
    Triple: arm-unknown-linux-unknown
  Hostname: localhost
 Connected: yes
 Container: no
WorkingDir: /data/local/tmp
    Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
(lldb) breakpoint set --file <filename> --line <linenum>
...
(lldb) process attach --pid <pid1>                // Or 'attach <pid1>'
...
(lldb) continue
...
(lldb) process detach                           // Or 'detach'
(lldb) platform disconnect                                            
  Disconnected from "localhost"
(lldb) quit                                     // Or 'q'
```

Device side

Stop service 2 to be debugged.

```
service_control stop <servicename2>
```

Start service 2 so that the service process stops in the initialization phase and waits for the debugger to connect.

```
service_control start <servicename2>
```

Obtain the PID of service 2.

```
ps -elf | grep init
```

LLDB client 2

1)Run `lldb.exe`.

2)Debug the connection in the LLDB CLI.

```
(lldb) platform select remote-ohos
  Platform: remote-ohos
 Connected: no
 Container: no
(lldb) platform connect connect://localhost:8080
  Platform: remote-ohos
    Triple: arm-unknown-linux-unknown
  Hostname: localhost
 Connected: yes
 Container: no
WorkingDir: /data/local/tmp
    Kernel: #1 SMP Fri Mar 31 20:52:12 CST 2023
(lldb) breakpoint set --file <filename> --line <linenum>
...
(lldb) process attach --pid <pid2>                // Or 'attach <pid2>'
...
(lldb) continue
...
(lldb) process detach                           // Or 'detach'
(lldb) platform disconnect                                            
  Disconnected from "localhost"
(lldb) quit                                     // Or 'q'
```

The procedure for debugging multiple services is the same as that for debugging two services. You only need to open multiple LLDB CLIs.

# 5 Auxiliary Script for Command Line Debugging

## 5.1 Running Python Code

LLDB supports the running of Python code. Currently, this feature is supported by the NDK of Linux, Windows, and macOS.

1. Run a single line of Python code.

```
script print("Hello OpenHarmony!")
```

2. Run the `script` command to enter the Python shell and execute the Python code.

```python
import os
env_path_value = os.environ.get("PATH", "")
print(f"PATH='{env_path_value}'")
```

## 5.2 Using the Auxiliary Script to Connect to and Start `lldb-server`

To use the LLDB command lines of OpenHarmony NDK for remote debugging, perform the following steps:

1. Push `lldb-server` to the OpenHarmony device.

2. Start `lldb-server` on the OpenHarmony device.

3. Run `lldb` on the computer and connect it to `lldb-server`.

4. Attach the running process or start the CLI, and then run the debugging commands.

OpenHarmony NDK provides auxiliary scripts to simplify the debugging process. You can start and connect to `lldb-server` by entering the following commands:

```
script import lldb_python
script lldb_python.main()
```

### 5.2.1 Viewing Parameters

Check the `lldb-server` startup and connection parameters used by the script.

```
script lldb_python.show_server_configs()
```

### 5.2.2 Modifying Parameters

Modify the startup and connection parameters.

```
script lldb_python.set_server_config("tcp-listen-port", 8080)
```

The following parameters are supported:

| Name| Default Value| Description|
| --- | --- | --- |
| `arch` | `unknown` | The value can be `arm`, `aarch64`, or `x86_64`. `arm` indicates the ARM 32-bit architecture, `aarch64` indicates the ARM 64-bit architecture, and `x86_64` indicates the x86 64-bit architecture. Select `lldb-server` based on the hardware architecture. If this parameter is set to `unknown`, the system attempts to determine the device architecture. If an error occurs during automatic architecture determination, you can manually set this parameter to another value.|
| `install-path` | `/data/local/tmp/lldb/lldb-server` | Installation path of `lldb-server` on the OpenHarmony device.|
| `tcp-listen-port` | `1234` | TCP port number listened by `lldb-server`.|
| `platform` | `remote-ohos` | Platform where `lldb-server` is located.|


# 6. Standalone Debugging

Unlike remote debugging, standalone debugging enables you to perform debugging on a device without configuring a remote connection.

## 6.1 Obtaining LLDB Standalone Tools

To obtain LLDB Standalone tools, add the **--build-lldb-static** option when building the LLVM project.

For details about how to build the LLVM project, see [LLVM Build](https://gitee.com/openharmony/third_party_llvm-project/blob/master/llvm-build/README.md).

After the build is complete, you can obtain LLDB Standalone tools from the following path:

1)Generation path: llvm-project/out/lib/lldb-server-[platform]-linux-ohos/bin

2)Installation path: llvm-project/out/llvm-install/lib/clang/15.0.4/bin/[platform]-linux-ohos

3)Package: clang-dev/lib/clang/15.0.4/bin/[platform]-linux-ohos in llvm-project/out/clang-dev-linux-x86_64.tar.bz2

​	The value of `[platform]` depends on the architecture of the debugging device. The value can be `arm` or `aarch64`.

## 6.2 Preparing for Standalone Debugging

1)Use hdc to transfer the `lldb` and `lldb-server` files obtained to the working path of the device, for example, `/data/local/tmp`.

```
hdc.exe file send lldb /data/local/tmp
hdc.exe file send lldb-server /data/local/tmp
```

2)Enter the interactive command mode.

```
hdc.exe shell
```

3)Grant the execute permission on LLDB.

```
# chmod +x /data/local/tmp/lldb
# chmod +x /data/local/tmp/lldb-server
```
## 6.3 Performing Standalone Debugging

Generally, you can start debugging by attaching to a running process or directly starting a program through LLDB. Then you can use [LLDB Commands](#lldb-commands) to debug the target program.

> **NOTE**
>
> - The `print`, `call`, and `expr` commands do not support function invoking.
> - If the `Backspace` key cannot be used to delete a file, use `Ctrl+Backspace` instead.

### 6.3.1 Attaching to a Running Process
#### 6.3.1.1 Starting LLDB

```
# /data/local/tmp/lldb
(lldb)
```

Run the `ps` or `top` command to obtain the PID or process name of the target program.

#### 6.3.1.2 Attaching to a Process by ID

```
(lldb) attach -p <pid>
```

#### 6.3.1.3 Attaching to a Process by Name

```
(lldb) process attach --name <name>
```

### 6.3.2 Starting a Program Through LLDB

#### 6.3.2.1 Specifying the Target Program hello_world when Starting LLDB

```
# /data/local/tmp/lldb /data/local/tmp/hello_world
```

#### 6.3.2.2 Running `target create` to Specify the Target Program hello_world

```
# /data/local/tmp/lldb
(lldb) target create /data/local/tmp/hello_world
```

#### 6.3.2.3 Setting a Breakpoint and Starting the Target Program

```
(lldb) b main
(lldb) r
```
