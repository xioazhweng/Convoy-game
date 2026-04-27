#include "repository/include/MissionRepositoryImpl.h"
#include "service/mission_service/include/MissionServiceImpl.h"
#include "strategy/AttackStrategy.h"
#include "ship/military_ship/include/MilitaryShip.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>


using SteadyClock = std::chrono::steady_clock;

namespace {
    struct Sample {
        std::size_t pirates = 0;
        double seq_us = 0.0;
        double par_us = 0.0;
    };

    struct ThreadedSample {
        std::size_t pirates = 0;
        std::array<double, 5> par_us{};
    };


    std::uint64_t now_us() {
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(SteadyClock::now().time_since_epoch()).count());
    }

    MissionRepositoryImpl make_repo(std::size_t pirates, std::size_t imperials) {
        Mission_params mp{};
        MissionRepositoryImpl repo(mp);

        for (std::size_t i = 0; i < pirates; ++i) {
            auto ship = std::make_unique<MilitaryShip>(
                "P" + std::to_string(i),
                100,
                50,
                1'000'000,
                1,
                "pirate",
                1.0);
            repo.add_ship(std::move(ship), Point(static_cast<int>(2 * i), std::rand() % pirates), true);
        }

        for (std::size_t i = 0; i < imperials; ++i) {
            auto ship = std::make_unique<MilitaryShip>(
                "I" + std::to_string(i),
                100,
                50,
                1'000'000,
                1,
                "imperial",
                1.0);
            repo.add_ship(std::move(ship), Point(static_cast<int>(2 * i + 1), std::rand() % imperials), false);
        }

        return repo;
    }

    template <class F>
    double measure_avg_us(std::size_t repeats, F&& f) {
        std::vector<std::uint64_t> times;
        times.reserve(repeats);
        for (std::size_t r = 0; r < repeats; ++r) {
            const auto t0 = now_us();
            f();
            const auto t1 = now_us();
            times.push_back(t1 - t0);
        }

        const double sum = std::accumulate(times.begin(), times.end(), 0.0);
        return sum / static_cast<double>(times.size());
    }

    bool save_png_with_gnuplot(const std::vector<Sample>& samples,
                                const std::string& csv_path,
                                const std::string& gp_path,
                                const std::string& png_path) {
        {
            std::ofstream csv(csv_path);
            if (!csv) {
                std::cerr << "Failed to open CSV for writing: " << csv_path << "\n";
                return false;
     }
            csv << "pirates,seq_us,par_us\n";
            for (const auto& s : samples) {
                csv << s.pirates << "," << std::fixed << std::setprecision(3) << s.seq_us << "," << s.par_us << "\n";
            }
        }

        {
            std::ofstream gp(gp_path);
            if (!gp) {
                std::cerr << "Failed to open gnuplot script for writing: " << gp_path << "\n";
                return false;
            }

            gp << "set terminal pngcairo size 1280,720\n";
            gp << "set output '" << png_path << "'\n";
            gp << "set datafile separator ','\n";
            gp << "set title 'pirate_attack: sequential vs parallel'\n";
            gp << "set xlabel 'Pirate ships (N)'\n";
            gp << "set ylabel 'Time (us)'\n";
            gp << "set grid\n";
            gp << "set key left top\n";
            gp << "plot \\\n   '" << csv_path << "' using 1:3 with lines lw 2 lc rgb 'red' title 'parallel', \\\n   '" << csv_path << "' using 1:2 with lines lw 2 lc rgb 'blue' title 'sequential'\n";
        }

        
        const std::string cmd = "gnuplot '" + gp_path + "'";
        const int rc = std::system(cmd.c_str());
        if (rc != 0) {
            std::cerr << "gnuplot failed (exit code=" << rc << "). Command: " << cmd << "\n";
            return false;
        }
     return true;
    }

    bool gnu_plot(const std::vector<ThreadedSample>& samples,
                  const std::array<unsigned, 5>& thread_counts,
                  unsigned logical_cores,
                  const std::string& csv_path,
                  const std::string& gp_path,
                  const std::string& png_path) {
        {
            std::ofstream csv(csv_path);
            if (!csv) {
                std::cerr << "Failed to open CSV for writing: " << csv_path << "\n";
                return false;
            }

            csv << "pirates";
            for (unsigned tc : thread_counts) {
                csv << ",par_" << tc;
            }
            csv << "\n";

            for (const auto& s : samples) {
                csv << s.pirates;
                for (double v : s.par_us) {
                    csv << "," << std::fixed << std::setprecision(3) << v;
                }
                csv << "\n";
            }
        }

        {
            std::ofstream gp(gp_path);
            if (!gp) {
                std::cerr << "Failed to open gnuplot script for writing: " << gp_path << "\n";
                return false;
            }

            gp << "set terminal pngcairo size 1280,720\n";
            gp << "set output '" << png_path << "'\n";
            gp << "set datafile separator ','\n";
            gp << "set title 'pirate_attack_parallel: 5 lines (logical cores=" << logical_cores << ")'\n";
            gp << "set xlabel 'Pirate ships (N)'\n";
            gp << "set ylabel 'Time (us)'\n";
            gp << "set grid\n";
            gp << "set key left top\n";

            gp << "plot \\\n   '" << csv_path << "' using 1:2 with lines lw 2 title '1 thread', \\\n   '" << csv_path << "' using 1:3 with lines lw 2 title '0.5 * thread_num (" << thread_counts[1] << ")', \\\n   '" << csv_path << "' using 1:4 with lines lw 2 title '1 * thread_num (" << thread_counts[2] << ")', \\\n   '" << csv_path << "' using 1:5 with lines lw 2 title '2 * thread_num (" << thread_counts[3] << ")', \\\n   '" << csv_path << "' using 1:6 with lines lw 2 title '4 * thread_num (" << thread_counts[4] << ")'\n";
        }

        const std::string cmd = "gnuplot '" + gp_path + "'";
        const int rc = std::system(cmd.c_str());
        if (rc != 0) {
            std::cerr << "gnuplot failed (exit code=" << rc << "). Command: " << cmd << "\n";
            return false;
        }
        return true;
    }
}

int main(void) {
    std::srand(std::time(nullptr));
    std::size_t max_pirates = 10000;
    std::size_t step = 250;
    std::size_t repeats = 5;
    unsigned threads = 16; 

    const unsigned range = 3;
    const unsigned damage = 10;

    const unsigned logical_cores = std::max(1u, std::thread::hardware_concurrency());
    const std::array<unsigned, 5> thread_counts{
        1u,
        std::max(1u, logical_cores / 2u),
        std::max(1u, logical_cores),
        std::max(1u, logical_cores * 2u),
        std::max(1u, logical_cores * 4u),
    };

    std::cout << " step =" << step
              << " repeats =" << repeats
              << " threads =" << (threads == 0 ? std::string("auto") : std::to_string(threads))
              << " logical_cores =" << logical_cores
              << "\n\n";

    std::vector<Sample> samples;
    std::vector<ThreadedSample> threaded_samples;
    for (std::size_t n = step; n <= max_pirates; n += step) {
        auto repo = make_repo(n, n);
        DefaultAttackStrategy strat(repo);
        MissionServiceImpl svc(repo, strat);

        const double seq_us = measure_avg_us(repeats, [&]() {
            (void)svc.pirate_attack(range, 0);
        });

        const double par_us = measure_avg_us(repeats, [&]() {
            MissionServiceImpl svc(repo, strat);
            (void)svc.pirate_attack_parallel(range, 0, threads);
        });

        ThreadedSample ts;
        ts.pirates = n;
        for (std::size_t i = 0; i < ts.par_us.size(); ++i) {
            const unsigned tc = thread_counts[i];
            ts.par_us[i] = measure_avg_us(repeats, [&]() {
                MissionServiceImpl svc(repo, strat);
                (void)svc.pirate_attack_parallel(range, 0, tc);
            });
        }

        samples.push_back(Sample{n, seq_us, par_us});
        threaded_samples.push_back(ts);
        std::cout << "N=" << std::setw(6) << n
                  << " | seq=" << std::setw(10) << std::fixed << std::setprecision(2) << seq_us << " us"
                  << " | par=" << std::setw(10) << std::fixed << std::setprecision(2) << par_us << " us"
                  << "\n";
    }

    const std::string csv_path = "data_race.csv";
    const std::string gp_path = "data_race.gp";
    const std::string png_path = "data_race.png";
    const std::string csv_path_threads = "data_race_threads.csv";
    const std::string gp_path_threads = "data_race_threads.gp";
    const std::string png_path_threads = "data_race_threads.png";


    if (save_png_with_gnuplot(samples, csv_path, gp_path, png_path)) {
        std::cout << "\nSaved plot: " << png_path << "\n";
        std::cout << "Saved data: " << csv_path << "\n";
    } else {
        std::cout << "\nPNG was not generated (gnuplot missing or error).\n";
    }
    
    if (gnu_plot(threaded_samples, thread_counts, logical_cores, csv_path_threads, gp_path_threads, png_path_threads)) {
        std::cout << "Saved plot: " << png_path_threads << "\n";
        std::cout << "Saved data: " << csv_path_threads << "\n";
    } else {
        std::cout << "PNG was not generated (gnuplot missing or error).\n";
    }

   
    return 0;
}
