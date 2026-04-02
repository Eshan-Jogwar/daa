#include <stdio.h>

typedef struct {
  int *arr;
  int length;
} test_case;

void reverse_array(int nums[], int n) {
  int left = 0;
  int right = n - 1;

  while (left < right) {
    int temp = nums[left];
    nums[left] = nums[right];
    nums[right] = temp;
    left++;
    right--;
  }
}

void display(int arr[], int n) {
  for (int i = 0; i < n; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
}

void execute(test_case t_cases[], int tn) {
  for (int i = 0; i < tn; i++) {
    reverse_array(t_cases[i].arr, t_cases[i].length);
    display(t_cases[i].arr, t_cases[i].length);
  }
}

int main() {
  // Define the data separately
  int data1[] = {1, 3, 5, 1, 2, 6};
  int data2[] = {3, 1, 5, 2, 5, 7};
  int data3[] = {7, 2, 4, 1, 3, 0};
  int data4[] = {23, 9, 0, 2, 6};
  int data5[] = {23, 75, 24, 2, 5, 7};
  int n = 5;
  // Assign pointers to the struct array
  test_case t_cases[] = {
      {.arr = data1, .length = 6}, {.arr = data2, .length = 6},
      {.arr = data3, .length = 6}, {.arr = data4, .length = 5},
      {.arr = data5, .length = 6},
  };
  execute(t_cases, n);

  return 0;
}
