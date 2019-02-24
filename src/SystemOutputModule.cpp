#include <soundstone/SystemOutputModule.hpp>

using namespace soundstone;

SystemOutputModule::SystemOutputModule(soundstone::SystemAudio *audio) {
    _audio = audio;
}

void SystemOutputModule::commit() {
}

void SystemOutputModule::sample(const float *const *input_buffers, float *output_buffer, uint32_t nsamples) {
    _audio->update(input_buffers[0], nsamples);
}
