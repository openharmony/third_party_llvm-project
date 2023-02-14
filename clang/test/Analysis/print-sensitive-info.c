// RUN: %clang_analyze_cc1 -analyzer-checker=openharmony -verify %s

void HILOG(int x, int y) {
    return;
}

int getPassword() {
    return 1;
}

void test1() {
    int x = getPassword();
    int y = getPassword();
    HILOG(x, y);
    // expected-warning@-1 {{x is a sensitive information}}
    // expected-warning@-2 {{y is a sensitive information}}
}

void test2(int password, int passwd) {
    HILOG(password, passwd);
    // expected-warning@-1 {{password is a sensitive information}}
    // expected-warning@-2 {{passwd is a sensitive information}}
}
