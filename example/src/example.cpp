#include <soundstone/SoundSystem.hpp>
#include "SinewaveSampler.hpp"
#include "SquareSampler.hpp"
#include <thread>
#include <iostream>
#include <chrono>
#include <cmath>

using namespace soundstone;
using namespace soundstone_example;
using namespace std;

typedef chrono::duration<double> dseconds;

int main(int argc, char **argv) {
    SoundSystem system;
    SinewaveSampler sampler(440);
    SquareSampler square(440);
    system.add_sampler(&sampler);
    system.add_sampler(&square);
    system.set_thread_count(4);

    chrono::high_resolution_clock::time_point frame_start = chrono::high_resolution_clock::now();

    system.update(1024);
    for (;;) {
        // Begining of frame
        //frame_start = chrono::high_resolution_clock::now();

        auto now = chrono::high_resolution_clock::now();
        square.set_amplitude(sin(chrono::duration_cast<dseconds>(now.time_since_epoch()).count()));


        // Simulate processing logic on main frame
        this_thread::sleep_for(chrono::duration<int, milli>(10));

        now = chrono::high_resolution_clock::now();
        auto frame_duration = now - frame_start;
        frame_start = now;
        auto frame_duration_seconds = chrono::duration_cast<dseconds>(frame_duration);

        double samples = static_cast<double>(system.sample_rate()) * frame_duration_seconds.count();
        size_t nsamples = static_cast<size_t>(samples);

        bool sample_count_changed = false;

        size_t samples_buffered = system.samples_buffered();
        size_t min_samples_target = size_t(44100) / size_t(60);
        size_t max_samples_target = (size_t(44100) / size_t(60)) * 8;
        if (nsamples + samples_buffered < min_samples_target) {
            cout << "Increasing sample count from " << nsamples;
            nsamples = min_samples_target - samples_buffered;
            cout << " to " << nsamples;
            sample_count_changed = true;

        }
        else if (nsamples + samples_buffered > max_samples_target) {
            cout << "Descreasing sample count from " << nsamples;
            nsamples = max_samples_target - samples_buffered;
            cout << " to " << nsamples;
            sample_count_changed = true;
        }

        system.update(nsamples);

        if (sample_count_changed) {
            cout << " update sample count: " << nsamples << ", buffer size: " << system.samples_buffered() << endl;
        }

    }

}
