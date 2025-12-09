#include <bits/stdc++.h>
using namespace std;

/* ---------------- Data Structures ---------------- */

struct Edge {
    int to;
    double cost; // travel time or cost
};

struct HeapSalt {
    int id;
    int node;            // graph node where heap is located
    int productionTime;  // minutes from start of day
    int safeDuration;    // minutes safe before moisture damage
};

struct HeapWithUrgency {
    HeapSalt heap;
    int timeLeft;        // remaining safe time (can be negative if overdue)
    double bestTravel;   // best travel cost/time to nearest godown
    int bestGodown;      // node index of nearest godown
};

/* ---------------- Dijkstra: shortest path in weighted graph ---------------- */

vector<double> dijkstra(const vector<vector<Edge>>& graph, int src) {
    int n = (int)graph.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);

    using State = pair<double,int>; // (dist, node)
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[src] = 0.0;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        for (const auto& e : graph[u]) {
            int v = e.to;
            double nd = d + e.cost;
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}

/* ---------------- Merge Sort: sort heaps by urgency then travel ---------------- */

bool heapLess(const HeapWithUrgency& a, const HeapWithUrgency& b) {
    // First: less timeLeft -> more urgent
    if (a.timeLeft < b.timeLeft) return true;
    if (a.timeLeft > b.timeLeft) return false;
    // If same urgency, choose smaller travel cost
    return a.bestTravel < b.bestTravel;
}

void mergeHeaps(vector<HeapWithUrgency>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<HeapWithUrgency> L(n1), R(n2);

    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (heapLess(L[i], R[j])) arr[k++] = L[i++];
        else                       arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSortHeaps(vector<HeapWithUrgency>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortHeaps(arr, left, mid);
    mergeSortHeaps(arr, mid + 1, right);
    mergeHeaps(arr, left, mid, right);
}

/* ------------------------------ MAIN ------------------------------ */

int main() {
    // 1) Build a sample graph (nodes = junctions, pans, godowns)
    int N = 7; // 0..6
    vector<vector<Edge>> graph(N);

    auto addEdge = [&](int u, int v, double cost) {
        graph[u].push_back({v, cost});
        graph[v].push_back({u, cost}); // undirected
    };

    // Example roads with time cost (minutes)
    addEdge(0, 1, 4.0);
    addEdge(1, 2, 3.0);
    addEdge(2, 3, 5.0);
    addEdge(1, 4, 2.5);
    addEdge(4, 5, 4.5);
    addEdge(3, 6, 6.0);
    addEdge(5, 6, 2.0);

    // 2) Salt godowns (storage nodes)
    vector<int> godowns = {3, 5};

    // 3) Salt heaps
    vector<HeapSalt> heaps = {
        {0, 0,  60, 120},  // id, node, produced at t=60, safe 120 min
        {1, 1, 100,  90},
        {2, 2, 110,  80},
        {3, 4, 130,  60},
        {4, 6, 140,  50}
    };

    int currentTime = 180; // minutes from start of day

    // 4) Precompute distance from each node to EACH godown? Instead:
    //    We'll run Dijkstra from each godown and keep minimum.
    int n = (int)graph.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> bestDistToGodown(n, INF);
    vector<int> bestGodownForNode(n, -1);

    for (int g : godowns) {
        vector<double> dist = dijkstra(graph, g);
        for (int node = 0; node < n; ++node) {
            if (dist[node] < bestDistToGodown[node]) {
                bestDistToGodown[node] = dist[node];
                bestGodownForNode[node] = g;
            }
        }
    }

    // 5) Build HeapWithUrgency list
    vector<HeapWithUrgency> list;
    for (const auto& h : heaps) {
        int expiry = h.productionTime + h.safeDuration;
        int timeLeft = expiry - currentTime;
        double travel = bestDistToGodown[h.node];
        int godownNode = bestGodownForNode[h.node];
        list.push_back({h, timeLeft, travel, godownNode});
    }

    // 6) Sort heaps by urgency -> timeLeft, then travel cost, using Merge Sort
    if (!list.empty()) {
        mergeSortHeaps(list, 0, (int)list.size() - 1);
    }

    // 7) Print plan
    cout << "Salt Storage Optimization Plan\n\n";
    cout << "Current time: " << currentTime << " minutes\n\n";
    cout << left << setw(6) << "ID"
         << setw(6) << "Node"
         << setw(12) << "TimeLeft"
         << setw(12) << "BestGdn"
         << setw(12) << "Travel" << "\n";
    cout << string(48, '-') << "\n";

    for (const auto& hw : list) {
        cout << left << setw(6) << hw.heap.id
             << setw(6) << hw.heap.node
             << setw(12) << hw.timeLeft
             << setw(12) << hw.bestGodown
             << setw(12) << fixed << setprecision(2) << hw.bestTravel
             << "\n";
    }

    cout << "\nExplanation:\n";
    cout << "- Dijkstra is used (from each godown) to compute realistic travel time on roads.\n";
    cout << "- Merge Sort orders heaps by urgency (timeLeft) and then smaller travel cost.\n";
    cout << "- The farmer can move the top (most urgent) heaps to the suggested godowns first.\n";

    return 0;
}
