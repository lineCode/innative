// Copyright (c)2020 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in innative.h

#include "test.h"
#include <stdio.h>

TestHarness::TestHarness(const INExports& exports, const char* arg0, int loglevel, FILE* out, const path& folder) :
  _exports(exports), _arg0(arg0), _loglevel(loglevel), _target(out), _folder(folder), _testdata(0, 0)
{}
TestHarness::~TestHarness()
{
  // Clean up all the files we just produced
  for(auto f : _garbage)
    remove(f);
}
size_t TestHarness::Run(FILE* out)
{
  std::pair<const char*, void (TestHarness::*)()> tests[] = { { "wasm_malloc.c", &TestHarness::test_malloc },
                                                              { "debugging.cpp", &TestHarness::test_debug },
                                                              { "embedding", &TestHarness::test_embedding },
                                                              { "funcreplace.c", &TestHarness::test_funcreplace },
                                                              { "internal.c", &TestHarness::test_environment },
                                                              { "queue.h", &TestHarness::test_queue },
                                                              { "stack.h", &TestHarness::test_stack },
                                                              { "stream.h", &TestHarness::test_stream },
                                                              { "utility.h", &TestHarness::test_util },
                                                              { "manual", &TestHarness::test_manual },
                                                              //{ "assemblyscript", &TestHarness::test_assemblyscript },
                                                              { "allocator", &TestHarness::test_allocator },
                                                              { "parallel parsing", &TestHarness::test_parallel_parsing },
                                                              { "whitelist", &TestHarness::test_whitelist },
                                                              { "serializer", &TestHarness::test_serializer },
                                                              { "errors", &TestHarness::test_errors },
                                                              { "atomic_waitnotify",
                                                                &TestHarness::test_atomic_waitnotify } };

  static const size_t NUMTESTS    = sizeof(tests) / sizeof(decltype(tests[0]));
  static constexpr int COLUMNS[3] = { 24, 11, 8 };

  fprintf(out, "%-*s %-*s %-*s\n", COLUMNS[0], "Internal Tests", COLUMNS[1], "Subtests", COLUMNS[2], "Pass/Fail");
  fprintf(out, "%-*s %-*s %-*s\n", COLUMNS[0], "--------------", COLUMNS[1], "--------", COLUMNS[2], "---------");

  size_t failures = 0;
  for(size_t i = 0; i < NUMTESTS; ++i)
  {
    (this->*tests[i].second)();
    auto results = Results();
    failures += results.second - results.first;

    char buf[COLUMNS[1] + 1] = { 0 };
    snprintf(buf, COLUMNS[1] + 1, "%u/%u", results.first, results.second);
    fprintf(out, "%-*s %-*s %-*s\n", COLUMNS[0], tests[i].first, COLUMNS[1], buf, COLUMNS[2],
            (results.first == results.second) ? "PASS" : "FAIL");
  }

  {
    TEST(CompileWASM("../scripts/test-h.wat", nullptr) == ERR_SUCCESS);
#ifdef IN_PLATFORM_WIN32
  #ifdef IN_32BIT
    TEST(CompileWASM("../scripts/test-win32-cref.wat", nullptr) == ERR_SUCCESS);
  #else
    TEST(CompileWASM("../scripts/test-win64.wat", nullptr) == ERR_SUCCESS);
    TEST(CompileWASM("../scripts/test-win64-cref.wat", nullptr) == ERR_SUCCESS);
  #endif
#endif

    char buf[COLUMNS[1] + 1] = { 0 };
    auto results             = Results();
    snprintf(buf, COLUMNS[1] + 1, "%u/%u", results.first, results.second);
    fprintf(out, "%-*s %-*s %-*s\n", COLUMNS[0], "aux tests", COLUMNS[1], buf, COLUMNS[2],
            (results.first == results.second) ? "PASS" : "FAIL");
  }

  fprintf(out, "\n");
  return failures;
}

int TestHarness::CompileWASM(const path& file, int (TestHarness::*fn)(void*), const char* system,
                             std::function<int(Environment*)> preprocess, const char* name)
{
  Environment* env = (*_exports.CreateEnvironment)(1, 0, 0);
  if(!env)
    return ERR_UNKNOWN_ENVIRONMENT_ERROR;
  env->flags       = ENV_ENABLE_WAT | ENV_LIBRARY;
  env->optimize    = ENV_OPTIMIZE_O3;
  env->features    = ENV_FEATURE_ALL;
  env->log         = stdout;
  env->loglevel    = _loglevel;
  if(system)
    env->system = system;

#ifdef IN_DEBUG
  env->flags |= ENV_DEBUG;
  env->optimize = ENV_OPTIMIZE_O0;
#endif

  if(preprocess)
    if(int err = preprocess(env); err < 0)
      return err;

  auto stem = file.stem().u8string();
  int err = (*_exports.AddEmbedding)(env, 0, (void*)INNATIVE_DEFAULT_ENVIRONMENT, 0, 0);
  if(err >= 0)
    (*_exports.AddModule)(env, file.u8string().c_str(), 0, (!name ? stem.c_str() : name), &err);

  if(err >= 0)
    (*_exports.FinalizeEnvironment)(env);

  path base = _folder / file.stem();
  _out      = base;
  _out.replace_extension(IN_LIBRARY_EXTENSION);

  if(err >= 0)
    err = (*_exports.Compile)(env, _out.u8string().c_str());

  (*_exports.DestroyEnvironment)(env);

  if(err < 0)
    return err;

  _garbage.push_back(_out);
#ifdef IN_PLATFORM_WIN32
  base.replace_extension(".lib");
  _garbage.push_back(base);
#endif
  void* m = (*_exports.LoadAssembly)(_out.u8string().c_str());
  if(!m)
    return ERR_FATAL_INVALID_MODULE;
  if(fn)
    err = (this->*fn)(m);
  (*_exports.FreeAssembly)(m);

  return err;
}
