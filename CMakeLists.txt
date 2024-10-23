cmake_minimum_required(VERSION 3.20)
project(TestProj CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ImguiPath deps/imgui/)
set(ImplotPath deps/implot/)
set(ImnodesPath deps/imnodes/)
set(ClassesPath src/classes/)

set(ImguiSources 
    ${ImguiPath}imgui_demo.cpp
    ${ImguiPath}imgui_draw.cpp
    ${ImguiPath}imgui_tables.cpp
    ${ImguiPath}imgui_widgets.cpp
    ${ImguiPath}imgui.cpp
    ${ImguiPath}backends/imgui_impl_opengl3.cpp
    ${ImguiPath}backends/imgui_impl_glfw.cpp
)

set(ImplotSources 
    ${ImplotPath}implot_demo.cpp
    ${ImplotPath}implot_items.cpp
    ${ImplotPath}implot.cpp
)

set(ImnodesSources 
    ${ImnodesPath}imnodes.cpp
)

set(classes
    ${ClassesPath}Signals/SignalData/SignalData.cpp
    ${ClassesPath}WAVController/WAVController.cpp
    ${ClassesPath}Bluprints/NodeBase.cpp
    ${ClassesPath}Bluprints/Nodes.cpp
)

add_executable(lab WIN32 src/main.cpp 
                         ${classes} 
                         ${ImguiSources} 
                         ${ImplotSources}
                         ${ImnodesSources}
)

target_compile_options(lab PRIVATE)
target_include_directories(lab PUBLIC ${ImguiPath})
target_include_directories(lab PUBLIC ${ImguiPath}backends)
target_include_directories(lab PUBLIC ${ImplotPath})
target_include_directories(lab PUBLIC ${ImnodesPath})
target_include_directories(lab PUBLIC ${ClassesPath})

add_subdirectory(deps/glfw)

target_link_libraries(lab PUBLIC 
                                stdc++exp
                                glfw
                                opengl32
                                winmm
)