// OHOS_LOCAL begin
// RUN: %clang_cc1 -verify -fsyntax-only %s

__attribute__((optimize())) // expected-error {{'optimize' attribute takes at least 1 argument}}
void f0() {}

__attribute__((optimize(a))) // expected-warning {{'optimize' attribute only support string argument; attribute ignored}}
void f1() {}

int b = 1;
__attribute__((optimize(b))) // expected-warning {{'optimize' attribute only support string argument; attribute ignored}}
void f2() {}

__attribute__((optimize("omit-frame-pointer"))) // expected-no-error
void f3() {}

__attribute__((optimize(1))) // expected-warning {{'optimize' attribute only support string argument; attribute ignored}}
void f12() {}

__attribute__((optimize(0))) // expected-warning {{'optimize' attribute only support string argument; attribute ignored}}
void f13() {}
// OHOS_LOCAL end
