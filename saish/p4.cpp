#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to;
    double cost; // distance or time along this road
};

/* --------- Multi-source Dijkstra: from all current bin nodes --------- */

void multiSourceDijkstra(
    const vector<vector<Edge>>& graph,
    const vector<int>& bins,          // current bin locations
    vector<double>& dist,             // output: distance to nearest bin
    vector<int>& nearestBin           // output: which bin is nearest
) {
    int n = (int)graph.size();
    const double INF = 1e18;
    dist.assign(n, INF);
    nearestBin.assign(n, -1);

    using State = pair<double,int>; // (distance, node)
    priority_queue<State, vector<State>, greater<State>> pq;

    // Initialize all current bins as sources
    for (int b : bins) {
        dist[b] = 0.0;
        nearestBin[b] = b;
        pq.push({0.0, b});
    }

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;

        for (const auto& e : graph[u]) {
            int v = e.to;
            double nd = d + e.cost;
            if (nd < dist[v]) {
                dist[v] = nd;
                nearestBin[v] = nearestBin[u]; // inherit source bin
                pq.push({nd, v});
            }
        }
    }
}

/* ---------------------- Demo: bin placement ---------------------- */

int main() {
    // --- 1. Build a sample residential graph ---
    int N = 8; // 8 localities/junctions numbered 0..7
    vector<vector<Edge>> graph(N);

    auto addRoad = [&](int u, int v, double cost) {
        graph[u].push_back({v, cost});
        graph[v].push_back({u, cost}); // undirected
    };

    // Example roads (you can change distances)
    addRoad(0, 1, 3.0);
    addRoad(1, 2, 4.0);
    addRoad(2, 3, 2.5);
    addRoad(1, 4, 5.0);
    addRoad(4, 5, 3.5);
    addRoad(5, 6, 2.0);
    addRoad(6, 7, 4.0);
    addRoad(3, 7, 3.0);

    // --- 2. We want K dustbins ---
    int K = 4; // e.g., 4 bin locations

    // Start with one initial bin (could be central area, here node 0)
    vector<int> bins;
    bins.push_back(0);

    vector<double> dist;
    vector<int> nearestBin;

    // --- 3. Greedily add bins at farthest localities ---
    while ((int)bins.size() < K) {
        // Run multi-source Dijkstra from all current bins
        multiSourceDijkstra(graph, bins, dist, nearestBin);

        // Find the locality farthest from any existing bin
        int farNode = -1;
        double farDist = -1.0;

        for (int node = 0; node < N; ++node) {
            // skip if already a bin location
            if (find(bins.begin(), bins.end(), node) != bins.end()) continue;

            if (dist[node] > farDist) {
                farDist = dist[node];
                farNode = node;
            }
        }

        if (farNode == -1) break; // all nodes already bins or isolated

        bins.push_back(farNode);
    }

    // --- 4. Final assignment: which locality uses which bin? ---
    multiSourceDijkstra(graph, bins, dist, nearestBin);

    cout << "Chosen dustbin locations (nodes): ";
    for (int b : bins) cout << b << " ";
    cout << "\n\n";

    cout << "Locality -> nearest bin (and distance):\n";
    for (int node = 0; node < N; ++node) {
        cout << "  Locality " << node
             << " -> Bin at node " << nearestBin[node]
             << " (distance " << fixed << setprecision(2)
             << dist[node] << ")\n";
    }

    return 0;
}
