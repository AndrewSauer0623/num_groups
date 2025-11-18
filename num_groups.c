/*
Program written by Andrew Sauer
Calculates the number of groups of size n (would not reccomend n > 5 until multithreading support and math magic are added)
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

/*
 * Function to check if a number is prime
 * This would be horrible if it was possible to run large n
 * Fortunately the rest of the code is FAR less efficient so it does not matter
 */
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
    /* holy nested loops! */
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



/*
 * This function is essentially straight ChatGPT as I got lost stuck
 *
 Compute a canonical flattened representation of a Cayley table up to isomorphism
 under relabelings that send the identity element to 0. This is done by
 considering all permutations of the non-identity elements, permuting rows,
 columns, and entries accordingly, and selecting the lexicographically smallest
 flattened table.
 Assumes the provided table is a valid group table (has identity, inverses, associativity).
*/
void compute_canonical_flat(int **table, int n, int *out_flat) {
    int i, j, a;

    int e = find_identity(table, n);
    if (e == -1) {
        /* Fallback: just flatten as-is (should not happen for valid groups) */
        flatten_table(table, n, out_flat);
        return;
    }

    /* Build list of elements other than identity */
    int m = n - 1;
    int *others = (int *)malloc(sizeof(int) * m);
    int idx = 0;
    for (a = 0; a < n; a++) {
        if (a != e) {
            others[idx++] = a;
        }
    }

    /* permutation over positions 0..m-1 that assigns new labels 1..n-1 */
    int *perm = (int *)malloc(sizeof(int) * m);
    for (i = 0; i < m; i++) perm[i] = i; /* start with 0..m-1 */

    /* Working buffers */
    int *best = (int *)malloc(sizeof(int) * n * n);
    int *candidate = (int *)malloc(sizeof(int) * n * n);
    int best_set = 0;

    do {
        /* Build mapping old -> new index in 0..n-1 with identity mapped to 0 */
        int *map_old_to_new = (int *)malloc(sizeof(int) * n);
        map_old_to_new[e] = 0;
        for (i = 0; i < m; i++) {
            int old_elem = others[i];
            int new_label = perm[i] + 1; /* labels 1..n-1 */
            map_old_to_new[old_elem] = new_label;
        }

        /* Produce fully permuted table and flatten to candidate (also wastes time) */
        for (i = 0; i < n; i++) {
            int ni = map_old_to_new[i];
            for (j = 0; j < n; j++) {
                int nj = map_old_to_new[j];
                int val = table[i][j];
                int nv = map_old_to_new[val];
                candidate[ni * n + nj] = nv;
            }
        }

        /* Update best if this candidate is lexicographically smaller */
        if (!best_set) {
            memcpy(best, candidate, sizeof(int) * n * n);
            best_set = 1;
        } else {
            int better = 0;
            for (i = 0; i < n * n; i++) {
                if (candidate[i] < best[i]) { better = 1; break; }
                if (candidate[i] > best[i]) { break; }
            }
            if (better) {
                memcpy(best, candidate, sizeof(int) * n * n);
            }
        }

        free(map_old_to_new);
    } while (next_permutation(perm, m));

    memcpy(out_flat, best, sizeof(int) * n * n);

    free(others);
    free(perm);
    free(best);
    free(candidate);
}

/* Check if a Cayley table is isomorphic to any previously stored table */
int is_new_group(int **table, int n) {
    int i;
    /* Compute canonical flat for this table */
    int *canon = (int *)malloc(sizeof(int) * n * n);
    if (canon == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    compute_canonical_flat(table, n, canon);

    /* compare with all stored canonical tables */
    TableNode *cur = seen_head;
    while (cur != NULL) {
        int match = 1;
        for (i = 0; i < n * n; i++) {
            if (cur->flat[i] != canon[i]) { match = 0; break; }
        }
        if (match) {
            free(canon);
            return 0; /* not new */
        }
        cur = cur->next;
    }

    /* if no match found, it is new */
    free(canon);
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
            /* store canonical representative so that future comparisons match */
            compute_canonical_flat(table, n, flat);
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
        printf("Error: n must be a positive integer.\n");
        return 1;
    }

    table = malloc(n * sizeof(int *));

    for (i = 0; i < n; i++) {
        table[i] = malloc(n * sizeof(int));
        if (table[i] == NULL) {
            printf("Memory allocation failed.\n");
            return 1;
        }

        // initialize each row to zero
        for (int j = 0; j < n; j++) {
            table[i][j] = 0;
        }
    }


    if (is_prime(n) || is_prime_square(n)) {
        must_be_abelian = 1;
        printf("n is prime or prime squared, all groups must be abelian.");
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
