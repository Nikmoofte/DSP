#include "SignalData.hpp"

#include "Signals/Signals.hpp"

extern const uint32_t SAMPLE_RATE;

namespace DSP
{
    namespace Signals
    {
        SignalData::SignalData(double A, double freq, double phase, double d) :
            amplitude(std::make_shared<std::shared_ptr<Signals::SignalBase>>(std::make_shared<Signals::Constant>(A))),
            freq(std::make_shared<std::shared_ptr<Signals::SignalBase>>(std::make_shared<Signals::Constant>(freq))),
            time(std::make_shared<std::shared_ptr<Signals::SignalBase>>(std::make_shared<Signals::Constant>(SAMPLE_RATE))),
            phase(std::make_shared<std::shared_ptr<Signals::SignalBase>>(std::make_shared<Signals::Constant>(phase))),
            d(std::make_shared<std::shared_ptr<Signals::SignalBase>>(std::make_shared<Signals::Constant>(d)))
        {
        }

        SignalData::SignalData(const SignalData &other):
            amplitude(other.amplitude),
            freq(other.freq),
            time(other.time),
            phase(other.phase),
            d(other.d)
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
}