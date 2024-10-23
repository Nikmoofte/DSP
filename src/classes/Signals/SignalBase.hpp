#ifndef SIGNALBASE_HPP
#define SIGNALBASE_HPP

#include <utility>
#include <memory>

#include "Signals/SignalData/SignalData.hpp"

namespace DSP
{
    namespace Signals
    {

        struct SignalData;

        /**
         * @class SignalBase
         * @brief Base class for any signal
         */
        class SignalBase
        {
        public:
            SignalBase(
                double A = 0.5, 
                double freq = 440.0, 
                double phase = 0.0, 
                double d = 0.5
            ) : 
                data(std::make_unique<SignalData>(A, freq, phase, d))
            {};
            SignalBase(const SignalBase& other) : data(std::make_unique<SignalData>(*(other.data))){}
            SignalBase(const SignalData& data) : data(std::make_unique<SignalData>(data)){}
            SignalBase(SignalBase&& other) : data(std::move(other.data)){}
            SignalBase(SignalData&& data) : data(std::make_unique<SignalData>(std::move(data))){}
            virtual ~SignalBase(){}
            virtual bool isValid() const { return true; };
            virtual bool isComplex() const { return false; }
            virtual double get(double x) = 0;


            auto clone() const { return std::unique_ptr<SignalBase>(cloneImpl()); }
            
            virtual SignalData& getData() { return *data; };
        protected:
            //crutch to create signal without SignalData
            SignalBase(int* Null){}
            virtual SignalBase* cloneImpl() const = 0;
            
            std::unique_ptr<SignalData> data;
        };

    }
}

#endif