#include <bits/stdc++.h>
using namespace std;

/* ------------ Data Structures ------------ */

struct Pan {
    int id;
    double x, y;   // coordinates (simple 2D)
    double area;   // in m^2
    bool affected; // whether this pan is affected by storm
};

/* ------------ Rabin–Karp: detect dangerous pattern in sensor data ------------ */
// Here we treat sensor data as an integer sequence (e.g., pressure readings).

bool rabinKarpMatch(const vector<int>& text, const vector<int>& pattern) {
    if (pattern.empty() || text.size() < pattern.size()) return false;

    const long long base = 257;
    const long long mod  = 1000000007LL;

    int n = text.size();
    int m = pattern.size();

    long long patHash = 0, txtHash = 0, power = 1;

    // base^(m-1)
    for (int i = 0; i < m - 1; ++i) {
        power = (power * base) % mod;
    }

    // initial hash
    for (int i = 0; i < m; ++i) {
        patHash = (patHash * base + pattern[i]) % mod;
        txtHash = (txtHash * base + text[i]) % mod;
    }

    for (int i = 0; i <= n - m; ++i) {
        if (patHash == txtHash) {
            bool same = true;
            for (int j = 0; j < m; ++j) {
                if (text[i + j] != pattern[j]) {
                    same = false;
                    break;
                }
            }
            if (same) return true;
        }
        // roll hash
        if (i < n - m) {
            long long remove = (text[i] * power) % mod;
            txtHash = (txtHash - remove + mod) % mod;
            txtHash = (txtHash * base + text[i + m]) % mod;
        }
    }
    return false;
}

/* ------------ Build adjacency (graph) of neighboring pans ------------ */

vector<vector<int>> buildAdjacency(const vector<Pan>& pans, double neighborDist) {
    int n = (int)pans.size();
    vector<vector<int>> adj(n);
    double nd2 = neighborDist * neighborDist;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double dx = pans[i].x - pans[j].x;
            double dy = pans[i].y - pans[j].y;
            double d2 = dx * dx + dy * dy;
            if (d2 <= nd2) {
                adj[i].push_back(j);
                adj[j].push_back(i);
            }
        }
    }
    return adj;
}

/* ------------ BFS: propagate risk from directly-hit pans ------------ */

vector<int> bfsAffected(const vector<vector<int>>& adj,
                        const vector<int>& initial,
                        int n) {
    vector<bool> visited(n, false);
    queue<int> q;
    for (int idx : initial) {
        if (!visited[idx]) {
            visited[idx] = true;
            q.push(idx);
        }
    }
    vector<int> affected;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        affected.push_back(u);
        for (int v : adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                q.push(v);
            }
        }
    }
    return affected;
}

/* ------------ Merge Sort: sort pans by area (descending) ------------ */

bool panLess(const Pan& a, const Pan& b) {
    // we want descending order, so "less" means larger area
    return a.area > b.area;
}

void mergePans(vector<Pan>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<Pan> L(n1), R(n2);

    for (int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for (int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (panLess(L[i], R[j])) arr[k++] = L[i++];
        else                      arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSortPans(vector<Pan>& arr, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSortPans(arr, left, mid);
    mergeSortPans(arr, mid + 1, right);
    mergePans(arr, left, mid, right);
}

/* ----------------------------- MAIN ----------------------------- */

int main() {
    // 1) Simulated sensor data & danger pattern
    vector<int> sensorData   = {1010, 1008, 1005, 1001, 995, 990, 988};
    vector<int> dangerPattern = {1008, 1005, 1001, 995}; // rapid pressure drop

    bool stormDetected = rabinKarpMatch(sensorData, dangerPattern);

    if (!stormDetected) {
        cout << "No dangerous weather pattern detected. No impact prediction.\n";
        return 0;
    }

    cout << "Dangerous weather pattern detected! Predicting impact on salt pans...\n\n";

    // 2) Example pans (id, x, y, area)
    vector<Pan> pans = {
        {0, 10, 10, 1000, false},
        {1, 12, 11, 1500, false},
        {2, 20, 20, 1300, false},
        {3, 25, 19, 2000, false},
        {4, 30, 35, 1800, false},
        {5, 40, 40, 2200, false}
    };

    int n = (int)pans.size();

    // 3) Storm center + radius
    double cx = 15.0, cy = 15.0, radius = 12.0;
    double r2 = radius * radius;

    // Find directly affected pans (inside circle)
    vector<int> initialAffectedIdx;
    for (int i = 0; i < n; ++i) {
        double dx = pans[i].x - cx;
        double dy = pans[i].y - cy;
        double d2 = dx * dx + dy * dy;
        if (d2 <= r2) {
            pans[i].affected = true;
            initialAffectedIdx.push_back(i);
        }
    }

    cout << "Directly affected pans (inside storm radius):\n";
    if (initialAffectedIdx.empty()) {
        cout << "  None directly inside storm region.\n\n";
    } else {
        for (int idx : initialAffectedIdx) {
            cout << "  Pan " << pans[idx].id
                 << " at (" << pans[idx].x << "," << pans[idx].y
                 << "), area = " << pans[idx].area << " m^2\n";
        }
        cout << "\n";
    }

    // 4) Build adjacency of pans (neighbors within distance) and BFS
    double neighborDist = 8.0;
    vector<vector<int>> adj = buildAdjacency(pans, neighborDist);

    vector<int> allAffectedIdx;
    if (!initialAffectedIdx.empty()) {
        allAffectedIdx = bfsAffected(adj, initialAffectedIdx, n);
        for (int idx : allAffectedIdx) pans[idx].affected = true;
    }

    // 5) Collect affected pans and sort by area (descending) using Merge Sort
    vector<Pan> affectedPans;
    for (const auto& p : pans) {
        if (p.affected) affectedPans.push_back(p);
    }

    if (!affectedPans.empty()) {
        mergeSortPans(affectedPans, 0, (int)affectedPans.size() - 1);
    }

    double totalArea = 0.0;
    for (const auto& p : affectedPans) totalArea += p.area;

    cout << "All affected pans (after BFS propagation), sorted by area:\n";
    for (const auto& p : affectedPans) {
        cout << "  Pan " << p.id
             << " | area = " << p.area
             << " m^2 at (" << p.x << "," << p.y << ")\n";
    }
    cout << "\nTotal affected area (if no action taken): "
         << totalArea << " m^2\n";

    return 0;
}
