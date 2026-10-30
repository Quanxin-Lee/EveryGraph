#include <latch>
#include <numeric>
#include <span>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>

namespace {

std::vector<int> make_owned_snapshot() {
    std::vector<int> source{1, 2, 3, 4};
    const std::span<const int> view{source};
    std::vector<int> snapshot(view.begin(), view.end());
    source[0] = 99;
    return snapshot;
}

}

int main() {
    auto snapshot = make_owned_snapshot();
    if (snapshot.size() != 4 || snapshot[0] != 1) {
        return 1;
    }

    std::latch stop_ready{1};
    int sum = 0;
    bool stop_observed = false;
    bool stop_accepted = false;
    {
        std::jthread worker([values = std::move(snapshot), &stop_ready, &sum,
                             &stop_observed](std::stop_token stop) {
            stop_ready.wait();
            const std::span<const int> view{values};
            sum = std::accumulate(view.begin(), view.end(), 0);
            stop_observed = stop.stop_possible() && stop.stop_requested();
        });
        stop_accepted = worker.request_stop();
        stop_ready.count_down();
    }

    if (!stop_accepted) {
        return 2;
    }
    if (!stop_observed) {
        return 3;
    }
    if (sum != 10) {
        return 4;
    }
    return 0;
}
