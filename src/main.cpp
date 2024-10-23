#include <print>
#include <cmath>
#include <list>
#include <algorithm>
#include <random>
#include <numbers>
#include <memory>

#include "imgui.h"
#include "imnodes.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> 

#include "Signals/Signals.hpp"
#include "WAVController/WAVController.hpp"
#include "Bluprints/Nodes.hpp"

static void glfw_error_callback(int error, const char* description)
{
    std::print(stderr, "GLFW Error %d: %s\n", error, description);
}

extern const uint32_t SAMPLE_RATE = 44100;
extern const uint32_t DURATION = 4;
static constexpr uint32_t total_size = SAMPLE_RATE * DURATION;

static int id_counter = 0;
struct Link
{
    int id;
    int start_attr, end_attr;
};

struct Editor
{
    std::list<std::unique_ptr<DSP::NodeBase>> nodes;
    std::list<Link> links;
};

// Main code
int main(int, char**)
{
    
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Lab", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    
    ImGui::StyleColorsLight();
    ImNodes::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    std::vector<float> soundPoints(total_size);
    static constexpr int plotDataSize = total_size / 512;
    std::vector<float> plotPoints(plotDataSize);
    std::vector<float> modulatoplotPoints(plotDataSize);
    std::vector<float> xes(plotDataSize);


    double x = 0.0;
    double offset = 0.0;
    bool isAnimated = true;
    Editor editor;
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(1, ImGui::GetMainViewport());

        // ImGui::SetNextWindowDockID(1u);
        // ImGui::ShowDemoWindow();

        ImGui::SetNextWindowDockID(1u);
        if(ImGui::Begin("Editor", static_cast<bool*>(0), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
        {
            ImNodes::BeginNodeEditor();

            const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && 
                                    ImNodes::IsEditorHovered() && 
                                    ImGui::IsKeyReleased(ImGuiKey_A);

            if (!ImGui::IsAnyItemHovered() && open_popup)
            {
                ImGui::OpenPopup("my_select_popup");
            }

            int selected_node_type = -1;
            if (ImGui::BeginPopup("my_select_popup"))
            {
                const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

                static const char* names[] = {"Signals", "Functions", "Constants", "Outputs"};
                ImGui::SeparatorText("Aquarium");
                for (int i = 0; i < IM_ARRAYSIZE(names); i++)
                    if (ImGui::Selectable(names[i]))
                        selected_node_type = i;
                ImGui::EndPopup();
                if(selected_node_type != -1)
                {
                    const int node_id = ++id_counter;
                    switch(selected_node_type)
                    {
                        case 0:
                            editor.nodes.push_back(std::make_unique<DSP::SignalNode>());
                            break;
                        case 1:
                            editor.nodes.push_back(std::make_unique<DSP::FunctionNode>());
                            break;
                        case 2:
                            editor.nodes.push_back(std::make_unique<DSP::ConstantNode>());
                            break;
                        case 3:
                            editor.nodes.push_back(std::make_unique<DSP::OutputNode>());
                            break;
                    }
                    selected_node_type = -1;
                    ImNodes::SetNodeScreenSpacePos(editor.nodes.back()->getId(), click_pos);
                }
            }


            for(auto& node : editor.nodes)
            {
                node->Draw();
            }
            for(auto& link : editor.links)
            {
                ImNodes::Link(link.id, link.start_attr, link.end_attr);
            }


            ImNodes::EndNodeEditor();
            //Link creation
            {
                Link link;
                if (ImNodes::IsLinkCreated(&link.start_attr, &link.end_attr))
                {
                    link.id = ++id_counter;
                    if(((link.start_attr & DSP::NodeBase::AttribIdMask) < 0x0002'0000) && // check if it is an output attribute
                        (link.end_attr & DSP::NodeBase::AttribIdMask) >= 0x0002'0000) // check if it is an input attribute
                    {
                        uint32_t startId = link.start_attr & DSP::NodeBase::NodeIdMask;
                        uint32_t endId = link.end_attr & DSP::NodeBase::NodeIdMask;

                        auto& OutputSignal = (*std::find_if(editor.nodes.begin(), editor.nodes.end(), [startId](std::unique_ptr<DSP::NodeBase>& node) -> bool {
                            return node->getId() == startId;
                        }))->getSignal();
                        std::unique_ptr<DSP::NodeBase>& endNode = *std::find_if(editor.nodes.begin(), editor.nodes.end(), [endId](std::unique_ptr<DSP::NodeBase>& node) -> bool {
                            return node->getId() == endId;
                        });

                        switch(endNode->getType())
                        {
                            case DSP::Signal:
                                switch (link.end_attr & DSP::NodeBase::AttribIdMask)
                                {
                                case DSP::SignalNode::InAmplitudeAttrib:
                                    (*endNode->getSignal())->getData().amplitude = OutputSignal;
                                    break;
                                case DSP::SignalNode::InFrequencyAttrib:
                                    (*endNode->getSignal())->getData().freq = OutputSignal;
                                    break;
                                case DSP::SignalNode::InPhaseAttrib:
                                    (*endNode->getSignal())->getData().phase = OutputSignal;
                                    break;
                                case DSP::SignalNode::InDAttrib:
                                    (*endNode->getSignal())->getData().d = OutputSignal;
                                    break;
                                default:
                                    assert(false);
                                    break;
                                }
                                break;
                            case DSP::Function:
                                switch (link.end_attr & DSP::NodeBase::AttribIdMask)
                                {
                                case DSP::FunctionNode::InLeftSignalAttrib:
                                    dynamic_cast<DSP::Signals::ComplexSignal&>(*(*endNode->getSignal())).SetLeft(OutputSignal);
                                    break;
                                case DSP::FunctionNode::InRightSignalAttrib:
                                    dynamic_cast<DSP::Signals::ComplexSignal&>(*(*endNode->getSignal())).SetRight(OutputSignal);
                                    break;
                                default:
                                    assert(false);
                                    break;
                                }
                                break;
                            case DSP::Constant:
                                assert(false);
                                break;
                            case DSP::Output:
                                endNode->setSignal(OutputSignal);
                        }


                        editor.links.push_back(link);
                    }
                }
            }
            //Link destruction internal
            {
                int link_id;
                if (ImNodes::IsLinkDestroyed(&link_id))
                {
                    auto iter = std::find_if(
                        editor.links.begin(), editor.links.end(), [link_id](const Link& link) -> bool {
                            return link.id == link_id;
                        });
                    assert(iter != editor.links.end());
                    editor.links.erase(iter);
                }
            }

            //Link destruction
            {
                const int num_selected = ImNodes::NumSelectedLinks();
                if (num_selected > 0 && ImGui::IsKeyReleased(ImGuiKey_X))
                {
                    static std::vector<int> selected_links;
                    selected_links.resize(static_cast<size_t>(num_selected));
                    ImNodes::GetSelectedLinks(selected_links.data());
                    for (const int link_id : selected_links)
                    {
                        auto iter = std::find_if(
                            editor.links.begin(), editor.links.end(), [link_id](const Link& link) -> bool {
                                return link.id == link_id;
                            });
                        assert(iter != editor.links.end());
                        editor.links.erase(iter);
                    }
                }
            }
            //Node destruction
            {
                const int num_selected = ImNodes::NumSelectedNodes();
                if (num_selected > 0 && ImGui::IsKeyReleased(ImGuiKey_X))
                {
                    static std::vector<int> selected_nodes;
                    selected_nodes.resize(static_cast<size_t>(num_selected));
                    ImNodes::GetSelectedNodes(selected_nodes.data());
                    for (const int node_id : selected_nodes)
                    {
                        auto iter = std::find_if(
                            editor.nodes.begin(), editor.nodes.end(), [node_id](const std::unique_ptr<DSP::NodeBase>& node) -> bool {
                                return node->getId() == node_id;
                            });
                        assert(iter != editor.nodes.end());
                        editor.nodes.erase(iter);
                    }
                }
            }

        }ImGui::End();

        

        /*ImGui::SetNextWindowDockID(1u);
        if(ImGui::Begin("Main", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
        {
            if(ImPlot::BeginPlot("plot", ImVec2(-1, 500)))
            {
                for(int i = 0; i < plotDataSize; ++i)
                {
                    xes[i] = static_cast<double>(i);
                    plotPoints[i] = signal->get(i);
                    modulatoplotPoints[i] = modulator.get(i);
                }
  

                ImPlot::PlotLine("Plot", xes.data(), plotPoints.data(), plotPoints.size());
                ImPlot::PlotLine("modulator", xes.data(), modulatoplotPoints.data(), modulatoplotPoints.size());
                // ImPlot::PlotLineG("triangle", [](int idx, void* user_data) -> ImPlotPoint {return {double(idx), DSP::tirangle(1.0, idx, 1.0, 64.0, 0.0)};}, xes, 1000);
                // ImPlot::PlotLineG("sawtooth", [](int idx, void* user_data) -> ImPlotPoint {return {double(idx), DSP::sawtooth(1.0, idx, 1.0, 64.0, 0.0)};}, xes, 1000);
                // ImPlot::PlotLineG("noise", [](int idx, void* user_data) -> ImPlotPoint {return {double(idx), DSP::noise(1.0, idx)};}, xes, 1000);

                ImPlot::EndPlot();
            }

            // editSignal(*signal);
            static auto& modulatedPhaseObj = dynamic_cast<DSP::Signals::Constant&>(*signal->getData().phase);
            static auto& modulatorPhaseObj = dynamic_cast<Constant&>(*((dynamic_cast<SumParam&>(modulator.getRight())).getRight()).getData().phase);
            // static auto& modulatorPhaseObj = modulator;
            offset = modulatedPhaseObj.get(0);
            if(isAnimated)
                offset += 0.01;
            ImGui::Checkbox("Animate", &isAnimated);
            ImGui::SameLine();
            ImGui::InputDouble("Offset", &offset, 0.1f, 1.0);
            modulatedPhaseObj.set(offset);
            modulatorPhaseObj.set(offset);
            // dynamic_cast<DSP::Signals::Constant&>(*modulatorPhaseObj.getLeft().getData().phase).set(offset);
            // dynamic_cast<DSP::Signals::Constant&>(*modulatorPhaseObj.getRight().getData().phase).set(offset);

            if(ImGui::Button("Play"))
            {
                WAVController::PlaylayWAV(soundPoints);
            }
            if(ImGui::Button("Save"))
            {
                WAVController::CreateWAVFile("test.wav", soundPoints);
            }
        }
        ImGui::End();*/


        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
