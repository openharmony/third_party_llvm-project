//===-- ThreadPlanStepOverBreakpoint.cpp ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "lldb/Target/ThreadPlanStepOverBreakpoint.h"

#include "lldb/Target/Process.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Stream.h"

using namespace lldb;
using namespace lldb_private;

// ThreadPlanStepOverBreakpoint: Single steps over a breakpoint bp_site_sp at
// the pc.
// Current behavior is to skip signal handlers, if a signal is received, so that
// signals in the background do not interrupt our stepping (otherwise a
// breakpoint can be seen as hit multiple times in a row, even though the
// underlying instruction was not executed). If a breakpoint is hit inside the
// handler (even with a false condition), the plan will finish and the user will
// see another initial breakpoint hit once the control exits the handler.

ThreadPlanStepOverBreakpoint::ThreadPlanStepOverBreakpoint(Thread &thread)
    : ThreadPlan(
          ThreadPlan::eKindStepOverBreakpoint, "Step over breakpoint trap",
          thread, eVoteNo,
          eVoteNoOpinion), // We need to report the run since this happens
                           // first in the thread plan stack when stepping over
                           // a breakpoint
      m_breakpoint_addr(LLDB_INVALID_ADDRESS),
      m_auto_continue(false), m_reenabled_breakpoint_site(false),
      m_handling_signal(false), m_is_stale(false)
{
  m_breakpoint_addr = thread.GetRegisterContext()->GetPC();
  m_breakpoint_site_id =
      thread.GetProcess()->GetBreakpointSiteList().FindIDByAddress(
          m_breakpoint_addr);
}

ThreadPlanStepOverBreakpoint::~ThreadPlanStepOverBreakpoint() = default;

void ThreadPlanStepOverBreakpoint::GetDescription(
    Stream *s, lldb::DescriptionLevel level) {
  s->Printf("Single stepping past breakpoint site %" PRIu64 " at 0x%" PRIx64,
            m_breakpoint_site_id, (uint64_t)m_breakpoint_addr);
}

bool ThreadPlanStepOverBreakpoint::ValidatePlan(Stream *error) { return true; }

bool ThreadPlanStepOverBreakpoint::DoPlanExplainsStop(Event *event_ptr) {
  StopInfoSP stop_info_sp = GetPrivateStopInfo();
  if (stop_info_sp) {
    StopReason reason = stop_info_sp->GetStopReason();

    Log *log = GetLog(LLDBLog::Step);
    LLDB_LOG(log, "Step over breakpoint stopped for reason: {0}.",
             Thread::StopReasonAsString(reason));

    switch (reason) {
      case eStopReasonTrace:
      case eStopReasonNone:
        return !m_handling_signal;
      case eStopReasonBreakpoint:
      {
        // It's a little surprising that we stop here for a breakpoint hit.
        // However, when you single step ONTO a breakpoint we still want to call
        // that a breakpoint hit, and trigger the actions, etc.  Otherwise you
        // would see the PC at the breakpoint without having triggered the
        // actions, then you'd continue, the PC wouldn't change, and you'd see
        // the breakpoint hit, which would be odd. So the lower levels fake 
        // "step onto breakpoint address" and return that as a breakpoint hit.  
        // So our trace step COULD appear as a breakpoint hit if the next 
        // instruction also contained a breakpoint.  We don't want to handle 
        // that, since we really don't know what to do with breakpoint hits.  
        // But make sure we don't set ourselves to auto-continue or we'll wrench
        // control away from the plans that can deal with this.
        // Be careful, however, as we may have "seen a breakpoint under the PC
        // because we stopped without changing the PC, in which case we do want
        // to re-claim this stop so we'll try again.
        lldb::addr_t pc_addr = GetThread().GetRegisterContext()->GetPC();

        if (pc_addr == m_breakpoint_addr) {
          // If we came from a signal handler, just reset the flag and try again.
          m_handling_signal = false;
          LLDB_LOGF(log,
                    "Got breakpoint stop reason but pc: 0x%" PRIx64
                    " hasn't changed, resetting m_handling_signal."
                    " If we came from a signal handler, trying again.",
                    pc_addr);
          return true;
        }

        if (m_handling_signal) {
          LLDB_LOG(log,
                   "Got breakpoint stop reason inside a signal handler, "
                   "step over breakpoint is finished for now.");
          // Even if we are in a signal handler, handle the breakpoint as usual
          // and finish the plan
          m_is_stale = true;
        }

        SetAutoContinue(false);
        return false;
      }
      case eStopReasonSignal:
        if (!m_handling_signal) {
          // Next stop may be a signal handler.
          LLDB_LOG(log, "Preparing for signal handler handling.");
          m_handling_signal = true;
        }
        return false;
      default:
        return false;
    }
  }
  return false;
}

bool ThreadPlanStepOverBreakpoint::ShouldStop(Event *event_ptr) {
  return !ShouldAutoContinue(event_ptr);
}

bool ThreadPlanStepOverBreakpoint::StopOthers() { return true; }

StateType ThreadPlanStepOverBreakpoint::GetPlanRunState() {
  if (m_handling_signal) {
    // Resume & wait to hit our initial breakpoint
    Log *log = GetLog(LLDBLog::Step);
    LLDB_LOG(log, "Step over breakpoint resuming through a potential signal handler.");
    return eStateRunning;
  }
  return eStateStepping;
}

bool ThreadPlanStepOverBreakpoint::DoWillResume(StateType resume_state,
                                                bool current_plan) {
  if (current_plan) {
    BreakpointSiteSP bp_site_sp(
        m_process.GetBreakpointSiteList().FindByAddress(m_breakpoint_addr));
    if (bp_site_sp) {
      Log *log = GetLog(LLDBLog::Step);
      if (m_handling_signal) {
        // Turn the breakpoint back on and wait to hit it.
        // Even if there is no userspace signal handler, we'll immediately stop
        // on the breakpoint and try again.
        LLDB_LOG(log, "Step over breakpoint reenabling breakpoint to try again after a potential signal handler");
        ReenableBreakpointSite();
      } else if (bp_site_sp->IsEnabled()) {
        LLDB_LOG(log, "Step over breakpoint disabling breakpoint.");
        m_process.DisableBreakpointSite(bp_site_sp.get());
        m_reenabled_breakpoint_site = false;
      }
    }
  }
  return true;
}

bool ThreadPlanStepOverBreakpoint::WillStop() {
  ReenableBreakpointSite();
  return true;
}

void ThreadPlanStepOverBreakpoint::DidPop() { ReenableBreakpointSite(); }

bool ThreadPlanStepOverBreakpoint::MischiefManaged() {
  lldb::addr_t pc_addr = GetThread().GetRegisterContext()->GetPC();

  if (pc_addr == m_breakpoint_addr || m_handling_signal) {
    // If we are still at the PC of our breakpoint, then for some reason we
    // didn't get a chance to run, or we received a signal and want to try again.
    return false;
  } else {
    Log *log = GetLog(LLDBLog::Step);
    LLDB_LOGF(log, "Completed step over breakpoint plan.");
    // Otherwise, re-enable the breakpoint we were stepping over, and we're
    // done.
    ReenableBreakpointSite();
    ThreadPlan::MischiefManaged();
    return true;
  }
}

void ThreadPlanStepOverBreakpoint::ReenableBreakpointSite() {
  if (!m_reenabled_breakpoint_site) {
    m_reenabled_breakpoint_site = true;
    BreakpointSiteSP bp_site_sp(
        m_process.GetBreakpointSiteList().FindByAddress(m_breakpoint_addr));
    if (bp_site_sp) {
      m_process.EnableBreakpointSite(bp_site_sp.get());
    }
  }
}
void ThreadPlanStepOverBreakpoint::ThreadDestroyed() {
  ReenableBreakpointSite();
}

void ThreadPlanStepOverBreakpoint::SetAutoContinue(bool do_it) {
  m_auto_continue = do_it;
}

bool ThreadPlanStepOverBreakpoint::ShouldAutoContinue(Event *event_ptr) {
  lldb::addr_t pc_addr = GetThread().GetRegisterContext()->GetPC();

  if (pc_addr == m_breakpoint_addr) {
    // Do not stop again at the breakpoint we are trying to step over
    Log *log = GetLog(LLDBLog::Step);
    LLDB_LOG(log, "Stopped step over breakpoint plan on its own breakpoint, auto-continue.");
    return true;
  }
  return m_auto_continue;
}

bool ThreadPlanStepOverBreakpoint::IsPlanStale() {
  if (m_handling_signal) {
    return m_is_stale;
  }
  return GetThread().GetRegisterContext()->GetPC() != m_breakpoint_addr;
}
