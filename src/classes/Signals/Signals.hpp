#ifndef SIGNALS_HPP
#define SIGNALS_HPP


#include <cmath>
#include <random>
#include <memory>
#include <functional>

#include "SignalBase.hpp"

constexpr static double pi2 = 2 * M_PI;
extern const uint32_t SAMPLE_RATE;

#define ConstructorsInit(className) (className)(double A = 0.5, double freq = 440.0, double phase = 0.0, double d = 0.5) : SignalBase(A, freq, phase, d){}\
                                    (className)(const SignalBase& other) : SignalBase(other){}\
                                    (className)(const SignalData& data) : SignalBase(data){}\
                                    (className)(SignalBase&& other) : SignalBase(std::move(other)){}\
                                    (className)(SignalData&& data) : SignalBase(std::move(data)){}

#define CloneImplimentation(className) virtual className* cloneImpl() const override { return new className(*this); }

namespace DSP
{
    class ConstantNode;
namespace Signals
{

    class Sin : public SignalBase
    {
    public:
        ConstructorsInit(Sin);
        double get(double x) override
        {
            double res = (*data->amplitude)->get(x) * ::sin(pi2 * (*data->freq)->get(x) * x / (*data->time)->get(x) + (*data->phase)->get(x)); 
            return res;
        }
    private:
        CloneImplimentation(Sin);
    };

    class Cos : public SignalBase
    {
    public:
        ConstructorsInit(Cos);
        double get(double x) override
        {
            double res = (*data->amplitude)->get(x) * ::cos(pi2 * (*data->freq)->get(x) * x / (*data->time)->get(x) + (*data->phase)->get(x)); 
            return res;
        }
    private:
        CloneImplimentation(Cos);
    };

    class Triangle : public SignalBase
    {
    public:
        ConstructorsInit(Triangle);
        double get(double x) override
        {
            double res = (*data->amplitude)->get(x) * M_2_PI *(std::abs(fmod(pi2 * (*data->freq)->get(x) * x / (*data->time)->get(x) + (*data->phase)->get(x) + 3 * M_PI_2, pi2) - M_PI) - M_PI_2);
            return res;
        }
    private:
        CloneImplimentation(Triangle);
    };

    class Sawtooth : public SignalBase
    {
    public:
        ConstructorsInit(Sawtooth)
        double get(double x) override
        {
            double res = (*data->amplitude)->get(x) * M_1_PI * (fmod(pi2 * (*data->freq)->get(x) * x / (*data->time)->get(x) + (*data->phase)->get(x) + M_PI, pi2) - M_PI);
            return res;
        }
    private:
        CloneImplimentation(Sawtooth);
    };

    class Pulse : public SignalBase
    {
    public:
        ConstructorsInit(Pulse)
         double get(double x) override
        {
            double res = std::fmod(pi2 * (*data->freq)->get(x) * x / (*data->time)->get(x) + (*data->phase)->get(x), pi2) / pi2;
            return res <= (*data->d)->get(x) ? (*data->amplitude)->get(x) : -(*data->amplitude)->get(x);
        }
    private:
        CloneImplimentation(Pulse);
    };

    class Noise : public SignalBase
    {
    public:
        ConstructorsInit(Noise)
         double get(double x) override
        {
            std::mt19937 gen(x + (*data->phase)->get(x)); 
            static std::uniform_real_distribution<> dis(-1.0f, 1.0f);
            double res = (*data->amplitude)->get(x) * dis(gen);
            return res;
        }
    private:
        CloneImplimentation(Noise);
    };

    class Constant : public SignalBase
    {
        friend class DSP::ConstantNode;
    public:
        Constant() : SignalBase(nullptr), value(0.0){}
        Constant(const Constant& other) : SignalBase(nullptr),  value(other.value){}
        Constant(Constant&& other) : SignalBase(nullptr), value(other.value){}
        Constant(double value) : SignalBase(nullptr), value(value){}
        
        double get(double x) override
        {
            return value;
        }
        void set(double newValue)
        {
            value = newValue;
        }
        ~Constant() override {}
    private:
        CloneImplimentation(Constant);
        double value;
    };

    class ComplexSignal : public SignalBase
    {
    public:
        ComplexSignal(
            std::shared_ptr<std::shared_ptr<SignalBase>>&& left, 
            std::shared_ptr<std::shared_ptr<SignalBase>>&& right,
            const std::function<double(double, double)>& func 
        ) : 
            SignalBase(nullptr), 
            left(std::move(left)),
            right(std::move(right)),
            _func(func)
        {}
        ComplexSignal(
            std::shared_ptr<std::shared_ptr<SignalBase>>& left, 
            std::shared_ptr<std::shared_ptr<SignalBase>>& right,
            const std::function<double(double, double)>& func 
        ) : 
            SignalBase(nullptr), 
            left(left),
            right(right),
            _func(func)
        {}
        ComplexSignal(const ComplexSignal& other) :
            SignalBase(nullptr), 
            left(other.left),  
            right(other.right),
            _func(other._func)    
        {}
        bool isValid() const override
        {
            return left && (*left) && (*left)->isValid() && right && (*right) && (*right)->isValid();
        }
        bool isComplex() const override
        {
            return true;
        }
        virtual double get(double x) override
        {
            return _func((*left)->get(x), (*right)->get(x));
        }
        virtual SignalData& getData() override
        {
            return (*left)->getData();
        }
        std::shared_ptr<std::shared_ptr<SignalBase>>& getLeft()
        {
            return left;
        }
        std::shared_ptr<std::shared_ptr<SignalBase>>& getRight()
        {
            return right;
        }
        void SetLeft(std::shared_ptr<std::shared_ptr<SignalBase>>& newLeft)
        {
            left = newLeft;
        }
        void SetRight(std::shared_ptr<std::shared_ptr<SignalBase>>& newRight)
        {
            right = newRight;
        }
        virtual ~ComplexSignal() override {}
    protected:
        CloneImplimentation(ComplexSignal);
        std::shared_ptr<std::shared_ptr<SignalBase>> left;
        std::shared_ptr<std::shared_ptr<SignalBase>> right;
        std::function<double(double, double)> _func;
    };

    class freqModulator : public ComplexSignal
    {
    public:
        freqModulator(
            std::shared_ptr<std::shared_ptr<SignalBase>>&& modulated,  
            std::shared_ptr<std::shared_ptr<SignalBase>>&& modulator    
        ) : ComplexSignal(
            std::move(modulated), 
            std::move(modulator), 
            [](double, double) { return 0;})
        {}
        freqModulator(
            std::shared_ptr<std::shared_ptr<SignalBase>>& modulated,  
            std::shared_ptr<std::shared_ptr<SignalBase>>& modulator    
        ) : ComplexSignal(
            modulated, 
            modulator, 
            [](double, double) { return 0;})
        {}

        double get(double x) override
        {
            double result = 0.0;
            double sum = 0.0;

            sum = accumulatedIntegrate(x);
            auto tempPrevFreq = (*left)->getData().freq;
            auto temp = std::make_shared<Signals::Constant>(sum);
            auto tempPtr = std::make_shared<std::shared_ptr<Signals::SignalBase>>(temp);
            (*left)->getData().freq = tempPtr;
            result = (*left)->get(SAMPLE_RATE);
            (*left)->getData().freq = tempPrevFreq;
            
            return result;
        }
    private:
        CloneImplimentation(freqModulator);
        double accumulatedIntegrate(double x)
        {
            static double prev = 0.0;
            
            if(x < 1)
            {
                prev = 0.0;
                return prev;
            }

            prev += (*((*left)->getData().freq))->get(x) * (1 + (*right)->get(x)) / SAMPLE_RATE; 
            return prev;
        }
        double integrate(double a, double b, int step)
        {
            double h = (b - a) / step; 
            double sum = sin(2 * M_PI * a / SAMPLE_RATE) + sin(2 * M_PI * b / SAMPLE_RATE);  
            
            for (int i = 1; i < step; i++) {
                double x_i = a + i * h;
                sum += 2 * sin(2 * M_PI * x_i / SAMPLE_RATE);
            }

            return (h / 2) * sum; 
        }
    };

    class SumParam : public ComplexSignal
    {
    public:
        SumParam(
            std::shared_ptr<std::shared_ptr<SignalBase>>&& left, 
            std::shared_ptr<std::shared_ptr<SignalBase>>&& right    
        ) : 
            ComplexSignal(
                std::move(left), 
                std::move(right), 
                [](double l, double r){ return l + r; }
            )
        {}
        SumParam(
            std::shared_ptr<std::shared_ptr<SignalBase>>& left, 
            std::shared_ptr<std::shared_ptr<SignalBase>>& right    
        ) : 
            ComplexSignal(
                left, 
                right, 
                [](double l, double r){ return l + r; }
            )
        {}
        
        ~SumParam() override {}
    private:
        CloneImplimentation(SumParam);
    };

    class MulParam : public ComplexSignal
    {
    public:
        MulParam(
            std::shared_ptr<std::shared_ptr<SignalBase>>&& left, 
            std::shared_ptr<std::shared_ptr<SignalBase>>&& right    
        ) : 
            ComplexSignal(
                std::move(left), 
                std::move(right), 
                [](double l, double r){ return l * r; }
            )
        {}
        MulParam(
            std::shared_ptr<std::shared_ptr<SignalBase>>& left, 
            std::shared_ptr<std::shared_ptr<SignalBase>>& right    
        ) : 
            ComplexSignal(
                left, 
                right, 
                [](double l, double r){ return l * r; }
            )
        {}

        ~MulParam() override {}
    private:
        CloneImplimentation(MulParam);
    };


}// namespace Signals
}// namespace DSP

#endif