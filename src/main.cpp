#include <print>
#include <cmath>
#include <algorithm>
#include <random>
#include <numbers>
#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <Windows.h>
#include <mmreg.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> 

#include "Signals/Signals.hpp"

static void glfw_error_callback(int error, const char* description)
{
    std::print(stderr, "GLFW Error %d: %s\n", error, description);
}

extern const int SAMPLE_RATE = 44100;
constexpr static int FREQUENCY = 440;
constexpr static int DURATION = 10;
static constexpr int total_size = SAMPLE_RATE * DURATION;

// void editSignal(DSP::SignalBase& sigData)
// {
//     double amplitude = sigData.getAmplitude();
//     ImGui::InputDouble("Amplitude", &amplitude);
//     sigData.setAmplitude(amplitude);

//     double freq = sigData.getFrequency();
//     ImGui::InputDouble("Frequency", &freq);
//     sigData.setFrequency(freq);

//     float phase = sigData.getPhase();
//     ImGui::SliderFloat("Phase", &phase, 0.0f, 100.0f);
//     sigData.setPhase(phase);

//     double d = sigData.getWorkingCycle();
//     ImGui::InputDouble("Working Cycle", &d);
//     sigData.setWorkingCycle(d);
// }

#pragma pack(push, 1)
struct WAVHeader {
    // RIFF Header
    char riff[4] = { 'R', 'I', 'F', 'F' };
    uint32_t fileSize;    // Size of the entire file minus 8 bytes
    char wave[4] = { 'W', 'A', 'V', 'E' };

    // Format Chunk
    char fmtChunkID[4] = { 'f', 'm', 't', ' ' };
    uint32_t fmtChunkSize = 16;  // PCM format
    uint16_t audioFormat = 3;    // IEEE Float = 3
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample / 8
    uint16_t blockAlign;     // numChannels * bitsPerSample / 8
    uint16_t bitsPerSample;

    // Data Chunk
    char dataChunkID[4] = { 'd', 'a', 't', 'a' };
    uint32_t dataChunkSize;  // Size of the audio data
};
#pragma pack(pop)

// Function to write WAV file with IEEE float audio format
void writeWAVFileIEEEFloat(const char* filename, float* data, uint32_t dataSize, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample) {
    // Create and initialize the WAV header
    WAVHeader header;
    header.numChannels = numChannels;
    header.sampleRate = sampleRate;
    header.bitsPerSample = bitsPerSample;
    header.byteRate = sampleRate * numChannels * bitsPerSample / 8;
    header.blockAlign = numChannels * bitsPerSample / 8;
    header.dataChunkSize = dataSize * numChannels * bitsPerSample / 8;
    header.fileSize = 36 + header.dataChunkSize;

    // Create the WAV file using WinAPI
    HANDLE hFile = CreateFileA(
        filename,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        std::print(stderr, "Failed to create file: {}", filename);
        return;
    }

    // Write WAV header
    DWORD bytesWritten;
    WriteFile(hFile, &header, sizeof(header), &bytesWritten, nullptr);

    // Write audio data (IEEE Float data)
    WriteFile(hFile, data, header.dataChunkSize, &bytesWritten, nullptr);

    // Close the file
    CloseHandle(hFile);
    std::print("WAV file created: {}", filename);
}

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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    
    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    std::vector<float> soundPoints(total_size);
    static constexpr int plotDataSize = total_size / 1024;
    std::vector<float> plotPoints(plotDataSize);
    std::vector<float> modulatoplotPoints(plotDataSize);
    std::vector<float> xes(plotDataSize);
    auto modulatorPtr = std::make_unique<DSP::SumParam>(
        std::make_unique<DSP::Sawtooth>(0.2, 120.0),
        std::make_unique<DSP::Pulse>(0.2, 220.0)
    );
    auto& modulator = *modulatorPtr;
    auto modulatedPtr = std::make_unique<DSP::Sin>(0.2, 440.0);
    auto& modulated = *modulatedPtr;
    std::unique_ptr<DSP::SignalBase> signal = std::move(modulatedPtr);

    auto& data = signal->getData();  
    // data.amplitude = std::make_unique<DSP::MulParam>(
    //     std::make_unique<DSP::Constant>(0.5),
    //     std::make_unique<DSP::SumParam>(
    //         std::make_unique<DSP::Constant>(1.0),
    //         std::make_unique<DSP::Pulse>(0.5, 110.0)
    //     )
    // ); 
    
    // data.phase = std::make_unique<DSP::MulParam>(
    //     std::make_unique<DSP::Constant>(1.0),
    //     std::make_unique<DSP::Sin>(0.5, 660.0)
    // );



    //auto& pulse = dynamic_cast<DSP::MulParam&>(*data.freq).getRight();

    
    



    // data.amplitude->getData().amplitude = std::make_unique<DSP::Sawtooth>();
    // data.amplitude->getData().amplitude->getData().freq = std::make_unique<DSP::Constant>(2000.0);

    double x = 0.0;
    double offset = 0.0;
    bool isAnimated = true;
    for(int i = 0; i < soundPoints.size(); ++i)
    {
        soundPoints[i] = signal->get(i); 
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(1, ImGui::GetMainViewport());

        ImGui::SetNextWindowDockID(1u);
        ImGui::ShowDemoWindow();
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
            static auto& modulatedPhaseObj = dynamic_cast<DSP::Constant&>(*modulated.getData().phase);
            static auto& modulatorPhaseObj = modulator;
            offset = modulatedPhaseObj.get(0);
            if(isAnimated)
                offset += 0.001;
            ImGui::Checkbox("Animate", &isAnimated);
            ImGui::SameLine();
            ImGui::InputDouble("Offset", &offset, 0.1f, 1.0);
            modulatedPhaseObj.set(offset);
            dynamic_cast<DSP::Constant&>(*modulatorPhaseObj.getLeft().getData().phase).set(offset);
            dynamic_cast<DSP::Constant&>(*modulatorPhaseObj.getRight().getData().phase).set(offset);

            if(ImGui::Button("Play"))
            {
                // Set up the waveform audio format
                WAVEFORMATEX wfx;
                wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
                wfx.nChannels = 1;  // Mono
                wfx.nSamplesPerSec = SAMPLE_RATE;
                wfx.wBitsPerSample = 32;  // 32-bit float audio
                wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
                wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
                wfx.cbSize = 0;


                WAVEHDR whdr;
                whdr.lpData = (LPSTR)soundPoints.data();
                whdr.dwBufferLength = total_size * sizeof(float);
                whdr.dwFlags = 0;
                whdr.dwLoops = 0;

                HWAVEOUT hWaveOut;
                if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) 
                {
                    std::print(stderr, "Failed to open wave output device.\n");
                }

                waveOutPrepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
                waveOutWrite(hWaveOut, &whdr, sizeof(WAVEHDR));

                Sleep(DURATION * 1000);

                waveOutUnprepareHeader(hWaveOut, &whdr, sizeof(WAVEHDR));
                waveOutClose(hWaveOut);
            }
            if(ImGui::Button("Save"))
            {
                writeWAVFileIEEEFloat("test.wav", soundPoints.data(), SAMPLE_RATE * DURATION, 1, SAMPLE_RATE, 32);
            }
        }
        ImGui::End();


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
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
