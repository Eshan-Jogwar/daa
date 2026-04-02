#include <stdio.h>

typedef struct {
  int *arr;
  int length;
} test_case;

void print_subsets(int nums[], int n) {
  for (int i = 0; i < (1 << n); i++) {
    int count = 0;
    int subset[2]; // Note: This only works if you strictly want size 2
    int pointer = 0;

    for (int j = 0; j < n; j++) {
      if ((i >> j) & 1) {
        // Only fill the buffer if we have space
        if (pointer < 2) {
          subset[pointer] = nums[j];
        }
        pointer++;
        count++;
      }
    }

    // MOVE THIS OUTSIDE THE J LOOP
    // Only print if the TOTAL count for this mask is exactly 2
    if (count == 2) {
      printf("%d %d \n", subset[0], subset[1]);
    }
  }
}

void execute(test_case t_cases[], int tn) {
  for (int i = 0; i < tn; i++) {
    printf("test case %d START", i);
    print_subsets(t_cases[i].arr, t_cases[i].length);
    printf("test case %d END\n\n\n", i);
  }
}

int main() {
  // Define the data separately
  int data1[] = {1, 3, 5, 1, 2, 6};
  int data2[] = {3, 1, 5, 2, 5, 7};
  int data3[] = {7, 2, 4, 1, 3, 0};
  int data4[] = {23, 9, 0, 2, 6};
  int data5[] = {23, 75, 24, 2, 5, 7};
  int n = 5; // Assign pointers to the struct array
  test_case t_cases[] = {
      {.arr = data1, .length = 6}, {.arr = data2, .length = 6},
      {.arr = data3, .length = 6}, {.arr = data4, .length = 5},
      {.arr = data5, .length = 6},
  };
  execute(t_cases, n);

  return 0;
}
