#include <bits/stdc++.h>
using namespace std;

/* ------------- Data Structures ------------- */

struct Edge {
    int to;
    double timeCost; // travel time along this road (e.g. minutes)
};

struct Worker {
    int id;
    string name;
    string serviceType; // e.g. "plumber", "electrician"
    int node;           // node where the worker is located in the graph
    double price;       // base service charge (excluding travel)
    double travelTime;  // computed using Dijkstra from user node
};

/* ------------- Dijkstra: shortest travel time in weighted graph ------------- */

vector<double> dijkstra(const vector<vector<Edge>>& graph, int src) {
    int n = (int)graph.size();
    const double INF = numeric_limits<double>::infinity();
    vector<double> dist(n, INF);

    using State = pair<double,int>; // (time, node)
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[src] = 0.0;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [curTime, u] = pq.top();
        pq.pop();
        if (curTime > dist[u]) continue;

        for (const auto& e : graph[u]) {
            int v = e.to;
            double nd = curTime + e.timeCost;
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}

/* ------------- Merge Sort: sort workers by price then travelTime ------------- */

bool workerLess(const Worker& a, const Worker& b) {
    if (a.price < b.price) return true;
    if (a.price > b.price) return false;
    // if price equal, sort by travel time (faster first)
    return a.travelTime < b.travelTime;
}

void mergeWorkers(vector<Worker>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<Worker> L(n1), R(n2);

    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (workerLess(L[i], R[j])) arr[k++] = L[i++];
        else                        arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSortWorkers(vector<Worker>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortWorkers(arr, left, mid);
    mergeSortWorkers(arr, mid + 1, right);
    mergeWorkers(arr, left, mid, right);
}

/* ------------- MAIN: Demo of the technician app logic ------------- */

int main() {
    // 1) Build a sample city graph (nodes 0..6)
    int N = 7;
    vector<vector<Edge>> graph(N);

    auto addRoad = [&](int u, int v, double timeCost) {
        graph[u].push_back({v, timeCost});
        graph[v].push_back({u, timeCost}); // undirected road
    };

    // Roads with realistic time (minutes) or cost
    addRoad(0, 1, 5.0);
    addRoad(1, 2, 8.0);
    addRoad(1, 3, 3.0);
    addRoad(2, 4, 6.0);
    addRoad(3, 4, 4.0);
    addRoad(3, 5, 7.0);
    addRoad(4, 6, 5.0);
    addRoad(5, 6, 2.0);

    // 2) User’s location (node index in the graph)
    int userNode = 0;

    // 3) Available workers (id, name, serviceType, node, price)
    vector<Worker> workers = {
        {0, "Ramesh", "plumber",     2, 300.0, 0.0},
        {1, "Suresh", "electrician", 4, 250.0, 0.0},
        {2, "Anita",  "plumber",     5, 280.0, 0.0},
        {3, "Vijay",  "carpenter",   6, 260.0, 0.0},
        {4, "Kiran",  "electrician", 3, 250.0, 0.0}
    };

    // Filter workers by requested service type (e.g., plumber)
    string requestedService = "plumber";
    vector<Worker> filtered;
    for (auto w : workers) {
        if (w.serviceType == requestedService) {
            filtered.push_back(w);
        }
    }

    if (filtered.empty()) {
        cout << "No workers available for service: " << requestedService << "\n";
        return 0;
    }

    // 4) Run Dijkstra from user node to get travel time to all nodes
    vector<double> dist = dijkstra(graph, userNode);
    const double INF = numeric_limits<double>::infinity();

    // Assign travelTime to each filtered worker
    for (auto& w : filtered) {
        w.travelTime = dist[w.node];
        if (w.travelTime == INF) {
            w.travelTime = 1e9; // large number for unreachable
        }
    }

    // 5) Sort filtered workers using custom Merge Sort
    mergeSortWorkers(filtered, 0, (int)filtered.size() - 1);

    // 6) Display sorted worker list (by price, then travel time)
    cout << "User node: " << userNode << "\n";
    cout << "Requested service: " << requestedService << "\n\n";

    cout << left << setw(4) << "ID"
         << setw(12) << "Name"
         << setw(12) << "Service"
         << setw(8) << "Node"
         << setw(10) << "Price"
         << setw(12) << "Time(min)" << "\n";

    cout << string(58, '-') << "\n";

    for (const auto& w : filtered) {
        cout << left << setw(4) << w.id
             << setw(12) << w.name
             << setw(12) << w.serviceType
             << setw(8) << w.node
             << setw(10) << fixed << setprecision(2) << w.price
             << setw(12) << fixed << setprecision(2) << w.travelTime
             << "\n";
    }

    cout << "\nExplanation:\n";
    cout << "- Dijkstra computes realistic travel time on the road graph from the user to each worker.\n";
    cout << "- Merge Sort ranks workers by price, then by travel time.\n";
    cout << "- The user can then pick a cheaper but farther worker, or a closer one, as they prefer.\n";

    return 0;
}
