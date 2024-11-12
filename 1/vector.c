#include "vector.h"
#include <stdlib.h>

void vector_init(struct vector *vector) {
  vector->length = 0;
  vector->capacity = 2;
  vector->arr = malloc(vector->capacity * sizeof(int));
}

void vector_push_back(struct vector *vector, int val) {
  if(vector->length == vector->capacity) {
    vector->capacity *= 2;
    vector->arr = realloc(vector->arr, vector->capacity * sizeof(int));
  }
  vector->arr[vector->length++] = val;
}

void vector_delete(struct vector *vector) {
  free(vector->arr);
}
