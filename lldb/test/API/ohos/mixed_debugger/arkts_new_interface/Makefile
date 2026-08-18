# Copyright (C) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LD_EXTRAS := -ldl -Wl,-rpath,'$$ORIGIN'

C_SOURCES := main.c

include Makefile.rules

all: libarkshim.so

libarkshim.so: $(SRCDIR)/arkshim.c
	$(CC) $(CFLAGS) -fPIC -fvisibility=hidden -shared -Wl,-soname,libarkshim.so \
		-o $@ $<
	- strip -g $@  # 没有 strip 也不致命，加个 '-' 忽略报错

clean::
	$(RM) libarkshim.so
