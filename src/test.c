#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"
#include "field.h"

int test_matrix() {
    char **a = NULL;
    int n, m;
    int i, j;
    int res;
    n = 20, m = 20;
    res = new_matrix(&a, n, m);
    if (res == 1)
        return 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) 
            a[i][j] = '1';
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) 
            printf("%c", a[i][j]);
        printf("\n");
    }
    free_matrix(a, n);
    return 0;
}

void test_field() {
    struct Field a;
    int n = 5, m = 7;
    int res = get_random_field(&a, n, m);
    if (res) {
        puts("FAIL");
    }
    print_field(&a); 
    free_field(&a);
}

void test_field_from_file() {
    struct Field a;
    int res = get_field_from_file(&a, "test.in");
    printf("res = %d\n", res);
    print_field(&a);
    free_field(&a);
}

int main() {
    test_matrix();
    test_field();
    test_field_from_file();
    return 0;
}
