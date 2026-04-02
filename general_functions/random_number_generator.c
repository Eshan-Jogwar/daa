// gcc rng.c -o rng
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
  // 1. Now we expect 5 arguments: program, quantity, a, b, and filename
  if (argc != 5) {
    printf("Usage: %s <quantity> <a> <b> <filename>\n", argv[0]);
    printf("Example: %s 3 1234 4 numbers.txt\n", argv[0]);
    return 1;
  }

  int n = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  char *filename = argv[4]; // The new filename argument

  if (n <= 0) {
    printf("Error: Quantity must be greater than 0.\n");
    return 1;
  }

  int min = (a < b) ? a : b;
  int max = (a > b) ? a : b;

  srand(time(NULL));

  // 2. Open the file in "w" (write) mode.
  // If it doesn't exist, C will create it. If it does exist, C will overwrite
  // it.
  FILE *file = fopen(filename, "w");

  // Safety check: Make sure the file actually opened successfully
  if (file == NULL) {
    printf("Error: Could not open file %s for writing.\n", filename);
    return 1;
  }

  // 3. Generate and write the numbers
  for (int i = 0; i < n; i++) {
    int random_num = (rand() % (max - min + 1)) + min;

    // Write to the file using a space as the separator
    fprintf(file, "%d ", random_num);
  }

  // 4. Always close your files to save the data and free up memory!
  fclose(file);

  // Let the user know it finished successfully
  printf("Successfully generated %d numbers and saved to %s\n", n, filename);

  return 0;
}
