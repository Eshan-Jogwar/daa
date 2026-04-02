#include <math.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
  int *arr;
  int length;
} test_case;

bool is_prime(int n) {
  if (n < 2)
    return false;
  if (n == 2 || n == 3)
    return true;
  if (n % 2 == 0 || n % 3 == 0)
    return false;
  for (int i = 5; i * i <= n; i += 6) {
    if (n % i == 0 || n % (i + 2) == 0)
      return false;
  }
  return true;
}

bool is_heptagonal(int x) {
  if (x < 1)
    return false;
  long long disc = 49 - 28 * (2 - 2LL * x);
  if (disc < 0)
    return false;
  long long root = (long long)sqrt(disc);
  if (root * root != disc)
    return false;
  return (7 + root) % 14 == 0;
}

void solve(int nums[], int n) {
  int count = 0;
  for (int i = 0; i < n; i++) {
    if (is_heptagonal(nums[i]) && is_prime(nums[i])) {
      printf("%d ", nums[i]);
      count++;
    }
  }
  printf("\nResult: %d\n", count);
}

void execute(test_case t_cases[], int tn) {
  for (int i = 0; i < tn; i++) {
    printf("CASE %d: ", i + 1);
    solve(t_cases[i].arr, t_cases[i].length);
    printf("\n");
  }
}

int main() {
  int d1[] = {43, 71, 197, 10, 20};
  int d2[] = {463, 547, 953, 11, 13};
  int d3[] = {1471, 1933, 2647, 2843, 3697};
  int d4[] = {1, 2, 3, 4, 5, 6, 7};
  int d5[] = {100, 200, 300, 43, 71};

  test_case t_cases[] = {{.arr = d1, .length = 5},
                         {.arr = d2, .length = 5},
                         {.arr = d3, .length = 5},
                         {.arr = d4, .length = 7},
                         {.arr = d5, .length = 5}};

  execute(t_cases, 5);
  return 0;
}
