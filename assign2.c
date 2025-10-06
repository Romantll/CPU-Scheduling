#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pslibrary.h"

static void usage(void){
    fprintf(stderr, "Usage: ./assign2 q x1 y1 z1 x2 y2 z2\n");
}

static int count_char(const char *s, char c){
    int n = 0;
    for (const char *p = s; *p; ++p) if (*p == c) n++;
    return n;
}

static int str_len_visible(const char *s){
    return (int)strlen(s);
}

static void print_block(const char *name, const char *s1, const char *s2){
    int w1 = count_char(s1, 'r');
    int w2 = count_char(s2, 'r');
    int R1 = count_char(s1, 'R');
    int R2 = count_char(s2, 'R');
    int L1 = str_len_visible(s1);
    int L2 = str_len_visible(s2);
    int longer = (L1 > L2) ? L1 : L2;
    double avgW = (w1 + w2) / 2.0;
    double util = longer > 0 ? (double)(R1 + R2) / (double)longer : 0.0;

    printf("\nScheduler %s:\n\n", name);
    printf("%s\n", s1);
    printf("%s\n", s2);
    printf("%d %d %.1f %.5f\n", w1, w2, avgW, util);
}

int main(int argc, char **argv){
    if (argc != 8){
        usage();
        return 1;
    }

    int q  = atoi(argv[1]);
    int x1 = atoi(argv[2]);
    int y1 = atoi(argv[3]);
    int z1 = atoi(argv[4]);
    int x2 = atoi(argv[5]);
    int y2 = atoi(argv[6]);
    int z2 = atoi(argv[7]);

    printf("CS 3733 / Assignment 2 / Roman Rendon / gjh148\n");
    printf("%d %d %d %d %d %d %d\n", q, x1, y1, z1, x2, y2, z2);

    // Buffers large enough for harness-sized tests
    static char s1[2000005];
    static char s2[2000005];

    //FCFS
    s1[0] = s2[0] = '\0';
    fcfs(s1, s2, x1, y1, z1, x2, y2, z2);
    print_block("FCFS", s1, s2);

    // SJF
    s1[0] = s2[0] = '\0';
    sjf(s1, s2, x1, y1, z1, x2, y2, z2);
    print_block("SJF", s1, s2);

    // PSJF
    s1[0] = s2[0] = '\0';
    psjf(s1, s2, x1, y1, z1, x2, y2, z2);
    print_block("PSJF", s1, s2);

    // RR
    s1[0] = s2[0] = '\0';
    rr(s1, s2, q, x1, y1, z1, x2, y2, z2);
    print_block("RR", s1, s2);

    return 0;
}
