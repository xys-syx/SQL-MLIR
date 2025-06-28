#include "sqlite_loader.h"
#include <climits>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <chrono>
using clk = std::chrono::steady_clock;
using namespace std;

bool load_typer_tuple(const string &dbPath, int (*compute_max_typer)(int *, int *, size_t), int &outMax, uint64_t& pureSeconds) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        fprintf(stderr, "SQLite open error: %s\n", sqlite3_errmsg(db));
        return false;
    }

    const char* sql = "SELECT a, b FROM DATA;";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr)) {
        fprintf(stderr, "prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }


    vector<int> colA, colB;
    while (sqlite3_step(st) == SQLITE_ROW) {
        colA.push_back(sqlite3_column_int(st, 0));
        colB.push_back(sqlite3_column_int(st, 1));
    }
    sqlite3_finalize(st); 
    sqlite3_close(db);

    auto start = clk::now();
    outMax = compute_max_typer(colA.data(), colB.data(), colA.size());
    // pureSeconds = std::chrono::duration<double>(clk::now() - start).count();
    pureSeconds = std::chrono::duration_cast<std::chrono::microseconds>(clk::now() - start).count();
    return true;
}

bool load_tectorwise_batch(const string &dbPath, int (*compute_max_tectorwise)(int *, int *, size_t), int &outMax, size_t batch_sz, uint64_t& pureSeconds) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        fprintf(stderr, "SQLite open error: %s\n", sqlite3_errmsg(db));
        return false;
    }

    // sqlite3_stmt* stmtA = nullptr;
    // if (sqlite3_prepare_v2(db, "SELECT a FROM DATA;", -1, &stmtA, nullptr) != SQLITE_OK) {
    //     fprintf(stderr, "prepare A: %s\n", sqlite3_errmsg(db));
    //     sqlite3_close(db); 
    //     return false;
    // }

    // sqlite3_stmt* stmtB = nullptr;
    // if (sqlite3_prepare_v2(db, "SELECT b FROM DATA;", -1, &stmtB, nullptr) != SQLITE_OK) {
    //     fprintf(stderr, "prepare B: %s\n", sqlite3_errmsg(db));
    //     sqlite3_finalize(stmtA); 
    //     sqlite3_close(db); 
    //     return false;
    // }
    

    // vector<int> colA(batch_sz);
    // vector<int> colB(batch_sz);
    // vector<int> tailA;
    // vector<int> tailB;
    // tailA.reserve(1024);
    // tailB.reserve(1024);
    // int globalMax = INT_MIN;
    // bool ok = true;
    // while (true) {
    //     // dont need
    //     size_t loadedA = 0;
    //     for (; loadedA < batch_sz; loadedA++) {
    //         int rc = sqlite3_step(stmtA);
    //         if (rc == SQLITE_DONE) break;
    //         if (rc != SQLITE_ROW) {
    //             fprintf(stderr, "step A error: %s\n", sqlite3_errmsg(db));
    //             ok = false;
    //             break;
    //         }
    //         colA[loadedA] = sqlite3_column_int(stmtA, 0);
    //     }
    //     if (!ok || loadedA == 0)
    //         break;

    //     size_t loadedB = 0;
    //     for (; loadedB < batch_sz; loadedB++) {
    //         int rc = sqlite3_step(stmtB);
    //         if (rc == SQLITE_DONE) break;
    //         if (rc != SQLITE_ROW) {
    //             fprintf(stderr, "step B error: %s\n", sqlite3_errmsg(db));
    //             ok = false;
    //             break;
    //         }
    //         colA[loadedB] = sqlite3_column_int(stmtB, 0);
    //     }

    //     if (!ok) {
    //         break;
    //     }
    //     tailA.insert(tailA.end(), colA.begin(), colA.begin() + loadedA);
    //     tailB.insert(tailB.end(), colB.begin(), colB.begin() + loadedA);

    //     if (loadedA < batch_sz) break;
    // }
    // if (ok && !tailA.empty()) {
    //     auto start = clk::now();
    //     outMax = compute_max_tectorwise(tailA.data(), tailB.data(), tailA.size());
    //     pureSeconds = std::chrono::duration<double>(clk::now() - start).count();
    // } else {
    //     outMax = INT_MIN;
    // }

    // sqlite3_finalize(stmtA);
    // sqlite3_finalize(stmtB);
    // sqlite3_close(db);
    const char* sql = "SELECT a, b FROM DATA;";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr)) {
        fprintf(stderr, "prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    vector<int> colA, colB;
    while (sqlite3_step(st) == SQLITE_ROW) {
        colA.push_back(sqlite3_column_int(st, 0));
        colB.push_back(sqlite3_column_int(st, 1));
    }
    sqlite3_finalize(st); 
    sqlite3_close(db);

    auto start = clk::now();
    outMax = compute_max_tectorwise(colA.data(), colB.data(), colA.size());
    pureSeconds = std::chrono::duration_cast<std::chrono::microseconds>(clk::now() - start).count();


    return true;
}

bool load_tectorwise_simd_batch(const string &dbPath, int (*compute_max_tectorwise_simd)(int *, int *, size_t), int &outMax, size_t batch_sz, uint64_t& pureSeconds) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        fprintf(stderr, "SQLite open error: %s\n", sqlite3_errmsg(db));
        return false;
    }

    const char* sql = "SELECT a, b FROM DATA;";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr)) {
        fprintf(stderr, "prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    vector<int> colA, colB;
    while (sqlite3_step(st) == SQLITE_ROW) {
        colA.push_back(sqlite3_column_int(st, 0));
        colB.push_back(sqlite3_column_int(st, 1));
    }
    sqlite3_finalize(st); 
    sqlite3_close(db);

    auto start = clk::now();
    outMax = compute_max_tectorwise_simd(colA.data(), colB.data(), colA.size());
    pureSeconds = std::chrono::duration_cast<std::chrono::microseconds>(clk::now() - start).count();


    return true;
}