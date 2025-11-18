/*
Program written by Andrew Sauer
Calculates the number of groups of size n (would not reccomend n > 8)
Run with "./num_groups n"
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TableNode {
    int *flat;
    struct TableNode *next;
} TableNode;

TableNode *seen_head = NULL;

/* Function to check if a number is prime */
int is_prime(int n) {
    int i;
    if (n <= 1) {
        return 0;
    }
    for (i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

/* Check if n is a square of a prime (p*p) */
int is_prime_square(int n) {
    int p = 2;
    while (p * p <= n) {
        if (p * p == n && is_prime(p)) {
            return 1;
        }
        p++;
    }
    return 0;
}

/* Check if the table is closed */
int check_closure(int **table, int n) {
    int i;
    int j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (table[i][j] < 0 || table[i][j] >= n) {
                return 0;
            }
        }
    }
    return 1;
}

/* Check if an element e is an identity */
int is_identity(int **table, int n, int e) {
    int i;
    for (i = 0; i < n; i++) {
        if (table[i][e] != i) {
            return 0;
        }
        if (table[e][i] != i) {
            return 0;
        }
    }
    return 1;
}

/* Find identity element index */
int find_identity(int **table, int n) {
    int e;
    for (e = 0; e < n; e++) {
        if (is_identity(table, n, e)) {
            return e;
        }
    }
    return -1;
}

/* Check that each element has an inverse */
int check_inverses(int **table, int n, int identity) {
    int a;
    int b;
    int found;

    for (a = 0; a < n; a++) {
        found = 0;
        for (b = 0; b < n; b++) {
            if (table[a][b] == identity && table[b][a] == identity) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return 0;
        }
    }
    return 1;
}

/* Check associativity for ALL triples */
int check_associativity(int **table, int n) {
    int a;
    int b;
    int c;

    for (a = 0; a < n; a++) {
        for (b = 0; b < n; b++) {
            for (c = 0; c < n; c++) {
                int left = table[ table[a][b] ][c];
                int right = table[a][ table[b][c] ];
                if (left != right) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void flatten_table(int **table, int n, int *flat) {
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            flat[i * n + j] = table[i][j];
        }
    }
}

int next_permutation(int *arr, int length) {
    int pivot_index = length - 2;

    while (pivot_index >= 0 && arr[pivot_index] >= arr[pivot_index + 1]) {
        pivot_index--;
    }

    if (pivot_index < 0) {
        return 0;
    }

    int successor_index = length - 1;
    while (arr[successor_index] <= arr[pivot_index]) {
        successor_index--;
    }

    int temp = arr[pivot_index];
    arr[pivot_index] = arr[successor_index];
    arr[successor_index] = temp;

    int start = pivot_index + 1;
    int end = length - 1;

    while (start < end) {
        temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;
        start++;
        end--;
    }
    return 1;
}


/* Check if a Cayley table is isomorphic to any previously stored table) */
int is_new_group(int **table, int n) {
    /* Flatten current table for comparison */
    int *flat_current = malloc(n * n * sizeof(int));
    int row, col, k;
    for (row = 0; row < n; row++) {
        for (col = 0; col < n; col++) {
            flat_current[row * n + col] = table[row][col];
        }
    }
    /* Check against all previously stored tables */
    TableNode *cur_node = seen_head;
    while (cur_node != NULL) {
        int match = 1;
        for (k = 0; k < n * n; k++) {
            if (cur_node->flat[k] != flat_current[k]) {
                match = 0;
            }
        }
        if (match == 1) {
            free(flat_current);
            return 0;
        }
        cur_node = cur_node->next;
    }
    free(flat_current);
    /* If n == 1 or n == 2, no permutations needed */
    if (n == 1 || n == 2) {
        return 1;
    }
    /* Prepare permutation array for non-identity elements (1..n-1) */
    int perm[n - 1];
    for (k = 0; k < n - 1; k++) {
        perm[k] = k;
    }
    /* Generate all permutations and check for duplicates */
    do {
        int *flat_copy = malloc(n * n * sizeof(int));
        for (row = 0; row < n; row++) {
            for (col = 0; col < n; col++) {
                int val = table[row][col];
                int new_val;
                if (val == 0) {
                    new_val = 0;
                } else {
                    new_val = perm[val - 1] + 1;
                }
                flat_copy[row * n + col] = new_val;
            }
        }
        /* Compare permuted table to all stored tables */
        cur_node = seen_head;
        int duplicate = 0;
        while (cur_node != NULL) {
            int match = 1;
            for (k = 0; k < n * n; k++) {
                if (cur_node->flat[k] != flat_copy[k]) {
                    match = 0;
                }
            }
            if (match == 1) {
                duplicate = 1;
            }
            cur_node = cur_node->next;
        }
        free(flat_copy);
        if (duplicate == 1) {
            return 0;
        }

    } while (next_permutation(perm, n - 1) == 1);

    return 1;
}


void store_table(int *flat, int n) {
    TableNode *node = malloc(sizeof(TableNode));
    node->flat = malloc(n * n * sizeof(int));
    int i;
    for (i = 0; i < n * n; i++) {
        node->flat[i] = flat[i];
    }
    node->next = seen_head;
    seen_head = node;
}

/* Print one Cayley table */
void print_table(int **table, int n) {
    int i;
    int j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", table[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Recursive function to fill table row by row */
int generate_tables(int **table, int n, int row, int col, int must_be_abelian, int *count) {
    int val;

    if (row == n) {
        if (!check_closure(table, n)) {
            return 0;
        }

        if (must_be_abelian) {
            int i;
            int j;
            for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    if (table[i][j] != table[j][i]) {
                        return 0;
                    }
                }
            }
        }

        int identity = find_identity(table, n);
        if (identity == -1) {
            return 0;
        }
        if (!check_inverses(table, n, identity)) {
            return 0;
        }
        if (!check_associativity(table, n)) {
            return 0;
        }

        if (is_new_group(table, n)) {
            int *flat = malloc(n * n * sizeof(int));
            flatten_table(table, n, flat);
            store_table(flat, n);
            (*count)++;
            printf("Valid group #%d:\n", *count);
            print_table(table, n);
            free(flat);
        }

        return 1;
    }

    for (val = 0; val < n; val++) {
        table[row][col] = val;
        if (must_be_abelian) {
            if (col < row) {
                if (table[col][row] != val) {
                    continue;
                }
            }
        }

        int next_row = row;
        int next_col = col + 1;

        if (next_col == n) {
            next_row = row + 1;
            next_col = 0;
        }
        generate_tables(table, n, next_row, next_col, must_be_abelian, count);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int n;
    int i;
    int must_be_abelian;
    int **table;
    int count;

    if (argc < 2) {
        printf("Usage: %s <n>\n", argv[0]);
        return 1;
    }

    n = atoi(argv[1]);

    if (n <= 0) {
        printf("Error: n must be positive.\n");
        return 1;
    }

    table = malloc(n * sizeof(int *));

    for (i = 0; i < n; i++) {
        table[i] = malloc(n * sizeof(int));
        if (table[i] == NULL) {
            printf("Memory allocation failed.\n");
            return 1;
        }
        memset(table[i], 0, n * sizeof(int));
    }

    if (is_prime(n) || is_prime_square(n)) {
        must_be_abelian = 1;
    }
    else {
        must_be_abelian = 0;
    }

    count = 0;
    generate_tables(table, n, 0, 0, must_be_abelian, &count);

    printf("Total valid groups of size %d: %d\n", n, count);

    for (i = 0; i < n; i++) {
        free(table[i]);
    }
    free(table);
    TableNode *cur = seen_head;
    while (cur != NULL) {
        TableNode *next = cur->next;
        free(cur->flat);
        free(cur);
        cur = next;
    }
    return 0;
}
