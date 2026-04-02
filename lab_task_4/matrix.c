#include <stdbool.h>
#include <stdio.h>

typedef struct {
  int matrix[4][4];
  int target;
} test_case;

void solve(int mat[4][4], int target) {
  int r = 0;
  int c = 3;

  while (r < 4 && c >= 0) {
    if (mat[r][c] == target) {
      printf("FOUND at (%d, %d)\n", r, c);
      return;
    }
    if (mat[r][c] > target) {
      c--;
    } else {
      r++;
    }
  }
  printf("NOT FOUND\n");
}

void execute(test_case t_cases[], int tn) {
  for (int i = 0; i < tn; i++) {
    printf("CASE %d: ", i + 1);
    solve(t_cases[i].matrix, t_cases[i].target);
  }
}

int main() {
  test_case t_cases[] = {
      {{{1, 4, 7, 11}, {2, 5, 8, 12}, {3, 6, 9, 16}, {10, 13, 14, 17}}, 5},
      {{{1, 4, 7, 11}, {2, 5, 8, 12}, {3, 6, 9, 16}, {10, 13, 14, 17}}, 20}};

  execute(t_cases, 2);
  return 0;
}
