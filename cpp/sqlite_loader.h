// #include <vector>
// #include <string>

// using namespace std;

// bool loadColumns(const string &dbPath, vector<int>& colA, vector<int>& colB);
// TODO perf report
#include <cstddef>
#include <sqlite3.h>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

bool load_typer_tuple(const string& dbPath, int (*compute_max_typer)(int*, int*, size_t), int& outMax, uint64_t& pureSeconds);

bool load_tectorwise_batch(const string& dbPath, int (*compute_max_tectorwise)(int*, int*, size_t), int& outMax, size_t batch_sz, uint64_t& pureSeconds);

bool load_tectorwise_simd_batch(const string &dbPath, int (*compute_max_tectorwise_simd)(int *, int *, size_t), int &outMax, size_t batch_sz, uint64_t& pureSeconds);