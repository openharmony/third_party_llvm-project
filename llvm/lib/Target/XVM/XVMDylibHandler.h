
#ifndef __XVM_DYLIB_HANDLER__H
#define __XVM_DYLIB_HANDLER__H

#include "llvm/Support/Mutex.h"
#include <dlfcn.h>
#include <map>
#include <string.h>
using namespace std;

extern llvm::sys::SmartMutex<true>  mutexLoadDylib;
extern std::map<std::string, void *> mapTargetNameHandler;

#define LOAD_XVM_DYLIB \
  static void * \
  LoadDylibTarget() { \
    llvm::sys::SmartScopedLock<true> lock(mutexLoadDylib); \
    std::map<std::string, void *>::iterator itr = mapTargetNameHandler.find("XVM"); \
    void *Handler; \
    const char * targetDyLibName = "LLVMXVMTarget.so"; \
    if (itr == mapTargetNameHandler.end()) { \
      Handler = dlopen(targetDyLibName, RTLD_LAZY | RTLD_NOLOAD); \
      if (!Handler) { \
        Handler = dlopen(targetDyLibName, RTLD_LAZY); \
      } \
      if (Handler) { \
        mapTargetNameHandler[targetDyLibName] = Handler; \
      } \
    } else { \
      Handler = mapTargetNameHandler[targetDyLibName]; \
    } \
    return Handler; \
  }

#define LLVM_INIT_XVM_COMP(Component) \
  LOAD_XVM_DYLIB \
  extern "C" LLVM_EXTERNAL_VISIBILITY \
  void LLVMInitializeXVM##Component(void) { \
    void *Handler = LoadDylibTarget(); \
    if (!Handler) \
      return; \
    void (*FuncPtr)(void) = (void (*)())(dlsym(Handler, "LLVMInitializeXVM" #Component "CalledInDylib")); \
    if (!FuncPtr) { \
      return; \
    } \
    FuncPtr(); \
  }

#endif // __XVM_DYLIB_HANDLER__H
