#include <bits/stdc++.h>
using namespace std;

/* -------------------- Data structures -------------------- */

struct Node {
    int id;
    double x, y; // coordinates (or could be replaced by actual road node ids)
};

struct Edge {
    int u, v;
    double w; // weight (distance)
};

// For adjacency list (MST)
struct AdjEdge {
    int to;
    double w;
};

/* -------------------- Merge Sort for edges -------------------- */

void mergeEdges(vector<Edge>& a, int L, int M, int R) {
    int n1 = M - L + 1;
    int n2 = R - M;
    vector<Edge> left(n1), right(n2);
    for (int i = 0; i < n1; ++i) left[i] = a[L + i];
    for (int j = 0; j < n2; ++j) right[j] = a[M + 1 + j];
    int i = 0, j = 0, k = L;
    while (i < n1 && j < n2) {
        if (left[i].w <= right[j].w) a[k++] = left[i++];
        else a[k++] = right[j++];
    }
    while (i < n1) a[k++] = left[i++];
    while (j < n2) a[k++] = right[j++];
}

void mergeSortEdges(vector<Edge>& a, int L, int R) {
    if (L >= R) return;
    int M = L + (R - L) / 2;
    mergeSortEdges(a, L, M);
    mergeSortEdges(a, M+1, R);
    mergeEdges(a, L, M, R);
}

/* -------------------- Union-Find (DSU) -------------------- */

struct DSU {
    int n;
    vector<int> parent, rankv;
    DSU(int n=0): n(n), parent(n), rankv(n,0) {
        for (int i=0;i<n;++i) parent[i]=i;
    }
    int find(int x) {
        if (parent[x]==x) return x;
        return parent[x]=find(parent[x]);
    }
    bool unite(int a,int b) {
        a=find(a); b=find(b);
        if (a==b) return false;
        if (rankv[a]<rankv[b]) swap(a,b);
        parent[b]=a;
        if (rankv[a]==rankv[b]) rankv[a]++;
        return true;
    }
};

/* -------------------- Dijkstra (priority queue) -------------------- */

vector<double> dijkstraOnGraph(const vector<vector<AdjEdge>>& adj, int src) {
    int n = adj.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);
    using P = pair<double,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[src]=0.0; pq.push({0.0, src});
    while(!pq.empty()){
        auto [d,u]=pq.top(); pq.pop();
        if (d>dist[u]) continue;
        for (auto &e: adj[u]){
            int v=e.to; double nd = d + e.w;
            if (nd < dist[v]) { dist[v]=nd; pq.push({nd,v}); }
        }
    }
    return dist;
}

/* -------------------- Utility: euclidean distance -------------------- */

double euclid(const Node& a, const Node& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

/* -------------------- Main: build MST and analyze -------------------- */

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Sample input: nodes (0 is gas source)
    // Replace or extend nodes with real coordinates or actual road node ids.
    vector<Node> nodes = {
        {0, 0.0, 0.0},   // Gas source at origin
        {1, 2.3, 1.1},
        {2, 4.0, -0.5},
        {3, -1.2, 3.0},
        {4, 3.5, 3.2},
        {5, -2.0, -1.5},
        {6, 1.0, -3.0}
    };

    int N = nodes.size();

    // 1) Build candidate edges (here complete graph with Euclidean weights).
    // For larger real maps, you would only add feasible road edges with given distances.
    vector<Edge> edges;
    for (int i=0;i<N;++i){
        for (int j=i+1;j<N;++j){
            double w = euclid(nodes[i], nodes[j]);
            edges.push_back({i,j,w});
        }
    }

    // 2) Sort edges by weight using Merge Sort
    mergeSortEdges(edges, 0, (int)edges.size()-1);

    // 3) Kruskal's MST
    DSU dsu(N);
    vector<Edge> mstEdges;
    double totalLength = 0.0;
    for (const auto &e : edges) {
        if (dsu.unite(e.u, e.v)) {
            mstEdges.push_back(e);
            totalLength += e.w;
            if ((int)mstEdges.size() == N-1) break;
        }
    }

    // 4) Build adjacency list for MST
    vector<vector<AdjEdge>> mstAdj(N);
    for (const auto &e : mstEdges) {
        mstAdj[e.u].push_back({e.v, e.w});
        mstAdj[e.v].push_back({e.u, e.w});
    }

    // 5) Dijkstra from gas source (node 0) on MST to compute distances (proxy for pressure drop)
    vector<double> dist = dijkstraOnGraph(mstAdj, 0);

    // 6) Output results
    cout << fixed << setprecision(3);
    cout << "MST edges (pipelines to lay):\n";
    for (const auto &e : mstEdges) {
        cout << "  " << e.u << " -- " << e.v << "  (length = " << e.w << ")\n";
    }
    cout << "\nTotal pipeline length (MST): " << totalLength << "\n\n";

    cout << "Distance from source (node 0) to each node along MST:\n";
    double maxd = 0.0; int idxMax = -1;
    for (int i=0;i<N;++i) {
        cout << "  Node " << i << " : " << dist[i] << "\n";
        if (dist[i] > maxd) { maxd = dist[i]; idxMax = i; }
    }
    cout << "\nMaximum delivery distance from source = " << maxd
         << " (node " << idxMax << ")\n";

    // 7) Simple suggestion logic (threshold): if maxd too large, flag for pressure booster
    double threshold = 6.0; // example threshold in same units as distance
    if (maxd > threshold) {
        cout << "\nNote: maximum distance exceeds threshold (" << threshold << ").\n";
        cout << "Consider adding a booster station or re-evaluating pipeline placement.\n";
    } else {
        cout << "\nNetwork distances are within acceptable threshold.\n";
    }

    return 0;
}
