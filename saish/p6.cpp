#include <bits/stdc++.h>
using namespace std;

struct Edge { int to; double w; };
struct AssignResult { int slotNode; double dist; };

// Dijkstra from source, returns distances vector
vector<double> dijkstra(const vector<vector<Edge>>& g, int src) {
    int n = g.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);
    using P = pair<double,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[src] = 0.0; pq.push({0.0, src});
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto &e : g[u]) {
            int v = e.to; double nd = d + e.w;
            if (nd < dist[v]) { dist[v] = nd; pq.push({nd, v}); }
        }
    }
    return dist;
}

// Find nearest free slot node given entry node.
// slots = list of slot nodes; occupancy maps node -> bool (true if occupied)
AssignResult assignNearestSlot(const vector<vector<Edge>>& graph,
                               int entryNode,
                               const vector<int>& slots,
                               const unordered_map<int,bool>& occupied)
{
    AssignResult res = {-1, 1e18};
    vector<double> dist = dijkstra(graph, entryNode);

    // Option A (simple): scan all slots and pick nearest free
    for (int slotNode : slots) {
        auto it = occupied.find(slotNode);
        bool isOcc = (it != occupied.end() ? it->second : false);
        if (!isOcc && dist[slotNode] < res.dist) {
            res.slotNode = slotNode;
            res.dist = dist[slotNode];
        }
    }
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // --- Build sample graph ---
    // Nodes 0..9 (example)
    int N = 10;
    vector<vector<Edge>> graph(N);
    auto addEdge = [&](int u, int v, double w) {
        graph[u].push_back({v, w});
        graph[v].push_back({u, w});
    };

    // sample connections (bidirectional small campus)
    addEdge(0,1,2.0); addEdge(1,2,1.5); addEdge(2,3,2.2);
    addEdge(1,4,2.5); addEdge(4,5,1.0); addEdge(5,6,2.0);
    addEdge(2,7,3.0); addEdge(7,8,1.2); addEdge(8,9,2.3);
    addEdge(3,9,1.8);

    // --- Define parking slot nodes and entries ---
    vector<int> slots = {2, 4, 6, 8};   // nodes that are parking slots
    vector<int> entries = {0, 5};       // possible vehicle entry nodes

    // occupancy map: node -> occupied?
    unordered_map<int,bool> occupied;
    for (int s : slots) occupied[s] = false; // all free initially

    // Simulate arrivals
    cout << "Simulating arrivals:\n";

    vector<pair<int,string>> arrivals = {
        {0,"Car-A"}, {5,"Car-B"}, {0,"Car-C"}, {5,"Car-D"}, {0,"Car-E"}
    };

    // Map to remember which car in which slot (for releases)
    unordered_map<string,int> carToSlot;

    for (auto &p : arrivals) {
        int entry = p.first;
        string car = p.second;
        AssignResult a = assignNearestSlot(graph, entry, slots, occupied);
        if (a.slotNode == -1) {
            cout << car << " arrived at entry " << entry << " -> NO FREE SLOT\n";
        } else {
            cout << car << " arrived at entry " << entry
                 << " -> assigned slot node " << a.slotNode
                 << " at dist " << fixed << setprecision(2) << a.dist << "\n";
            occupied[a.slotNode] = true;
            carToSlot[car] = a.slotNode;
        }
    }

    // Simulate some departures
    cout << "\nSimulating departures:\n";
    vector<string> departures = {"Car-B", "Car-A"};
    for (auto &car : departures) {
        if (carToSlot.find(car) == carToSlot.end()) {
            cout << car << " not found\n";
            continue;
        }
        int slotNode = carToSlot[car];
        occupied[slotNode] = false;   // free it
        carToSlot.erase(car);
        cout << car << " left, freed slot node " << slotNode << "\n";
    }

    // New arrival after frees
    cout << "\nNew arrival after frees:\n";
    AssignResult a = assignNearestSlot(graph, 0, slots, occupied);
    if (a.slotNode == -1) cout << "No free slot\n";
    else cout << "NewCar assigned slot " << a.slotNode << " at dist " << a.dist << "\n";

    return 0;
}
