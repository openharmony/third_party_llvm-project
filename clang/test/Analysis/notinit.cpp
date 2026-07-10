// RUN: %clang --analyze -Xanalyzer -analyzer-checker=openharmony.OHPtrNotInit %s

#include "refbase.h"

using namespace OHOS;

void sptr_test_good_case01(){
    sptr<RefBase> s(new RefBase());
    s->GetRefCounter();
    (*s).GetRefCounter();
}
void sptr_test_good_case02(){
    sptr<RefBase> s;
    s.ForceSetRefPtr(new RefBase());
    s->GetRefCounter();
    (*s).GetRefCounter();
}
void sptr_test_good_case03(){
    sptr<RefBase> s(new RefBase());
    sptr<RefBase> p(new RefBase);
    s = p;
    s->GetRefCounter();
    (*s).GetRefCounter();
}
void sptr_test_good_case04(){
    sptr<RefBase> s;
    s.ForceSetRefPtr(new RefBase());
    s->GetRefCounter();
}




void sptr_test_good_case05(sptr<RefBase> s){
    if(s == nullptr){
        return;
    }
    s->GetRefCounter();
}


void sptr_test_good_case06(){
    class D{
        public:
        sptr<RefBase> s = new RefBase();
    };
    D d;
    if(1 < 2 || d.s == nullptr){
        return;
    }
    if(d.s->GetRefCounter() == nullptr){
        d.s->GetRefCounter();
    }
}



void sptr_test_good_case07_helper01(sptr<RefBase> s){
    if(s == nullptr){
        return;
    }
    s->GetRefCounter();
}
void sptr_test_good_case07_helper02(sptr<RefBase> s){
    if(s){
        s->GetRefCounter();
    }
}
void sptr_test_good_case07_helper03(sptr<RefBase> s){
    if(s != nullptr){
        s->GetRefCounter();
    }
}
void sptr_test_good_case07(){
    sptr<RefBase> s;
    sptr_test_good_case07_helper01(s);
    sptr_test_good_case07_helper02(s);
    sptr_test_good_case07_helper03(s);
}
void sptr_test_bad_case01(){
    sptr<RefBase> s;
    s->GetRefCounter();     // expected-warning {{sptr/wptr not init}}
}
void sptr_test_bad_case02(){
    sptr<RefBase> s;
    (*s).GetRefCounter();   // expected-warning {{sptr/wptr not init}}
}

/*****************wptr****************/

void wptr_test_good_case01(){
    wptr<RefBase> w(new RefBase());
    w->GetRefCounter();
}

void wptr_test_bad_case01(){
    wptr<RefBase> w;
    w->GetRefCounter(); // expected-warning {{sptr/wptr not init}}
}
