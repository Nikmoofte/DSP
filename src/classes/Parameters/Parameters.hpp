#ifndef PARAMETERBASE_HPP
#define PARAMETERBASE_HPP
    
#include <memory>

#include "Signals/Signals.hpp"

#define PARAM_CLASS_TYPE(type) static DSP::ParamType getStaticType() { return DSP::ParamType::type; }\
                                DSP::ParamType getType() const { return getStaticType(); }\
                                const char* getName() const { return #type; }


namespace DSP
{
    // enum ParamType
    // {
    //     NoType = 0,
    //     Constant,
    //     Signal,
    //     Function
    // };

    template<class ParamT>
    class ParameterBase
    {
    public:
        ParameterBase(std::unique_ptr<ParamT>&& param) : param(std::move(param)) {};
        ParameterBase(const ParameterBase& other) : param(std::make_unique<ParamT>(other.param->clone())) {}
        ParameterBase(ParameterBase&& other) : param(std::move(other.param)) {}

        inline double get(double x) { return param->get(x); }
        inline Signals::SignalData& getData() { return param->getData(); }

        inline const ParamT& getParam() const { return *param; }
        inline ParamT& getParam() { return const_cast<ParamT&>(const_cast<const ParameterBase&>(*this).getParam()); }
        operator ParamT() { return getParam(); }
        
    protected:
        std::unique_ptr<ParamT> param;
    };

    template<class paramT>
    class Parameter : ParameterBase<paramT>
    {
        Parameter() = 0;
        PARAM_CLASS_TYPE(NoType);
    };

    template<>
    class Parameter<Signals::Constant> : public ParameterBase<Signals::Constant>
    {
    public:
        Parameter(double value = 0.0) : 
            ParameterBase<Signals::Constant>(std::make_unique<Signals::Constant>(value))
        {}
        PARAM_CLASS_TYPE(Constant);
    };

    template<>
    class Parameter<Signals::ComplexSignal> : public ParameterBase<Signals::ComplexSignal>
    {
    public:
        template<class LeftT, class RightT>
        Parameter(
            const Parameter<LeftT>& left,
            const Parameter<RightT>& right,
            const std::function<double(double, double)>& func
        ) : 
            ParameterBase<Signals::ComplexSignal>(
                std::make_unique<Signals::ComplexSignal>(
                    left.getParam->clone(), 
                    right.getParam->clone(), 
                    func
                )
            ) 
        {}

        template<class LeftT, class RightT>
        Parameter(
            Parameter<LeftT>&& left,
            Parameter<RightT>&& right,
            const std::function<double(double, double)>& func
        ) : 
            ParameterBase<Signals::ComplexSignal>(
                std::make_unique<Signals::ComplexSignal>(
                    std::move(left.param), 
                    std::move(right.param), 
                    func
                )
            ) 
        {}
        Parameter(
            std::unique_ptr<DSP::Signals::SignalBase>&& left,
            std::unique_ptr<DSP::Signals::SignalBase>&& right,
            const std::function<double(double, double)>& func
        ) : 
            ParameterBase<Signals::ComplexSignal>(
                std::make_unique<Signals::ComplexSignal>(
                    std::move(left), 
                    std::move(right), 
                    func
                )
            ) 
        {}
        
        Parameter(std::unique_ptr<Signals::ComplexSignal>&& complSignal) : 
            ParameterBase<Signals::ComplexSignal>(std::move(complSignal)) 
        {}
        
        PARAM_CLASS_TYPE(Function);
    };

    template<>
    class Parameter<Signals::freqModulator> : public ParameterBase<Signals::ComplexSignal>
    {
    public:
        Parameter(std::unique_ptr<Signals::freqModulator>&& Signal) : 
            ParameterBase<Signals::ComplexSignal>(std::move(Signal)) 
        {}

        template<class LeftT, class RightT>
        Parameter(
            const Parameter<LeftT>& modulated,
            const Parameter<RightT>& modulator
        ) : 
            Parameter(
                std::make_unique<Signals::freqModulator>(
                    modulated.getParam->clone(), 
                    modulator.getParam->clone()
                )
            ) 
        {}

        template<class LeftT, class RightT>
        Parameter(
            Parameter<LeftT>&& modulated,
            Parameter<RightT>&& modulator
        ) : 
            Parameter(
                std::make_unique<Signals::freqModulator>(
                    std::move(modulated.param), 
                    std::move(modulator.param) 
                )
            ) 
        {}
        Parameter(
            std::unique_ptr<DSP::Signals::SignalBase>&& modulated,
            std::unique_ptr<DSP::Signals::SignalBase>&& modulator
        ) : 
            Parameter(
                std::make_unique<Signals::freqModulator>(
                    std::move(modulated), 
                    std::move(modulator)
                )
            ) 
        {}
        
        
        PARAM_CLASS_TYPE(Function);
    };
    
    template<>
    class Parameter<Signals::SumParam> : public ParameterBase<Signals::ComplexSignal>
    {
    public:
        Parameter(std::unique_ptr<Signals::SumParam>&& Signal) : 
            ParameterBase<Signals::ComplexSignal>(std::move(Signal)) 
        {}

        template<class LeftT, class RightT>
        Parameter(
            const Parameter<LeftT>& left,
            const Parameter<RightT>& right
        ) : 
            Parameter(
                std::make_unique<Signals::SumParam>(
                    left.getParam->clone(), 
                    right.getParam->clone()
                )
            ) 
        {}

        template<class LeftT, class RightT>
        Parameter(
            Parameter<LeftT>&& left,
            Parameter<RightT>&& right
        ) : 
            Parameter(
                std::make_unique<Signals::SumParam>(
                    std::move(left.param), 
                    std::move(right.param) 
                )
            ) 
        {}
        Parameter(
            std::unique_ptr<DSP::Signals::SignalBase>&& left,
            std::unique_ptr<DSP::Signals::SignalBase>&& right
        ) : 
            Parameter(
                std::make_unique<Signals::SumParam>(
                    std::move(left), 
                    std::move(right)
                )
            ) 
        {}
        
        
        PARAM_CLASS_TYPE(Function);
    };

    template<>
    class Parameter<Signals::SignalBase> : public ParameterBase<Signals::SignalBase>
    {
    public:
        Parameter(std::unique_ptr<Signals::SignalBase>&& signal) : 
            ParameterBase<Signals::SignalBase>(std::move(signal)) 
        {}
        PARAM_CLASS_TYPE(Signal);
    };

    template<>
    class Parameter<Signals::Sin> : public ParameterBase<Signals::SignalBase>
    {
    public:
        Parameter(double A = 0.5, double freq = 440, double phase = 0.0) : 
            ParameterBase<Signals::SignalBase>(std::make_unique<DSP::Signals::Sin>(A, freq, phase)) 
        {}
        PARAM_CLASS_TYPE(Signal);
    };
    template<>
    class Parameter<Signals::Cos> : public ParameterBase<Signals::SignalBase>
    {
    public:
        Parameter(double A = 0.5, double freq = 440, double phase = 0.0) : 
            ParameterBase<Signals::SignalBase>(std::make_unique<DSP::Signals::Cos>(A, freq, phase)) 
        {}
        PARAM_CLASS_TYPE(Signal);
    };
    template<>
    class Parameter<Signals::Pulse> : public ParameterBase<Signals::SignalBase>
    {
    public:
        Parameter(double A = 0.5, double freq = 440, double phase = 0.0, double d = 0.5) : 
            ParameterBase<Signals::SignalBase>(std::make_unique<DSP::Signals::Pulse>(A, freq, phase, d)) 
        {}
        PARAM_CLASS_TYPE(Signal);
    };

    template<>
    class Parameter<Signals::Sawtooth> : public ParameterBase<Signals::SignalBase>
    {
    public:
        Parameter(double A = 0.5, double freq = 440, double phase = 0.0) : 
            ParameterBase<Signals::SignalBase>(std::make_unique<DSP::Signals::Sawtooth>(A, freq, phase)) 
        {}
        PARAM_CLASS_TYPE(Signal);
    };

    template<>
    class Parameter<Signals::Triangle> : public ParameterBase<Signals::SignalBase>
    {
    public:
        Parameter(double A = 0.5, double freq = 440, double phase = 0.0) : 
            ParameterBase<Signals::SignalBase>(std::make_unique<DSP::Signals::Triangle>(A, freq, phase)) 
        {}
        PARAM_CLASS_TYPE(Signal);
    };
}

#endif