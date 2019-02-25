#include "SinewaveGenerator.hpp"
#include "SquareGenerator.hpp"
#include "Mixer.hpp"

#include <soundstone/SystemAudio.hpp>
#include <soundstone/AudioProcessor.hpp>
#include <soundstone/SystemOutputModule.hpp>
#include <soundstone/ThreadedDriver.hpp>

#include <thread>
#include <iostream>
#include <chrono>
#include <cmath>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"


using namespace soundstone;
using namespace soundstone_example;
using namespace std;
using namespace std::chrono;


int main(int argc, char **argv) {
    AudioProcessor processor;
    SystemAudio system;

    SystemOutputModule output(&system);
    SinewaveGenerator sin(540, system.sample_rate());
    SquareGenerator square(540, system.sample_rate());
    square.set_amplitude(0.25f);
    Mixer mixer;

    processor.add_module(&output);
    processor.add_module(&sin);
    processor.add_module(&square);
    processor.add_module(&mixer);

    processor.route(&sin).to(&mixer, 0);
    processor.route(&square).to(&mixer, 1);
    processor.route(&mixer).to(&output);

    processor.set_thread_count(1);

    uint32_t underflows = 0;

    system.set_drained_callback([&]{ ++underflows; });

    const uint32_t driver_latency = 512;

    cout << "=== Soundstone Example ===" << endl;
    cout << "Sample Rate: " << system.sample_rate() << endl;
    cout << "System Audio Latency: " << system.latency() << endl;
    cout << "Driver Latency: " << driver_latency << endl;

    ThreadedDriver driver(&processor, &system);
    driver.set_latency_samples(driver_latency);
    driver.start();

    while (true) {
        cout << "\r" << "Samples Buffered: " << system.samples_buffered() << ", Underflows: " << underflows;
        cout.flush();
        this_thread::sleep_for(milliseconds(250));
    }
}


#pragma clang diagnostic pop
