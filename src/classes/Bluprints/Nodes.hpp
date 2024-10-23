#ifndef NODES_HPP
#define NODES_HPP

#include <memory>

#include "NodeBase.hpp"
#include "Signals/Signals.hpp"

#define NODE_CLASS_TYPE(type) static DSP::NodeType getStaticType() { return DSP::NodeType::type; }\
                                DSP::NodeType getType() const override { return getStaticType(); }\
                                const char* getName() const override { return #type; }

struct ImPlotPoint;

extern const uint32_t SAMPLE_RATE;
extern const uint32_t DURATION;


namespace DSP
{

class SignalNode : public NodeBase
{
public:
    NODE_CLASS_TYPE(Signal);
    enum 
    {
        OutSignalAttrib =   0x00010000,
        StaticTypeAttrib =  0x10000000,
        InAmplitudeAttrib = 0x00020000,
        InFrequencyAttrib = 0x00030000,
        InPhaseAttrib =     0x00040000,
        InDAttrib =         0x00050000,
    };  

    SignalNode();
    void Draw() override;

    void updateSignalType();

    ~SignalNode() override = default;
private:
    ImPlotPoint plotFunc(int idx, void* user_data);
    bool isSignalTypeChanged = false;
    int signalType = 0;
};

class FunctionNode : public NodeBase
{
public:
    NODE_CLASS_TYPE(Function);
    enum
    {
        OutSignalAttrib =   0x00010000,
        StaticTypeAttrib =  0x10000000,
        InLeftSignalAttrib = 0x00020000,
        InRightSignalAttrib = 0x00030000,
    };
    FunctionNode();
    void Draw() override;

    void updateFunctionType();

    ~FunctionNode() override = default;
private:
    bool isFunctionTypeChanged = false;
    int functionType = 0;
};

class ConstantNode : public NodeBase
{
public:
    NODE_CLASS_TYPE(Constant);
    enum
    {
        OutValueAttrib = 0x00010000
    };
    ConstantNode(double value = 0.0);
    void Draw() override;

    ~ConstantNode() override = default;
private:
};

class OutputNode : public NodeBase
{
public:
    NODE_CLASS_TYPE(Output);
    enum
    {
        InSignalAttrib = 0x00020000,
        StaticOutAttrib = 0x10000000
    };
    OutputNode() : NodeBase(), soundPoints(SAMPLE_RATE * DURATION) {};
    void Draw() override;
    bool GenerateSound();

    void setSignal(std::shared_ptr<std::shared_ptr<DSP::Signals::SignalBase>>& signal) override
    {
        NodeBase::setSignal(signal);
        isGenerated = false;
    }

    ~OutputNode() override = default;
private:
    bool isGenerated = false;
    std::vector<float> soundPoints;
};

}// namespace DSP

#endif