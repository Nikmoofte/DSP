#ifndef SIGNALDATA_HPP
#define SIGNALDATA_HPP

#include <memory>

namespace DSP
{
    namespace Signals
    {
        class SignalBase;

        struct SignalData
        {
            SignalData(double A = 0.5, double freq = 440.0, double phase = 0.0, double d = 0.5);
            SignalData(const SignalData& other);
            SignalData(SignalData&& other);

            std::shared_ptr<std::shared_ptr<SignalBase>> amplitude;
            std::shared_ptr<std::shared_ptr<SignalBase>> freq;
            std::shared_ptr<std::shared_ptr<SignalBase>> time;
            std::shared_ptr<std::shared_ptr<SignalBase>> phase;
            std::shared_ptr<std::shared_ptr<SignalBase>> d;
        };
    }
}

#endif