"""
Test lldb attach hap package.This test case only runs when the local ubuntu is connected to 
the phone or other device and there is a remote-ohos connection,will skip on CI or remote = false.
1.must to build a native c++ project with bundle name:com.example.myapplication
2.replace lldb-test.hap with the project-built hap package name
3.the hello.cpp file of this project must include a method called stringFun() in the Add method
4.must call the defined method on line 154 of hello.cpp
"""


import subprocess
import sys

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil


class AttachHapPackage(TestBase):

    mydir = TestBase.compute_mydir(__file__)
    NO_DEBUG_INFO_TESTCASE = True
    
    @skipOnOpenHarmonyCI
    @skipIf(remote = False)
    def test_attach_hap_by_pid(self):
       
       # get the hap program path
       script_path = os.path.dirname(os.path.abspath(__file__))
       
       # run the hap applicaton
       cmd_unins = "hdc uninstall com.myapplication"
       subprocess.check_output(cmd_unins, shell = True)

       cmd_ins = "hdc install -r " + script_path + "/lldb-test.hap"
       subprocess.check_output(cmd_ins, shell = True)
       cmd_start = "hdc shell aa start -a EntryAbility -b com.myapplication"
       subprocess.check_output(cmd_start, shell = True)
       
       # source path map 
       # set target.exec-search-paths
       target_path = script_path
       self.runCmd("settings append target.exec-search-paths " + target_path)
       
       # add breakpoint at function by name
       self.runCmd("breakpoint set --name stringFunc")
       self.expect("breakpoint list -f", substrs = ['stringFunc'])
       self.runCmd("breakpoint delete 1")

       # add br in source code , and show the br list
       self.runCmd("breakpoint set --file hello.cpp --line 154")
       self.expect("breakpoint list -f", substrs = ['line = 154'])
           
       # get the pid of hap application
       cmd_pid = "hdc shell ps -elf | grep com.myapplication"
       res_pid = subprocess.check_output(cmd_pid, shell = True)
       str_pid = res_pid.decode(encoding="utf-8", errors="strict")
       pid = str_pid.split('     ',2)[1].split('   ',2)[0]
              
       #attach by pid
       self.expect("process attach -p " + pid, substrs=["appspawn"])
       self.expect("continue", substrs=["stop reason = breakpoint 1"])
       
       # mock clicking the screen events
       cmd_click = "hdc shell uinput -T -c 390 620"
       subprocess.check_output(cmd_click, shell = True)
       
       self.runCmd("frame info")
       self.runCmd("thread step-in")
       
