#include <soundstone/SoundSystem.hpp>

using namespace soundstone;
using namespace std;

SoundSystem::SoundSystem() {
    cubeb_init(&_cubeb, nullptr, nullptr);
    _stream = nullptr;
    if (!init_cubeb()) {
        destroy_cubeb();
    }
}

SoundSystem::SoundSystem(SoundSystem &&) noexcept = default;
SoundSystem::~SoundSystem() = default;
SoundSystem &SoundSystem::operator=(SoundSystem &&) noexcept = default;

bool SoundSystem::init_cubeb() {
    int rv;

    rv = cubeb_get_preferred_sample_rate(_cubeb, &_sample_rate);
    if (rv != CUBEB_OK) {
        return false;
    }

    cubeb_stream_params output_params;
    output_params.format = CUBEB_SAMPLE_FLOAT32NE;
    output_params.channels = 2;
    output_params.rate = _sample_rate;
    output_params.layout = CUBEB_LAYOUT_UNDEFINED;

    rv = cubeb_get_min_latency(_cubeb, &output_params, &_latency);
    if (rv != CUBEB_OK) {
        return false;
    }

    rv = cubeb_stream_init(_cubeb, &_stream, "Soundstone Application",
        nullptr, nullptr,
        nullptr, &output_params,
        _latency, data_callback, state_callback, this
    );
    if (rv != CUBEB_OK) {
        return false;
    }

    rv = cubeb_stream_start(_stream);
    if (rv != CUBEB_OK) {
        return false;
    }

    return true;
}

void SoundSystem::destroy_cubeb() {
    if (_stream != nullptr) {
        cubeb_stream_destroy(_stream);
        _stream = nullptr;
    }
    if (_cubeb != nullptr) {
        cubeb_destroy(_cubeb);
        _cubeb = nullptr;
    }
}

void SoundSystem::add_sampler(Sampler *sampler) {
}

void SoundSystem::remove_sampler(Sampler *sampler) {

}

void SoundSystem::update() {

    size_t sample_count = 0;



    for (Sampler *sampler : _samplers) {


    }



}


void SoundSystem::mix(float *a, const float *b, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        float sample_a = a[i];
        float sample_b = b[i];
        a[i] = sample_a + sample_b;
    }
}

long SoundSystem::data_callback(
    cubeb_stream *stream, void *user_ptr, const void *input_buffer,
    void *output_buffer, long nframes
) {
    return 0;
}

void SoundSystem::state_callback(
    cubeb_stream *stream, void *user_ptr, cubeb_state state
) {

}
