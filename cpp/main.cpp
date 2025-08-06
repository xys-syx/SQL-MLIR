#include "sqlite_loader.h"
#include <cstddef>
#include <cstdint>
#include <sqlite3.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <string>


using clk = std::chrono::high_resolution_clock;
namespace fs = std::filesystem;

extern int compute_max_typer(int*, int*, std::size_t);
extern int compute_max_tectorwise(int*, int*, std::size_t);
extern int compute_max_tectorwise_simd(int*, int*, std::size_t);

constexpr std::size_t START = 1'000;
constexpr std::size_t END   = 5'750'000;
constexpr std::size_t STEP  = 500;

constexpr const char* DB_FULL = "test.db";
const std::string DB_ABS = fs::absolute("test.db").string();

bool build_subset_db(const fs::path& dst, std::size_t rows) {
    if (fs::exists(dst)) fs::remove(dst);
    sqlite3* db = nullptr;
    if (sqlite3_open(dst.string().c_str(), &db) != SQLITE_OK) {
        std::cerr << "open subset failed: " << sqlite3_errmsg(db) << '\n';
        return false;
    }

    sqlite3_exec(db, "PRAGMA journal_mode=OFF;", nullptr,nullptr,nullptr);
    sqlite3_exec(db, "PRAGMA synchronous=OFF;",  nullptr,nullptr,nullptr);

    std::ostringstream oss;
    oss << "ATTACH DATABASE '" << DB_ABS << "' AS src;"
        << "CREATE TABLE DATA AS "
        << "SELECT a,b FROM src.DATA LIMIT " << rows << ";"
        << "DETACH src;";
    
        char* errMsg = nullptr;
    int rc = sqlite3_exec(db, oss.str().c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "sqlite_exec error: " << (errMsg ? errMsg : "unknown") << '\n';
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

int main() {
    std::ofstream csv("results.csv");
    csv << "rows, typer_s, tectorwise_s,tectorwise_simd_s\n";

    for (std::size_t N = START; N <= END; N += STEP) {
        // fs::path subset = fs::temp_directory_path() / ("subset_" + std::to_string(N) + ".db");
        
        fs::path subset = fs::current_path() / ("subset_" + std::to_string(N) + ".db");
        if (!build_subset_db(subset, N)) {
            std::cerr << "failed building subset for N=" << N << '\n';
            return 1;
        }

        uint64_t compTy = 0;
        int maxTy = 0;
        // load_typer_tuple(subset.string(), compute_max_typer, maxTy, compTy);
        
        uint64_t compTec = 0;
        int maxTec = 0;
        // load_tectorwise_batch(subset.string(), compute_max_tectorwise, maxTec, 1000, compTec);

        uint64_t compTecSimd = 0;
        int maxTecSimd = 0;
        // load_tectorwise_simd_batch(subset.string(), compute_max_tectorwise_simd, maxTecSimd, 1000, compTecSimd);
        
        int dummyMax;

        auto run3avg = [&](auto loader, uint64_t& avg, auto&&... fixedArgs) {
            uint64_t sum = 0;
            int outMax;
            for (int r = 0; r < 3; r++) {
                uint64_t t;
                loader(std::forward<decltype(fixedArgs)>(fixedArgs)..., t);
                sum += t;
            }
            avg = sum / 3;
        };
        std::vector<char> flush(64*1024*1024);
        std::fill(flush.begin(), flush.end(), 1);
        run3avg(load_typer_tuple, compTy, subset.string(), compute_max_typer, dummyMax);

        std::fill(flush.begin(), flush.end(), 1);
        run3avg(load_tectorwise_batch, compTec, subset.string(), compute_max_tectorwise, dummyMax, 1000);

        std::fill(flush.begin(), flush.end(), 1);
        run3avg(load_tectorwise_simd_batch, compTecSimd, subset.string(), compute_max_tectorwise_simd, dummyMax, 1000);

        // csv << N << ','
        //     << std::fixed << std::setprecision(3) << msTy << ','
        //     << msTe << '\n';

        double ty_s = compTy / 1e6;
        double tw_s = compTec / 1e6;
        double tw_simd_s = compTecSimd / 1e6;

        const uint64_t BYTES_TY = N * 8;
        const uint64_t BYTES_TW = N * 16;
        const uint64_t BYTES_SIMD = N * 16;
        double ty_bw = BYTES_TY / ty_s / 1e9;
        double tw_bw = BYTES_TW / tw_s / 1e9;
        double simd_bw = BYTES_SIMD / tw_simd_s / 1e9;

        csv << N << ',' << std::fixed << std::setprecision(9) 
                    << ty_s << ',' << tw_s <<
                    ',' << tw_simd_s << ','
                    << ty_bw << ',' << tw_bw << ',' << simd_bw << '\n';

        

        fs::remove(subset);
    }

    std::cout << "done\n";
    return 0;
}