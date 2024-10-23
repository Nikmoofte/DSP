#include "NodeBase.hpp"

#include <vector>
#include <numeric>

#include "imnodes.h"
#include "implot.h"
#include "imgui.h"

#include "Signals/Signals.hpp"

namespace DSP
{
NodeBase::NodeBase()
{
    static uint32_t NextId = 1;
    id = NextId++;
}

void NodeBase::plotAGraph()
{
    auto& signal = **this->signal;
    if(animate)
        animationPhase += animationSpeed;
    
    if(ImPlot::BeginPlot(("Plot##" + std::to_string(id)).c_str(), ImVec2(400, 200)))
    {
        static std::vector<double> xes(1000), yes(1000);
        std::iota(xes.begin(), xes.end(), 0);
        std::shared_ptr<std::shared_ptr<Signals::SignalBase>> tempPahseLeft = signal.getData().phase;
        std::shared_ptr<std::shared_ptr<Signals::SignalBase>> tempPahseRight;
        if(animate)
        {
            auto temp = std::make_shared<std::shared_ptr<Signals::SignalBase>>(std::make_shared<Signals::Constant>(animationPhase));
            auto newPtr = std::make_shared<Signals::SumParam>(
                    tempPahseLeft, 
                    temp
            );
            signal.getData().phase = std::make_shared<std::shared_ptr<Signals::SignalBase>>(newPtr);
            if(signal.isComplex())
            {
                auto& right = **(dynamic_cast<Signals::ComplexSignal&>(signal).getRight());
                tempPahseRight = right.getData().phase;
                auto newPtr = std::make_shared<Signals::SumParam>(
                        tempPahseRight, 
                        temp
                );
                right.getData().phase = std::make_shared<std::shared_ptr<Signals::SignalBase>>(
                    newPtr
                );
            }
        }
        for(int i = 0; i < xes.size(); ++i)
        {
            yes[i] = signal.get(xes[i]);
        }
        if(animate)
        {
            signal.getData().phase = tempPahseLeft;
            if(signal.isComplex())
            {
                auto& right = **(dynamic_cast<Signals::ComplexSignal&>(signal).getRight());
                right.getData().phase = tempPahseRight;
            }
        }
        ImPlot::PlotLine(
            "##d",
            xes.data(), 
            yes.data(), 
            xes.size()
        );
        ImPlot::EndPlot();
    }
    ImGui::Checkbox(("Animate##" + std::to_string(id)).c_str(), &animate);
}

void NodeBase::Draw()
{
    ImNodes::BeginNode(id);
    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Node Base. Id = %d", id);
    ImNodes::EndNodeTitleBar();
    ImGui::Dummy(ImVec2(80.0f, 45.0f));
    ImNodes::EndNode();
}   
}// namespace DSP