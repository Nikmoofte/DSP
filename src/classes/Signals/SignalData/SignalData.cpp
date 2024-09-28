#include "SignalData.hpp"

#include "Signals/Signals.hpp"

extern const int SAMPLE_RATE;

namespace DSP
{
    SignalData::SignalData(double A, double freq, double phase, double d) :
        amplitude(std::make_unique<Signals::Constant>(A)),
        freq(std::make_unique<Signals::Constant>(freq)),
        time(std::make_unique<Signals::Constant>(SAMPLE_RATE)),
        phase(std::make_unique<Signals::Constant>(phase)),
        d(std::make_unique<Signals::Constant>(d))
    {
    }

    SignalData::SignalData(const SignalData &other):
        amplitude(other.amplitude->clone()),
        freq(other.freq->clone()),
        time(other.time->clone()),
        phase(other.phase->clone()),
        d(other.d->clone())
    {
    }
    SignalData::SignalData(SignalData &&other) :
        amplitude(std::move(other.amplitude)),
        freq(std::move(other.freq)),
        time(std::move(other.time)),
        phase(std::move(other.phase)),
        d(std::move(other.d))
    {
    }
}