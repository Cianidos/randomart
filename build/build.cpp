#include <cstdlib>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char **argv) {
  struct stat t_stat;
  stat("build", &t_stat);
  time_t executable_mt = t_stat.st_mtimespec.tv_sec;
  stat("build.cpp", &t_stat);
  time_t source_mt = t_stat.st_mtimespec.tv_sec;

  // if the source file modified later then the executable than it should be
  // recompiled
  bool should_rebuild = difftime(executable_mt, source_mt) <= 0.0;
  if (should_rebuild) {
    std::system("clang++ -Wall -std=c++17 build.cpp -o build");
    std::system("./build");
    return 0;
  }

  std::system("clang++ -g -O0 -Wall -std=c++17 ../main.cpp -o ../main");
  std::system("./../main");
  return 0;
}
