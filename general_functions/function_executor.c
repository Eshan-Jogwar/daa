#include <stdio.h>
#include <time.h>
float execute_function(void (*fn)()) {
  clock_t start_time = clock();
  fn();
  clock_t end_time = clock();

  return end_time - start_time;
};

void fn() {
  for (int i = 0; i < 100000; i++)
    ;
}

int main() {
  int a = execute_function(fn);
  printf("%d", a);
}
