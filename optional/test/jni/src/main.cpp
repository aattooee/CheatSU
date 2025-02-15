#include "driver.hpp"
#include "linux/types.h"
#include <cstdint>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

uint64_t get_tick_count64() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1000 + ts.tv_nsec / (1000 * 1000));
}

pid_t get_name_pid(char *name) {
  FILE *fp;
  pid_t pid;
  char cmd[0x100] = "pidof ";

  strcat(cmd, name);
  fp = popen(cmd, "r");
  fscanf(fp, "%d", &pid);
  pclose(fp);
  return pid;
}
static uintptr_t test_ptr = 0x12345;
static int arr1[] = {1,2};
static int arr2[] = {3,4};
static int* testarr[] = {arr1,arr2};
int main() {

  uintptr_t base = 0;
  uint64_t result = 0;
  char module_name[0x100] = "libunity.so";
  pid_t pid = get_name_pid((char *)"com.tencent.tmgp.sgame");
  printf("pid = %d\n", pid);

  driver->initialize(pid);
  // test base
  {
    base = driver->get_module_base(module_name);
    printf("base = %lx\n", base);
  }

  // test perf
  {
    size_t number = 1000;
    uint64_t now = get_tick_count64();
    for (size_t i = 0; i < number; i++) {
      result = driver->read<uint64_t>(base);
    }
    printf("Read %ld times cost = %lfs\n", number,
           (double)(get_tick_count64() - now) / 1000);
    printf("result = %lx\n", result);
  }
  // test read ptr
  driver->initialize(getpid());
  {
    uintptr_t *ptr1 = &test_ptr;
    uintptr_t ptr2 = 0;
    memcpy(&ptr2, &ptr1, 8);
    uintptr_t res = driver->read<uintptr_t>(ptr2);
    printf("testptr:%p,from driver:%p\n", test_ptr, res);
  }
  // test read_with_offsets
  {
    void * ptr = testarr;
    uintptr_t ptr2 = 0;
    memcpy(&ptr2, &ptr, 8);
    uintptr_t offset[] = {8,4};
    int res = driver->read_with_offsets<int>(ptr2,2,offset);
    printf("test read with offsets:\ntestarr[1][1]:%d,from driver:%d\n", testarr[1][1], res);
  }
  // test bypass mincore detection
  {
    
  }

  return 0;
}
