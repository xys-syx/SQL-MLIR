#include <cstddef>
#include <climits>

using namespace std;
// SELECT MAX(A+B) FROM DATA
int compute_max_typer(int* colA, int* colB, size_t N) {
    int max_val = INT_MIN;
    for (size_t i = 0; i < N; i++) {
        int val = colA[i] + colB[i];
        if (val > max_val) {
            max_val = val;
        }
    }
    return max_val;
}