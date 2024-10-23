#include "Nodes.hpp"

#include <imgui.h>
#include <implot.h>
#include <imnodes.h>

#include <utility>

#include "WAVController/WAVController.hpp"

namespace DSP
{
SignalNode::SignalNode() :
    NodeBase() 
{
    signal = std::make_shared<std::shared_ptr<DSP::Signals::SignalBase>>(std::make_shared<DSP::Signals::Sin>());
}




void SignalNode::Draw()
{
    updateSignalType();
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Signal id %d", id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(id + OutSignalAttrib);
    plotAGraph();
    ImNodes::EndOutputAttribute();

    ImNodes::BeginStaticAttribute(id + StaticTypeAttrib);
    static const char* signalTypes[] = {"Sin", "Cos", "Pulse", "Sawtooth", "Triangle", "Noise"};
    int tempSingalType = signalType;
    ImGui::SetNextItemWidth(100);
    ImGui::Combo("Signal type", &tempSingalType, signalTypes, IM_ARRAYSIZE(signalTypes));
    if(tempSingalType != signalType)
    {
        signalType = tempSingalType;
        isSignalTypeChanged = true;
    }
    ImNodes::EndStaticAttribute();

    ImNodes::BeginInputAttribute(id + InAmplitudeAttrib);
    ImGui::Text("Amplitude");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id + InFrequencyAttrib);
    ImGui::Text("Frequency");
    ImNodes::EndInputAttribute();
    ImNodes::BeginInputAttribute(id + InPhaseAttrib);
    ImGui::Text("Phase");
    ImNodes::EndInputAttribute();
    if(signalType == 2)
    {
        ImNodes::BeginInputAttribute(id + InDAttrib);
        ImGui::Text("Duty cycle");
        ImNodes::EndInputAttribute();
    }


    ImNodes::EndNode();
}

void SignalNode::updateSignalType()
{
    if(isSignalTypeChanged)
    {
        switch(signalType)
        {
            case 0:
                *signal = std::make_shared<DSP::Signals::Sin>((*signal)->getData());
                break;
            case 1:
                *signal = std::make_shared<DSP::Signals::Cos>((*signal)->getData());
                break;
            case 2:
                *signal = std::make_shared<DSP::Signals::Pulse>((*signal)->getData());
                break;
            case 3:
                *signal = std::make_shared<DSP::Signals::Sawtooth>((*signal)->getData());
                break;
            case 4:
                *signal = std::make_shared<DSP::Signals::Triangle>((*signal)->getData());
                break;
            case 5:
                *signal = std::make_shared<DSP::Signals::Noise>((*signal)->getData());
                break;
        }
        isSignalTypeChanged = false;
    }
}



FunctionNode::FunctionNode() :
    NodeBase()
{
    signal = std::make_shared<std::shared_ptr<DSP::Signals::SignalBase>>(std::make_shared<DSP::Signals::SumParam>(nullptr, nullptr));
}

void FunctionNode::Draw()
{
    updateFunctionType();
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Function id %d", id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginStaticAttribute(id + StaticTypeAttrib);
    ImGui::Text("Type");
    int tempFunctionType = functionType;
    static const char* types[] = {"Sum", "Mul", "Frequency Modulator"};
    ImGui::SetNextItemWidth(100);
    ImGui::Combo("##type", &tempFunctionType, types, IM_ARRAYSIZE(types));
    if(tempFunctionType != functionType)
    {
        functionType = tempFunctionType;
        isFunctionTypeChanged = true;
    }
    ImNodes::EndStaticAttribute();


    ImNodes::BeginInputAttribute(id + InLeftSignalAttrib);
    ImGui::Text("Left");
    ImNodes::EndInputAttribute();

    ImNodes::BeginInputAttribute(id + InRightSignalAttrib);
    ImGui::Text("Right");
    ImNodes::EndInputAttribute();

    ImNodes::BeginOutputAttribute(id + OutSignalAttrib);
    ImGui::Text("Value");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
}

void FunctionNode::updateFunctionType()
{
    if(isFunctionTypeChanged)
    {
        switch(functionType)
        {
            case 0:
                *signal = std::make_shared<DSP::Signals::SumParam>(
                    std::move(dynamic_cast<Signals::ComplexSignal&>(**signal).getLeft()), 
                    std::move(dynamic_cast<Signals::ComplexSignal&>(**signal).getRight())
                );
                break;
            case 1:
                *signal = std::make_shared<DSP::Signals::MulParam>(
                    std::move(dynamic_cast<Signals::ComplexSignal&>(**signal).getLeft()), 
                    std::move(dynamic_cast<Signals::ComplexSignal&>(**signal).getRight())
                );
                break;
            case 2:
                *signal = std::make_shared<DSP::Signals::freqModulator>(
                    std::move(dynamic_cast<Signals::ComplexSignal&>(**signal).getLeft()), 
                    std::move(dynamic_cast<Signals::ComplexSignal&>(**signal).getRight())
                );
                break;
        }
        isFunctionTypeChanged = false;
    }
}

ConstantNode::ConstantNode(double value) :
    NodeBase()
{
    signal = std::make_shared<std::shared_ptr<DSP::Signals::SignalBase>>(std::make_shared<DSP::Signals::Constant>(value));
}

void ConstantNode::Draw()
{
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Constant id %d", id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginOutputAttribute(id + OutValueAttrib);
    ImGui::Text("Value");
    ImGui::SetNextItemWidth(100);
    auto& objPtr = dynamic_cast<DSP::Signals::Constant&>(**signal);
    float tempValue = objPtr.value;
    ImGui::SliderFloat("##value", &tempValue, 0.0f, 1.0f);
    objPtr.set(tempValue);
    ImNodes::EndOutputAttribute();


    ImNodes::EndNode();
}


void OutputNode::Draw()
{
    ImNodes::BeginNode(id);

    ImNodes::BeginNodeTitleBar();
    ImGui::Text("Output id %d", id);
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(id + InSignalAttrib);
    ImGui::Text("Signal");
    ImNodes::EndInputAttribute();

    ImNodes::BeginStaticAttribute(id + StaticOutAttrib);
    if(signal && (*signal)->isValid())
    {
        plotAGraph();
    }
    if(ImGui::Button("Play"))
    {
        if(GenerateSound())
            WAVController::PlaylayWAV(soundPoints);
    }
    if(ImGui::Button("Save"))
    {
        if(GenerateSound())
            WAVController::CreateWAVFile("test.wav", soundPoints);
    }
    ImNodes::EndStaticAttribute();

    ImNodes::EndNode();
}

bool OutputNode::GenerateSound()
{
    if(signal)
    {
        for(int i = 0; i < soundPoints.size(); ++i)
        {
            soundPoints[i] = (*signal)->get(i);
        }
    }
    return true;
}

} // namespace DSP