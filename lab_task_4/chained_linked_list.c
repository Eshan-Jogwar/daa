#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
  int data;
  struct Node *next1;
  struct Node *next2;
} Node;

Node *createlist1(int n, int a) {
  Node *head = NULL;
  Node *tail = NULL;

  for (int i = a; i <= n; i += a) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->data = i;
    newNode->next1 = NULL;
    newNode->next2 = NULL;

    if (!head) {
      head = newNode;
      tail = newNode;
    } else {
      tail->next1 = newNode;
      tail = newNode;
    }
  }
  return head;
}

Node *createlist2(int n, int a, int b, Node *A) {
  Node *head = NULL;
  Node *tail = NULL;
  Node *currA = A;

  for (int i = b; i <= n; i += b) {
    Node *targetNode = NULL;

    if (i % a == 0) {
      while (currA && currA->data < i) {
        currA = currA->next1;
      }
      if (currA && currA->data == i) {
        targetNode = currA;
      }
    }

    if (!targetNode) {
      targetNode = (Node *)malloc(sizeof(Node));
      targetNode->data = i;
      targetNode->next1 = NULL;
      targetNode->next2 = NULL;
    }

    if (!head) {
      head = targetNode;
      tail = targetNode;
    } else {
      tail->next2 = targetNode;
      tail = targetNode;
    }
  }
  return head;
}

void prnlist(Node *head, int indicator) {
  Node *curr = head;
  while (curr) {
    printf("%d ", curr->data);
    if (indicator == 1) {
      curr = curr->next1;
    } else {
      curr = curr->next2;
    }
  }
  printf("\n");
}

void prnboth(Node *A, Node *B) {
  Node *pA = A;
  Node *pB = B;

  while (pA && pB) {
    if (pA == pB) {
      printf("%d ", pA->data);
      pA = pA->next1;
      pB = pB->next2;
    } else if (pA->data < pB->data) {
      printf("%d ", pA->data);
      pA = pA->next1;
    } else {
      printf("%d ", pB->data);
      pB = pB->next2;
    }
  }

  while (pA) {
    printf("%d ", pA->data);
    pA = pA->next1;
  }

  while (pB) {
    printf("%d ", pB->data);
    pB = pB->next2;
  }
  printf("\n");
}

int main() {
  int n = 250;
  int a = 21;
  int b = 15;

  Node *A = createlist1(n, a);
  Node *B = createlist2(n, a, b, A);

  prnlist(A, 1);
  prnlist(B, 2);
  prnboth(A, B);

  return 0;
}
