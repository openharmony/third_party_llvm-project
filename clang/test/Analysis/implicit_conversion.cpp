// RUN: %clang --analyze -Xanalyzer -analyzer-checker=openharmony.OHPtrImplicitConversion %s

#include "refbase.h"

using namespace std;
using namespace OHOS;

class Derived:public RefBase{};
void sptr_test_good_case01(){
    sptr<RefBase> s(new RefBase);
}

void sptr_test_good_case02(){
    sptr<Derived> s(new Derived());
}

void sptr_test_good_case03(){
    class Derived:public RefBase{};
    sptr<RefBase> s(new Derived());
}

sptr<RefBase> sptr_test_good_case0405_helper(sptr<RefBase> s){
    return s;
}

void sptr_test_good_case04(){
    sptr<RefBase> s(new RefBase());
    sptr<RefBase> sp = sptr_test_good_case0405_helper(s);
}

sptr<RefBase> sptr_test_good_case05(){
    sptr<RefBase> s(new RefBase());
    sptr<RefBase> p = sptr_test_good_case0405_helper(s);
}

void sptr_test_bad_case01(){
    sptr<RefBase> s = new RefBase();    // expected-warning {{Implicit convert pointer to sptr/wptr}}
}

void sptr_test_bad_case02(){
    class Derived:public RefBase{};
    sptr<RefBase> s = new Derived();    // expected-warning {{Implicit convert pointer to sptr/wptr}}
}

sptr<RefBase> sptr_test_bad_case03_helper(sptr<RefBase> s){
    return s.GetRefPtr();   // expected-warning {{Implicit convert pointer to sptr/wptr}}
}

void sptr_test_bad_case03(){
    RefBase *r = new RefBase();
    sptr<RefBase> s = sptr_test_bad_case03_helper(r);   // expected-warning {{Implicit convert pointer to sptr/wptr}}
}

void sptr_test_bad_case04(){
    sptr<RefBase> s = nullptr;  // expected-warning {{Implicit convert pointer to sptr/wptr}}
}


/********************wptr******************/
void wptr_test_good_case01(){
    wptr<RefBase> w(new RefBase());
    wptr<RefBase> wp = w;
}

void wptr_test_bad_case01(){
    RefBase * r = new Derived();
    wptr<RefBase> w = r;    // expected-warning {{Implicit convert pointer to sptr/wptr}}
}
