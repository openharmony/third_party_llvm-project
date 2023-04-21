// RUN: %clang --analyze -Xanalyzer -analyzer-checker=openharmony.RefBaseDelete %s

#include "refbase.h"

#include <iostream>

using namespace std;
using namespace OHOS;

void test_good_case01(){
    RefBase * r = new RefBase();
    delete r;
}

void test_bad_case01(){
    RefBase * r = new RefBase();
    sptr<RefBase> s(r);
    delete r;   // expected-warning {{Manually deleted the RefBase pointer managed by sptr/wptr}}
}
void test_bad_case02(){
    class Derived:public RefBase{};
    Derived* d = new Derived();
    sptr<RefBase> s(d);
    delete d;   // expected-warning {{Manually deleted the RefBase pointer managed by sptr/wptr}}
}
void test_bad_case03(){
    class Derived:public RefBase{};
    Derived* d = new Derived();
    sptr<Derived> s(d);
    delete d;   // expected-warning {{Manually deleted the RefBase pointer managed by sptr/wptr}}
}