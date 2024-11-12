#ifndef VECTOR_H
#define VECTOR_H
struct vector {
    int length;
    int capacity;
    int *arr;
};
void vector_init(struct vector *v);

void vector_push_back(struct vector *v, int val);

void vector_delete(struct vector *v);

#endif //VECTOR_H
