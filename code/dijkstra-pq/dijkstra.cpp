#include <iostream>
#include <queue>
#include <vector>

using namespace std;

const int maxn = 1001;
const int inf = 0x3f3f3f3f;

vector<pair<int, int>> G[maxn];
int dis[maxn];

void add_edge(int u, int v, int w) {
    G[u].emplace_back(v, w);
}

void dijkstra_pq(int s, int n) {
    fill(dis + 1, dis + n + 1, inf);
    dis[s] = 0;

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    pq.emplace(0, s);

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dis[u]) continue;

        for (auto [v, w] : G[u]) {
            if (dis[u] + w < dis[v]) {
                dis[v] = dis[u] + w;
                pq.emplace(dis[v], v);
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // 简单图: 1 -2-> 2 -1-> 3, 1 -5-> 3
    // 从 1 到 3 最短距离为 3 (经 2)
    add_edge(1, 2, 2);
    add_edge(1, 3, 5);
    add_edge(2, 3, 1);

    dijkstra_pq(1, 3);

    cout << "Shortest distances from node 1 (priority queue):\n";
    for (int i = 1; i <= 3; i++) {
        cout << "  to " << i << ": " << dis[i] << '\n';
    }

    return 0;
}
