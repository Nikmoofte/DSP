#ifndef NODEBASE_HPP
#define NODEBASE_HPP

#include <cstdint>
#include <memory>

#include "Signals/SignalBase.hpp"

namespace DSP
{
enum NodeType
{
    Signal,
    Function,
    Constant,
    Output
};

class NodeBase
{
public:
    enum
    {
        NodeIdMask = 0x0000FFFF,
        AttribIdMask = 0xFFFF0000
    };
    NodeBase();
    NodeBase(std::shared_ptr<std::shared_ptr<DSP::Signals::SignalBase>>& signalPtr) : NodeBase() { this->signal = signalPtr; };
    NodeBase(const NodeBase& other) = default;

    virtual DSP::NodeType getType() const = 0;
    virtual const char* getName() const = 0;

    virtual std::shared_ptr<std::shared_ptr<DSP::Signals::SignalBase>>& getSignal() { return signal; }
    virtual void setSignal(std::shared_ptr<std::shared_ptr<DSP::Signals::SignalBase>>& signal) { this->signal = signal; }

    void plotAGraph();



    virtual void Draw();
    uint32_t getId() const { return id; }
    virtual ~NodeBase() = default;
protected:
    static constexpr const double animationSpeed = 0.01;
    bool animate = true;
    double animationPhase = 0.0;
    uint32_t id = 0;
    float x = 0.0f, y = 0.0f;
    std::shared_ptr<std::shared_ptr<DSP::Signals::SignalBase>> signal;
};
}// namespace DSP

#endif