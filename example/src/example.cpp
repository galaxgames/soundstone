#include <soundstone/SoundSystem.hpp>
#include <soundstone/RealTimeSoundSystemManager.hpp>
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
    SinewaveSampler sampler1(540);
    SinewaveSampler sampler2(440);
    SinewaveSampler sampler3(340);
    SinewaveSampler sampler4(240);
    SquareSampler square1(540);
    SquareSampler square2(440);
    SquareSampler square3(340);
    SquareSampler square4(240);
    system.add_sampler(&sampler1);
    system.add_sampler(&sampler2);
    system.add_sampler(&sampler3);
    system.add_sampler(&sampler4);
    system.add_sampler(&square1);
    system.add_sampler(&square2);
    system.add_sampler(&square3);
    system.add_sampler(&square4);

    system.route_sampler_to_root(&sampler1);
    system.route_sampler_to_root(&sampler2);
    system.route_sampler_to_root(&sampler3);
    system.route_sampler_to_root(&sampler4);
    system.route_sampler_to_root(&square1);
    system.route_sampler_to_root(&square2);
    system.route_sampler_to_root(&square3);
    system.route_sampler_to_root(&square4);

    system.set_thread_count(4);
    RealTimeSoundSystemManager manager(&system);

    system.update(1024);

    chrono::high_resolution_clock::time_point frame_start = chrono::high_resolution_clock::now();

    for (;;) {
        auto since_epoch = chrono::duration_cast<dseconds>(frame_start.time_since_epoch());
        square1.set_amplitude(sin(since_epoch.count()));

        // Simulate processing logic on main frame
        this_thread::sleep_for(chrono::duration<int, milli>(10));

        auto now = chrono::high_resolution_clock::now();
        auto frame_duration = now - frame_start;
        frame_start = now;
        auto frame_duration_seconds = chrono::duration_cast<dseconds>(frame_duration);
        manager.update(frame_duration_seconds.count());
    }

}
