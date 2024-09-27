#ifndef PARAMETERBASE_HPP
#define PARAMETERBASE_HPP
    
#include <memory>

#define PARAM_CLASS_TYPE(type) static DSP::ParamType getStaticType() { return DSP::ParamType::type; }\
                                virtual DSP::ParamType getType() const override { return getStaticType(); }\
                                virtual const char* getName() const override { return #type; }


namespace DSP
{
    enum ParamType
    {
        NoType = 0,
        Constant,
        Signal,
        Function
    };

    template<class ParamT>
    class ParameterBase 
    {
    public:
        virtual ParamType getType() const = 0;
        virtual const char* getName() const = 0;

        virtual ParamT& getParam() = 0;
    private:
        std::unique_ptr<ParamT> paramVal;
    };
}

#endif