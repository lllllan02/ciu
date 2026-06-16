#include <stdio.h>
#include <string.h>

#define maxn 1001
#define inf 0x3f3f3f3f

int dist[maxn][maxn];

void floyd(int n) {
    for (int k = 1; k <= n; k++) {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (dist[i][k] < inf && dist[k][j] < inf &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                }
            }
        }
    }
}

int main() {
    setbuf(stdout, NULL);

    int n = 3;
    memset(dist, inf, sizeof(dist));

    for (int i = 1; i <= n; i++) {
        dist[i][i] = 0;
    }

    // 简单图: 1 -2-> 2 -1-> 3, 1 -5-> 3
    // 1 到 3 最短距离为 3 (经 2)
    dist[1][2] = 2;
    dist[1][3] = 5;
    dist[2][3] = 1;

    floyd(n);

    printf("All-pairs shortest distances:\n");
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (dist[i][j] >= inf) {
                printf("  %d -> %d: inf\n", i, j);
            } else {
                printf("  %d -> %d: %d\n", i, j, dist[i][j]);
            }
        }
    }

    return 0;
}
