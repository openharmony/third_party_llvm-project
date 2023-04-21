// RUN: %clang --analyze -Xanalyzer -analyzer-checker=openharmony.NotHeapObj %s

#include "refbase.h"

using namespace std;
using namespace OHOS;

void sptr_test_good_case01(){
    RefBase *r = new RefBase();
    sptr<RefBase> s(r); 
}

RefBase *g_helper = new RefBase();
void sptr_test_good_case02(){
    sptr<RefBase> s(g_helper);
}

void sptr_test_bad_case01(){
    RefBase r;
    sptr<RefBase> s(&r);    // expected-warning {{sptr/wptr cannot manage non-heap objects}}
}
RefBase b_helper;
void sptr_test_bad_case02(){
    sptr<RefBase> s(&b_helper); // expected-warning {{sptr/wptr cannot manage non-heap objects}}
}


/********************wptr******************/
void wptr_test_good_case01(){
    RefBase *r = new RefBase();
    wptr<RefBase> w(r);
}

void wptr_test_good_case02(){
    wptr<RefBase> w(g_helper);
}

void wptr_test_bad_case01(){
    wptr<RefBase> w(&b_helper); // expected-warning {{sptr/wptr cannot manage non-heap objects}}
}
void wptr_test_bad_case02(){
    RefBase r;
    wptr<RefBase> s(&r);    // expected-warning {{sptr/wptr cannot manage non-heap objects}}
}
