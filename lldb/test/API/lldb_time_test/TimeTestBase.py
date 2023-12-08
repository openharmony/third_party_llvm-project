# Copyright (C) 2023 Huawei Device Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from lldbsuite.test.lldbtest import TestBase
import shutil
import json
import lldb
import csv
import os


class LogTagTime:
    def __init__(self, id=None, value=0, expect=None, msg=None):
        self.id = id
        self.value = value
        self.expect = expect
        self.msg = msg


class TimeTestBase(TestBase):
    benchmark = None
    force_remove_log = True
    dump_timers_log = None
    report_dir = None
    results = list()

    def setUp(self):
        # Remove cache
        run_with_cache = eval(os.environ.get("RUN_WITH_CACHE", "TRUE"))
        cache = os.path.join(os.path.expanduser("~"), ".lldb", "module_cache")
        if os.path.exists(cache) and not run_with_cache:
            lldb.SBModule.GarbageCollectAllocatedModules()
            shutil.rmtree(cache, True)
        TestBase.setUp(self)

        self.log = "%s.log" % self._testMethodName
        self.report = "%s_report.csv" % self._testMethodName

        # Set log and report
        self.set_log_and_report()
        # Log enable lldb performance -T
        self.dbg.EnableLog("lldb", ["performance"])

    def tearDown(self):
        # Generate report based on benchmark JSON file
        self.output_report(self.benchmark)

        # Remove log file
        if os.path.exists(self.log) and self.force_remove_log:
            os.remove(self.log)
        TestBase.tearDown(self)

    def set_log_and_report(self):
        # Set log and report\
        default_report_dir = os.path.join(os.getcwd(), "report")
        self.report = os.path.join(default_report_dir, self.report)
        self.log = os.path.join(default_report_dir, self.log)
        if not os.path.exists(default_report_dir):
            os.mkdir(default_report_dir)
        self.reset_time_test_suite()
        if os.path.exists(self.log):
            os.remove(self.log)
        self.log_file_write = open(self.log, "w")
        sbf = lldb.SBFile(self.log_file_write.fileno(), "w", False)
        self.dbg.SetOutputFile(sbf)

    def init_benchmark(self, benchmark_json):
        if os.path.exists(benchmark_json):
            with open(benchmark_json, "r") as file:
                self.benchmark = json.load(file).get(self._testMethodName)
        else:
            raise Exception("failed to find benchmark json file in %s" % benchmark_json)

    def handle_cmd(self, cmd, check=True, collect_result=True):
        """Ask the command interpreter to handle the command."""
        ret = lldb.SBCommandReturnObject()
        if collect_result:
            self.ci.HandleCommand(cmd, ret)
        else:
            self.dbg.HandleCommand(cmd)
        self.dbg.GetOutputFile().Flush()
        self.dbg.GetErrorFile().Flush()
        if collect_result and check:
            self.assertTrue(ret.Succeeded())
        return ret.GetOutput()

    def dump_timers(self):
        """
        Ask the command interpreter to handle the command ('log timers dump'),
        collect result and then check, then save result to log file.
        """
        cmd = "log timers dump"
        ret = lldb.SBCommandReturnObject()
        self.ci.HandleCommand(cmd, ret)
        self.dbg.GetOutputFile().Flush()
        self.dbg.GetErrorFile().Flush()
        self.assertTrue(ret.Succeeded())
        self.dump_timers_log = ret.GetOutput()
        self.log_file_write.write(self.dump_timers_log)

    def lldb_assert(self, expr, msg=""):
        """Check that the expression is true."""
        if not expr:
            raise self.failureException("LLDBTest " + msg)

    def lldb_assert_total_time(self, start, end):
        total_time = self.calculate_total_time(start, end)
        benchmark_time = float(self.benchmark.get("TotalTime").get("benchmark"))
        benchmark_range = self.benchmark.get("TotalTime").get("range")
        self.results.append(LogTagTime("TotalTime", total_time))
        percentage = total_time / benchmark_time
        if percentage < benchmark_range[0] or percentage > benchmark_range[1]:
            output_msg = (
                "LLDBTestTotalTime: actual time: %.9f, benchmark time: %.9f, range: %.2f%%~%.2f%%, actual percentage: %.2f%%"
                % (
                    total_time,
                    benchmark_time,
                    benchmark_range[0] * 100,
                    benchmark_range[1] * 100,
                    percentage * 100,
                )
            )
            raise self.failureException(output_msg)

    def get_tags(self, benchmark):
        tags = list(benchmark.keys())
        if "TotalTime" in tags:
            tags.remove("TotalTime")
        return tags

    def get_tag_time(self, tag, log_str):
        total_time = 0
        log_lines = log_str.splitlines()
        for line in log_lines:
            line = line.strip()
            num = line.find(tag)
            if num != -1:
                end = line.find(" ")
                time_str = line[:end]
                total_time += float(time_str)
        return total_time

    def calculate_total_time(self, start, end):
        start_integer = None
        start_decimal = None
        end_integer = None
        end_decimal = None
        tags = [start, end]
        for tag in tags:
            with open(self.log, "r+") as fo:
                for line in fo.readlines():
                    line = line.strip()
                    num = line.find(tag)
                    if num != -1:
                        integer_end = line.find(".")
                        decimal_end = line.find(" ")
                        integer_str = line[:integer_end]
                        decimal_str = line[integer_end:decimal_end]
                        if tag == start:
                            start_integer = float(integer_str)
                            start_decimal = float(decimal_str)
                        elif tag == end:
                            end_integer = float(integer_str)
                            end_decimal = float(decimal_str)
                        break
        if start_integer is None or start_decimal is None:
            raise Exception("start tag didn't exist in log")
        if end_integer is None or end_decimal is None:
            raise Exception("end tag didn't exist in log")
        calc_result = round(
            end_integer + end_decimal - start_integer - start_decimal, 9
        )
        return calc_result

    def get_results_with_benchmark(self):
        tags = self.get_tags(self.benchmark)
        for tag in tags:
            test_time = self.get_tag_time(tag, self.dump_timers_log)
            self.results.append(LogTagTime(tag, test_time))

    def output_report(self, benchmark):
        self.dump_timers()
        self.get_results_with_benchmark()
        rows = list()
        headers = ["tag", "actual time", "benchmark", "range", "actual percentage"]
        for result in self.results:
            benchmark_time = benchmark.get(result.id).get("benchmark")
            range = benchmark.get(result.id).get("range")
            percentage = result.value / benchmark_time * 100
            rows.append(
                [
                    result.id,
                    "%.9f" % result.value,
                    benchmark_time,
                    "[%.2f%%, %.2f%%]" % (range[0] * 100, range[1] * 100),
                    "%.2f%%" % percentage,
                ]
            )
        with open(self.report, "w", newline="") as f:
            f_csv = csv.writer(f)
            f_csv.writerow(headers)
            f_csv.writerows(rows)

    def reset_time_test_suite(self):
        if os.path.exists(self.report):
            os.remove(self.report)
        if os.path.exists(self.log):
            os.remove(self.log)
        self.results.clear()
