// Copyright �2018 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in innative.h

#ifndef __TOOLS_H__IR__
#define __TOOLS_H__IR__

#include "innative/export.h"
#include <stdint.h>
#include <ostream>

struct __WASM_ENVIRONMENT;

namespace innative {
  struct __WASM_ENVIRONMENT* CreateEnvironment(unsigned int flags, unsigned int modules, unsigned int maxthreads);
  void DestroyEnvironment(struct __WASM_ENVIRONMENT* env);
  void LoadModule(struct __WASM_ENVIRONMENT* env, size_t index, const void* data, uint64_t size, const char* name, int* err);
  void AddModule(struct __WASM_ENVIRONMENT* env, const void* data, uint64_t size, const char* name, int* err);
  void AddWhitelist(struct __WASM_ENVIRONMENT* env, const char* module_name, const char* export_name, const FunctionType* ftype);
  void WaitForLoad(struct __WASM_ENVIRONMENT* env);
  enum IR_ERROR AddEmbedding(struct __WASM_ENVIRONMENT* env, int tag, const void* data, uint64_t size);
  enum IR_ERROR Compile(struct __WASM_ENVIRONMENT* env, const char* file);
  IR_Entrypoint LoadFunction(void* cache, const char* module_name, const char* function, const FunctionType* ftype);
  void* LoadGlobal(void* cache, const char* module_name, const char* export_name);
  void* LoadAssembly(int flags, const char* file);
  void DumpModule(std::ostream& stream, Module& mod);
}

#endif  