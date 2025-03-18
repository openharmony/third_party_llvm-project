#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>
using namespace std;

stack<int> getstack() {
  stack<int> st; // Set stack break point at this line.
  st.push(1);
  st.push(2);
  st.push(3);
  return st;
}

string getstring() {
  string str = "Hello"; // Set string break point at this line.
  return str;
}

map<string, int> getmap() {
  map<string, int> mp; // Set map break point at this line.
  mp["A"] = 100;
  mp["B"] = 50;
  mp["C"] = 0;
  return mp;
}

class father {
public:
  father() {};
  vector<int> returnvec() { return this->vec; }
  list<int> returnlists() { return this->l; }
  string getstring() { return s; }
  int getint() { return a; }

protected:
  int a = 0;
  string s = "Hello";
  vector<int> vec{1, 2, 3};
  list<int> l{1, 1, 1};
};

class son : public father {
public:
  son() {};
  vector<int> returnvec() { return this->vec; }
  list<int> returnlists() { return this->l; }
  string getstring() { return s; }
  int getint() { return a; }
};

father getclass() {
  father fa; // Set class break point at this line.
  return fa;
}

son get_son_class() {
  son s; // Set child_class break point at this line.
  return s;
}

int main() {
  stack<int> stack1;
  stack1 = getstack();
  string str = getstring();
  map<string, int> mp = getmap();
  father fa = getclass();
  son s = get_son_class();
  return 0;
}

