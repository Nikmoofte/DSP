#ifndef SIGNALDATA_HPP
#define SIGNALDATA_HPP

#include <memory>

namespace DSP
{
    class SignalBase;

    struct SignalData
    {
        SignalData(double A = 0.5, double freq = 440.0, double phase = 0.0, double d = 0.5);
        SignalData(const SignalData& other);
        SignalData(SignalData&& other);

        std::unique_ptr<SignalBase> amplitude;
        std::unique_ptr<SignalBase> freq;
        std::unique_ptr<SignalBase> time;
        std::unique_ptr<SignalBase> phase;
        std::unique_ptr<SignalBase> d;
    };
}

#endif