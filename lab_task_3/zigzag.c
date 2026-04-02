#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int *arr;
  int length;
} test_case;

void solve(int nums[], int n) {
  if (n % 2 == 0) {
    printf("NOT POSSIBLE\n");
    return;
  }

  long long sum = 0;
  for (int i = 0; i < n; i++) {
    sum += nums[i];
  }

  int n_even = (n + 1) / 2;
  long long numerator = sum - n_even;

  if (numerator % n != 0) {
    printf("NOT POSSIBLE\n");
    return;
  }

  long long x = numerator / n;
  long long target_even = x + 1;
  long long target_odd = x;

  long long total_diff = 0;
  for (int i = 0; i < n; i++) {
    long long target = (i % 2 == 0) ? target_even : target_odd;
    total_diff += llabs(nums[i] - target);
  }

  printf("POSSIBLE. Min k: %lld\n", total_diff / 2);
}

void execute(test_case t_cases[], int tn) {
  for (int i = 0; i < tn; i++) {
    printf("CASE %d: ", i + 1);
    solve(t_cases[i].arr, t_cases[i].length);
  }
}

int main() {
  int d1[] = {2, 1, 2};
  int d2[] = {3, 3, 3, 3, 3};
  int d3[] = {10, 2, 10, 2, 10};
  int d4[] = {-2, -3, -2};
  int d5[] = {5, 1, 5};

  test_case t_cases[] = {{.arr = d1, .length = 3},
                         {.arr = d2, .length = 5},
                         {.arr = d3, .length = 5},
                         {.arr = d4, .length = 3},
                         {.arr = d5, .length = 3}};

  execute(t_cases, 5);
  return 0;
}
