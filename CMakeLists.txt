# Vulcao SSAO - compila main.cpp, os shaders e baixa o que falta (bibliotecas e mapa).
cmake_minimum_required(VERSION 3.24)
project(vulcao-ssao CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) # Debug liga a camada de validacao do Vulkan
endif()

# --- Bibliotecas. Vulkan vem do sistema; as outras sao usadas do sistema se
# --- existirem ou baixadas automaticamente na primeira configuracao.
find_package(Vulkan REQUIRED)
include(FetchContent)
FetchContent_Declare(glfw  GIT_REPOSITORY https://github.com/glfw/glfw      GIT_TAG 3.4       GIT_SHALLOW ON FIND_PACKAGE_ARGS NAMES glfw3)
FetchContent_Declare(glm   GIT_REPOSITORY https://github.com/g-truc/glm     GIT_TAG 1.0.1     GIT_SHALLOW ON FIND_PACKAGE_ARGS)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui  GIT_TAG v1.91.9b  GIT_SHALLOW ON) # interface
FetchContent_Declare(cgltf GIT_REPOSITORY https://github.com/jkuhlmann/cgltf GIT_TAG v1.15    GIT_SHALLOW ON) # leitor do mapa (.glb)
FetchContent_Declare(stb   GIT_REPOSITORY https://github.com/nothings/stb   GIT_TAG 9600522de53973336fa033a289858a4354cbb04c) # leitor de JPEG/PNG
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw glm imgui cgltf stb)

# --- Mapa do vulcao (.glb, ~170 MB compactado): grande demais para o repositorio,
# --- entao e baixado uma unica vez do release da Eruption Engine e conferido por SHA-256.
set(MAPA "${CMAKE_BINARY_DIR}/assets/external/parana_field/parana_field.glb")
if(NOT EXISTS "${MAPA}")
    message(STATUS "Baixando o mapa do vulcao (~170 MB)...")
    file(DOWNLOAD "https://github.com/eruptionlabs/eruption-engine/releases/download/demo-map/parana_field.zip"
         "${CMAKE_BINARY_DIR}/mapa.zip" SHOW_PROGRESS
         EXPECTED_HASH SHA256=40396b6ac5f3d97fb6ad636d8d6d6284a7f8c750f5a54c74ddd9695533a214a3)
    file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/mapa.zip" DESTINATION "${CMAKE_BINARY_DIR}")
    file(REMOVE "${CMAKE_BINARY_DIR}/mapa.zip")
endif()

# --- Shaders: cada arquivo GLSL vira um .spv (SPIR-V, o formato que o Vulkan carrega).
find_program(GLSLC glslc REQUIRED)
set(SHADERS_SPV "")
foreach(nome gbuffer.vert gbuffer.frag tela.vert ssao.frag blur.frag final.frag)
    add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/${nome}.spv"
        COMMAND ${GLSLC} "${CMAKE_SOURCE_DIR}/shaders/${nome}" -o "${CMAKE_BINARY_DIR}/${nome}.spv"
        DEPENDS "${CMAKE_SOURCE_DIR}/shaders/${nome}" "${CMAKE_SOURCE_DIR}/shaders/comum.glsl")
    list(APPEND SHADERS_SPV "${CMAKE_BINARY_DIR}/${nome}.spv")
endforeach()

# --- Executavel: nosso main.cpp + os arquivos da ImGui.
add_executable(vulcao main.cpp ${SHADERS_SPV}
    ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)
target_include_directories(vulcao PRIVATE ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${cgltf_SOURCE_DIR} ${stb_SOURCE_DIR})
target_link_libraries(vulcao PRIVATE Vulkan::Vulkan glfw glm::glm)
# Caminhos que o programa usa para achar os shaders compilados e o mapa.
target_compile_definitions(vulcao PRIVATE PASTA_SHADERS="${CMAKE_BINARY_DIR}" ARQUIVO_MAPA="${MAPA}")
