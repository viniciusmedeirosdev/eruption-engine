// =====================================================================================
//  VULCAO SSAO - demonstracao de Oclusao de Ambiente (SSAO) em Vulkan 1.3
//
//  O programa inteiro esta neste arquivo e segue esta ordem:
//    1. Estado global e funcoes auxiliares (memoria, buffers, imagens, comandos)
//    2. Inicializacao do Vulkan (janela, instancia, GPU, dispositivo)
//    3. Swapchain e imagens intermediarias (G-buffer e oclusao)
//    4. Carregamento do mapa do vulcao (.glb): geometria e texturas
//    5. Descritores e pipelines (um pipeline por passo de renderizacao)
//    6. Camera livre e interface de controles
//    7. Desenho de um quadro (os 4 passos) e funcao main
//
//  Os 4 passos de cada quadro (um shader para cada, na pasta shaders/):
//    1. G-buffer : desenha o mapa guardando cor, normal e profundidade   (gbuffer.vert/.frag)
//    2. SSAO     : calcula a oclusao de ambiente a partir do G-buffer    (ssao.frag)
//    3. Blur     : suaviza o ruido da oclusao                            (blur.frag)
//    4. Final    : ilumina a cena usando a oclusao e desenha a interface (final.frag)
// =====================================================================================
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // profundidade de 0 a 1, como o Vulkan usa
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>                  // leitor de arquivos glTF/.glb (o formato do mapa)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>              // decodificador de JPEG/PNG (as texturas do mapa)
#include <imgui.h>                  // interface com os controles deslizantes
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

// ============================ 1. ESTADO GLOBAL E AUXILIARES ============================

// Parametros enviados aos shaders a cada quadro. Precisa ser identico ao bloco
// "Parametros" de shaders/comum.glsl. Os valores abaixo sao os iniciais da interface.
struct Parametros {
    glm::mat4 viewProj, invViewProj;
    glm::vec4 posCamera;
    glm::vec4 dirSol = glm::vec4(glm::normalize(glm::vec3(-0.47f, 0.82f, -0.33f)), 0);
    float raio = 12, intensidade = 1.5f, bias = 0.1f, contraste = 1;
    int   amostras = 16, ssaoLigado = 1, blurLigado = 1, modo = 0;
    float divisoria = 0, luzAmbiente = 1, luzSol = 1, aoNaLuzDireta = 0;
} params;

struct Buffer { VkBuffer buf; VkDeviceMemory mem; };                   // buffer + sua memoria
struct Imagem { VkImage img; VkDeviceMemory mem; VkImageView vista; }; // imagem + memoria + vista
struct Vertice { glm::vec3 pos, normal; glm::vec2 uv; };               // formato dos vertices do mapa
struct Desenho {                                                       // um objeto a desenhar
    glm::mat4 modelo;                    // onde ele fica no mundo
    uint32_t  primeiroIndice, numIndices; // qual trecho do buffer de indices e a sua malha
    int       textura;                   // qual textura usar
};

// Objetos principais do Vulkan.
GLFWwindow*      janela;
VkInstance       instancia;
VkSurfaceKHR     superficie;    // ligacao entre o Vulkan e a janela
VkPhysicalDevice gpu;           // a placa de video escolhida
VkDevice         dev;           // conexao logica com a placa
uint32_t         familiaFila;   // tipo de fila que aceita desenho e apresentacao
VkQueue          fila;          // por onde os comandos sao enviados a GPU
VkCommandPool    poolComandos;
VkCommandBuffer  cmd;           // lista de comandos, regravada a cada quadro
VkSemaphore      semImagemLivre, semDesenhoPronto; // sincronizam GPU e apresentacao
VkFence          cercaQuadro;                      // avisa a CPU que a GPU terminou o quadro

// Swapchain (imagens mostradas na janela) e imagens intermediarias.
VkSwapchainKHR           swapchain;
VkFormat                 formatoTela;
VkExtent2D               tamanho;       // largura e altura atuais, em pixels
std::vector<VkImage>     imagensTela;
std::vector<VkImageView> vistasTela;
Imagem gAlbedo, gNormal, gProfundidade; // G-buffer
Imagem aoBruto, aoSuave;                // oclusao antes e depois do blur

// Dados do mapa.
Buffer bufVertices, bufIndices, bufParametros;
void*  memParametros;                   // endereco onde a CPU escreve os parametros
std::vector<Imagem>  texturas;
std::vector<Desenho> desenhos;

// Descritores (ligam imagens/buffers aos shaders) e pipelines.
VkSampler             amostradorTextura, amostradorTela;
VkDescriptorPool      poolDescritores;
VkDescriptorSetLayout layoutGlobal, layoutMaterial;
VkDescriptorSet       setGlobal;                // parametros + G-buffer + oclusao + camadas do chao
std::vector<VkDescriptorSet> setsMaterial;      // um por textura do mapa
VkPipelineLayout      layoutGeometria, layoutTela;
VkPipeline            pipeGBuffer, pipeSSAO, pipeBlur, pipeFinal;

// Camera livre.
glm::vec3 camPos(1900, 560, 1850);      // posicao inicial: de frente para o vulcao
float camGiro = 0.78f, camInclinacao = -0.05f; // angulos em radianos (horizontal, vertical)
float camVelocidade = 150;              // unidades de mundo por segundo
float distanciaObjetos = 1500;          // arvores e pedras alem disso nao sao desenhadas

// Filas de limpeza: cada recurso criado registra aqui como deve ser destruido.
// "limpezaTela" guarda o que depende do tamanho da janela (recriado ao redimensionar).
std::vector<std::function<void()>> limpezaGeral, limpezaTela;
void executar(std::vector<std::function<void()>>& lista) {
    for (auto it = lista.rbegin(); it != lista.rend(); ++it) (*it)(); // ordem inversa da criacao
    lista.clear();
}

// Encerra o programa com uma mensagem se uma chamada do Vulkan falhar.
void conferir(VkResult r, const char* onde) {
    if (r != VK_SUCCESS) { fprintf(stderr, "Erro do Vulkan em: %s (codigo %d)\n", onde, r); exit(1); }
}

// Reserva memoria da GPU com as propriedades pedidas (ex.: visivel pela CPU, ou so da GPU).
VkDeviceMemory alocar(VkMemoryRequirements req, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(gpu, &mp);
    uint32_t tipo = 0;
    while (!((req.memoryTypeBits & (1u << tipo)) && (mp.memoryTypes[tipo].propertyFlags & props) == props)) tipo++;
    VkMemoryAllocateInfo ai{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = req.size, .memoryTypeIndex = tipo};
    VkDeviceMemory mem;
    conferir(vkAllocateMemory(dev, &ai, nullptr, &mem), "alocar memoria");
    return mem;
}

// Cria um buffer (bloco de dados na GPU: vertices, indices, parametros...).
Buffer criarBuffer(VkDeviceSize bytes, VkBufferUsageFlags uso, VkMemoryPropertyFlags props) {
    Buffer b;
    VkBufferCreateInfo ci{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = bytes, .usage = uso};
    conferir(vkCreateBuffer(dev, &ci, nullptr, &b.buf), "criar buffer");
    VkMemoryRequirements req;
    vkGetBufferMemoryRequirements(dev, b.buf, &req);
    b.mem = alocar(req, props);
    vkBindBufferMemory(dev, b.buf, b.mem, 0);
    return b;
}
void destruir(Buffer b) { vkDestroyBuffer(dev, b.buf, nullptr); vkFreeMemory(dev, b.mem, nullptr); }

// Cria uma imagem 2D na GPU (textura ou alvo de renderizacao) com sua vista.
Imagem criarImagem(uint32_t larg, uint32_t alt, VkFormat formato, VkImageUsageFlags uso, uint32_t mips = 1) {
    Imagem im;
    VkImageCreateInfo ci{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType = VK_IMAGE_TYPE_2D, .format = formato,
        .extent = {larg, alt, 1}, .mipLevels = mips, .arrayLayers = 1, .samples = VK_SAMPLE_COUNT_1_BIT, .usage = uso};
    conferir(vkCreateImage(dev, &ci, nullptr, &im.img), "criar imagem");
    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(dev, im.img, &req);
    im.mem = alocar(req, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindImageMemory(dev, im.img, im.mem, 0);
    VkImageAspectFlags aspecto = formato == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    VkImageViewCreateInfo vi{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = im.img, .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = formato, .subresourceRange = {aspecto, 0, mips, 0, 1}};
    conferir(vkCreateImageView(dev, &vi, nullptr, &im.vista), "criar vista de imagem");
    return im;
}
void destruir(Imagem im) {
    vkDestroyImageView(dev, im.vista, nullptr); vkDestroyImage(dev, im.img, nullptr); vkFreeMemory(dev, im.mem, nullptr);
}

// Muda o "layout" de uma imagem. O Vulkan exige que a imagem esteja no layout certo
// para cada uso: receber copia, ser alvo de desenho, ser lida por shader, ir para a tela.
void transicao(VkImage img, VkImageLayout de, VkImageLayout para,
               VkImageAspectFlags aspecto = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t mip = 0, uint32_t numMips = VK_REMAINING_MIP_LEVELS) {
    VkImageMemoryBarrier2 b{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT, .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
        .oldLayout = de, .newLayout = para, .image = img, .subresourceRange = {aspecto, mip, numMips, 0, 1}};
    VkDependencyInfo d{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &b};
    vkCmdPipelineBarrier2(cmd, &d);
}

// Comandos avulsos (usados no carregamento): grava, envia e espera a GPU terminar.
void iniciarComandos() {
    VkCommandBufferBeginInfo bi{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cmd, &bi);
}
void enviarComandos() {
    vkEndCommandBuffer(cmd);
    VkSubmitInfo si{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd};
    conferir(vkQueueSubmit(fila, 1, &si, VK_NULL_HANDLE), "enviar comandos");
    vkQueueWaitIdle(fila);
}

// Cria um buffer na GPU ja preenchido com "dados". Os dados passam por um buffer
// temporario visivel pela CPU e sao copiados para a memoria rapida da placa.
Buffer enviarParaGPU(const void* dados, VkDeviceSize bytes, VkBufferUsageFlags uso) {
    Buffer temp = criarBuffer(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* destino;
    vkMapMemory(dev, temp.mem, 0, bytes, 0, &destino);
    memcpy(destino, dados, bytes);
    Buffer b = criarBuffer(bytes, uso | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    iniciarComandos();
    VkBufferCopy copia{.size = bytes};
    vkCmdCopyBuffer(cmd, temp.buf, b.buf, 1, &copia);
    enviarComandos();
    destruir(temp);
    limpezaGeral.push_back([b] { destruir(b); });
    return b;
}

// ============================ 2. INICIALIZACAO DO VULKAN ============================

void iniciarVulkan() {
    // Janela (GLFW). NO_API = sem OpenGL; quem desenha e o Vulkan.
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    janela = glfwCreateWindow(1280, 720, "Vulcao SSAO", nullptr, nullptr);

    // Instancia: ponto de entrada do Vulkan. As extensoes pedidas pelo GLFW permitem
    // desenhar em janelas. Em compilacao Debug liga a camada de validacao (se instalada),
    // que confere cada chamada e imprime os erros de uso no terminal.
    uint32_t numExt;
    const char** extensoes = glfwGetRequiredInstanceExtensions(&numExt);
    std::vector<const char*> camadas;
#ifndef NDEBUG
    uint32_t numCamadas;
    vkEnumerateInstanceLayerProperties(&numCamadas, nullptr);
    std::vector<VkLayerProperties> disponiveis(numCamadas);
    vkEnumerateInstanceLayerProperties(&numCamadas, disponiveis.data());
    for (auto& c : disponiveis)
        if (!strcmp(c.layerName, "VK_LAYER_KHRONOS_validation")) camadas.push_back("VK_LAYER_KHRONOS_validation");
#endif
    VkApplicationInfo app{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .pApplicationName = "Vulcao SSAO", .apiVersion = VK_API_VERSION_1_3};
    VkInstanceCreateInfo ici{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app,
        .enabledLayerCount = (uint32_t)camadas.size(), .ppEnabledLayerNames = camadas.data(),
        .enabledExtensionCount = numExt, .ppEnabledExtensionNames = extensoes};
    conferir(vkCreateInstance(&ici, nullptr, &instancia), "criar instancia (o driver suporta Vulkan 1.3?)");
    conferir(glfwCreateWindowSurface(instancia, janela, nullptr, &superficie), "criar superficie da janela");

    // Escolha da GPU: usa a primeira encontrada, mas prefere uma placa dedicada.
    uint32_t numGpus = 0;
    vkEnumeratePhysicalDevices(instancia, &numGpus, nullptr);
    std::vector<VkPhysicalDevice> gpus(numGpus);
    vkEnumeratePhysicalDevices(instancia, &numGpus, gpus.data());
    if (gpus.empty()) { fprintf(stderr, "Nenhuma GPU com Vulkan encontrada.\n"); exit(1); }
    gpu = gpus[0];
    for (auto g : gpus) {
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(g, &prop);
        if (prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) gpu = g;
    }

    // Familia de filas: procura uma que saiba desenhar E apresentar na janela.
    uint32_t numFamilias = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &numFamilias, nullptr);
    std::vector<VkQueueFamilyProperties> familias(numFamilias);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &numFamilias, familias.data());
    for (familiaFila = 0; familiaFila < numFamilias; familiaFila++) {
        VkBool32 apresenta = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, familiaFila, superficie, &apresenta);
        if ((familias[familiaFila].queueFlags & VK_QUEUE_GRAPHICS_BIT) && apresenta) break;
    }

    // Dispositivo logico, com os recursos usados pelo programa:
    //  - dynamicRendering: desenhar sem criar objetos de render pass/framebuffer
    //  - synchronization2: forma moderna de trocar o layout das imagens
    //  - samplerAnisotropy: texturas nitidas em superficies vistas de lado
    //  - shaderSampledImageArrayDynamicIndexing: o shader do chao percorre um vetor de texturas
    float prioridade = 1;
    VkDeviceQueueCreateInfo qci{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = familiaFila,
        .queueCount = 1, .pQueuePriorities = &prioridade};
    VkPhysicalDeviceVulkan13Features rec13{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .synchronization2 = VK_TRUE, .dynamicRendering = VK_TRUE};
    VkPhysicalDeviceFeatures2 recursos{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &rec13,
        .features = {.samplerAnisotropy = VK_TRUE, .shaderSampledImageArrayDynamicIndexing = VK_TRUE}};
    const char* extDispositivo[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo dci{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = &recursos, .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &qci, .enabledExtensionCount = 1, .ppEnabledExtensionNames = extDispositivo};
    conferir(vkCreateDevice(gpu, &dci, nullptr, &dev), "criar dispositivo (a GPU suporta Vulkan 1.3?)");
    vkGetDeviceQueue(dev, familiaFila, 0, &fila);

    // Buffer de comandos (um so, regravado a cada quadro) e objetos de sincronizacao.
    VkCommandPoolCreateInfo pci{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, .queueFamilyIndex = familiaFila};
    vkCreateCommandPool(dev, &pci, nullptr, &poolComandos);
    VkCommandBufferAllocateInfo cai{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = poolComandos,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1};
    vkAllocateCommandBuffers(dev, &cai, &cmd);
    VkSemaphoreCreateInfo sci{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(dev, &sci, nullptr, &semImagemLivre);
    vkCreateSemaphore(dev, &sci, nullptr, &semDesenhoPronto);
    VkFenceCreateInfo fci{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    vkCreateFence(dev, &fci, nullptr, &cercaQuadro);

    limpezaGeral.push_back([] {
        vkDestroyFence(dev, cercaQuadro, nullptr);
        vkDestroySemaphore(dev, semDesenhoPronto, nullptr);
        vkDestroySemaphore(dev, semImagemLivre, nullptr);
        vkDestroyCommandPool(dev, poolComandos, nullptr);
    });
}

// ==================== 3. SWAPCHAIN E IMAGENS INTERMEDIARIAS ====================

// Cria tudo o que depende do tamanho da janela. E chamada de novo ao redimensionar.
void criarTela() {
    // Janela minimizada tem tamanho zero: espera ela voltar.
    int larg = 0, alt = 0;
    glfwGetFramebufferSize(janela, &larg, &alt);
    while (larg == 0 || alt == 0) { glfwWaitEvents(); glfwGetFramebufferSize(janela, &larg, &alt); }
    vkDeviceWaitIdle(dev);
    executar(limpezaTela); // destroi as imagens do tamanho antigo (se houver)

    VkSurfaceCapabilitiesKHR cap;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, superficie, &cap);
    tamanho = cap.currentExtent.width != UINT32_MAX ? cap.currentExtent : VkExtent2D{(uint32_t)larg, (uint32_t)alt};

    // Formato de cor da tela: prefere sRGB (o driver aplica a correcao de gama sozinho).
    uint32_t numFormatos = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, superficie, &numFormatos, nullptr);
    std::vector<VkSurfaceFormatKHR> formatos(numFormatos);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, superficie, &numFormatos, formatos.data());
    VkSurfaceFormatKHR escolhido = formatos[0];
    for (auto& f : formatos) if (f.format == VK_FORMAT_B8G8R8A8_SRGB) escolhido = f;
    formatoTela = escolhido.format;

    // Swapchain: a fila de imagens que se revezam na janela. FIFO = sincronizado com o monitor.
    uint32_t numImagens = cap.minImageCount + 1;
    if (cap.maxImageCount > 0) numImagens = std::min(numImagens, cap.maxImageCount);
    VkSwapchainCreateInfoKHR sci{.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, .surface = superficie,
        .minImageCount = numImagens, .imageFormat = formatoTela, .imageColorSpace = escolhido.colorSpace,
        .imageExtent = tamanho, .imageArrayLayers = 1, .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = cap.currentTransform, .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, .clipped = VK_TRUE};
    conferir(vkCreateSwapchainKHR(dev, &sci, nullptr, &swapchain), "criar swapchain");
    vkGetSwapchainImagesKHR(dev, swapchain, &numImagens, nullptr);
    imagensTela.resize(numImagens);
    vistasTela.resize(numImagens);
    vkGetSwapchainImagesKHR(dev, swapchain, &numImagens, imagensTela.data());
    for (uint32_t i = 0; i < numImagens; i++) {
        VkImageViewCreateInfo vi{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = imagensTela[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = formatoTela, .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCreateImageView(dev, &vi, nullptr, &vistasTela[i]);
    }

    // Imagens intermediarias, do tamanho da janela. Cada uma e desenhada em um passo
    // (ATTACHMENT) e lida por shaders nos passos seguintes (SAMPLED).
    VkImageUsageFlags cor = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gAlbedo       = criarImagem(tamanho.width, tamanho.height, VK_FORMAT_R8G8B8A8_SRGB, cor);       // cor da superficie
    gNormal       = criarImagem(tamanho.width, tamanho.height, VK_FORMAT_R16G16B16A16_SFLOAT, cor); // direcao da superficie
    gProfundidade = criarImagem(tamanho.width, tamanho.height, VK_FORMAT_D32_SFLOAT,
                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    aoBruto       = criarImagem(tamanho.width, tamanho.height, VK_FORMAT_R8_UNORM, cor);            // saida do SSAO
    aoSuave       = criarImagem(tamanho.width, tamanho.height, VK_FORMAT_R8_UNORM, cor);            // saida do blur

    limpezaTela.push_back([] {
        for (Imagem im : {gAlbedo, gNormal, gProfundidade, aoBruto, aoSuave}) destruir(im);
        for (auto v : vistasTela) vkDestroyImageView(dev, v, nullptr);
        vkDestroySwapchainKHR(dev, swapchain, nullptr);
    });
}

// ============================ 4. CARREGAMENTO DO MAPA ============================

// Envia uma imagem RGBA para a GPU e gera seus mipmaps (versoes menores da textura,
// usadas quando ela aparece longe; sem elas a imagem cintila ao mover a camera).
Imagem criarTextura(const unsigned char* pixels, int larg, int alt, VkFormat formato) {
    uint32_t mips = (uint32_t)std::floor(std::log2(std::max(larg, alt))) + 1;
    VkDeviceSize bytes = (VkDeviceSize)larg * alt * 4;
    Buffer temp = criarBuffer(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* destino;
    vkMapMemory(dev, temp.mem, 0, bytes, 0, &destino);
    memcpy(destino, pixels, bytes);
    Imagem im = criarImagem(larg, alt, formato,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mips);

    iniciarComandos();
    // Copia os pixels para o nivel 0 (tamanho original).
    transicao(im.img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkBufferImageCopy copia{.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, .imageExtent = {(uint32_t)larg, (uint32_t)alt, 1}};
    vkCmdCopyBufferToImage(cmd, temp.buf, im.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copia);
    // Cada nivel seguinte e o anterior reduzido pela metade.
    for (uint32_t m = 1; m < mips; m++) {
        transicao(im.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, m - 1, 1);
        VkImageBlit reducao{
            .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, m - 1, 0, 1},
            .srcOffsets = {{0, 0, 0}, {std::max(larg >> (m - 1), 1), std::max(alt >> (m - 1), 1), 1}},
            .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, m, 0, 1},
            .dstOffsets = {{0, 0, 0}, {std::max(larg >> m, 1), std::max(alt >> m, 1), 1}}};
        vkCmdBlitImage(cmd, im.img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, im.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &reducao, VK_FILTER_LINEAR);
    }
    // Deixa todos os niveis prontos para leitura pelos shaders.
    transicao(im.img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, mips - 1, 1);
    transicao(im.img, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
    enviarComandos();
    destruir(temp);
    limpezaGeral.push_back([im] { destruir(im); });
    return im;
}

// Le o arquivo .glb do mapa: malhas (vertices e indices), texturas e a posicao de
// cada objeto. O chao e a malha 0; arvores, plantas e pedras sao malhas repetidas
// milhares de vezes, cada repeticao com sua matriz de posicao ("no" do glTF).
void carregarMapa(const char* caminho) {
    cgltf_options opcoes{};
    cgltf_data* mapa = nullptr;
    if (cgltf_parse_file(&opcoes, caminho, &mapa) != cgltf_result_success ||
        cgltf_load_buffers(&opcoes, mapa, caminho) != cgltf_result_success) {
        fprintf(stderr, "Nao foi possivel abrir o mapa: %s\n", caminho);
        exit(1);
    }

    // --- Texturas: decodifica cada JPEG/PNG embutido e envia para a GPU.
    for (size_t i = 0; i < mapa->images_count; i++) {
        printf("\rCarregando texturas: %zu/%zu", i + 1, mapa->images_count);
        fflush(stdout);
        const cgltf_buffer_view* dados = mapa->images[i].buffer_view;
        int larg, alt;
        unsigned char* pixels = stbi_load_from_memory(cgltf_buffer_view_data(dados), (int)dados->size, &larg, &alt, nullptr, 4);
        // Imagens 9 e 10 sao as mascaras do chao: guardam pesos, nao cores, entao nao usam sRGB.
        VkFormat formato = (i == 9 || i == 10) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB;
        texturas.push_back(criarTextura(pixels, larg, alt, formato));
        stbi_image_free(pixels);
    }

    // --- Geometria: junta todas as malhas em um unico vetor de vertices e um de indices.
    struct Parte { uint32_t primeiroIndice, numIndices; int textura; };
    std::vector<std::vector<Parte>> partesDaMalha(mapa->meshes_count);
    std::vector<Vertice>  vertices;
    std::vector<uint32_t> indices;
    for (size_t m = 0; m < mapa->meshes_count; m++) {
        for (size_t k = 0; k < mapa->meshes[m].primitives_count; k++) {
            const cgltf_primitive& prim = mapa->meshes[m].primitives[k];
            // Partes sem textura (marcadores auxiliares do mapa) nao sao desenhadas.
            if (!prim.material || !prim.material->pbr_metallic_roughness.base_color_texture.texture) continue;
            int textura = (int)cgltf_image_index(mapa, prim.material->pbr_metallic_roughness.base_color_texture.texture->image);

            uint32_t base = (uint32_t)vertices.size();
            for (size_t a = 0; a < prim.attributes_count; a++) {
                const cgltf_attribute& atr = prim.attributes[a];
                if (atr.type == cgltf_attribute_type_position) vertices.resize(base + atr.data->count);
            }
            for (size_t a = 0; a < prim.attributes_count; a++) {
                const cgltf_attribute& atr = prim.attributes[a];
                for (size_t v = 0; v < atr.data->count; v++) {
                    Vertice& vert = vertices[base + v];
                    if (atr.type == cgltf_attribute_type_position) cgltf_accessor_read_float(atr.data, v, &vert.pos.x, 3);
                    if (atr.type == cgltf_attribute_type_normal)   cgltf_accessor_read_float(atr.data, v, &vert.normal.x, 3);
                    if (atr.type == cgltf_attribute_type_texcoord && atr.index == 0) cgltf_accessor_read_float(atr.data, v, &vert.uv.x, 2);
                }
            }
            partesDaMalha[m].push_back({(uint32_t)indices.size(), (uint32_t)prim.indices->count, textura});
            for (size_t i = 0; i < prim.indices->count; i++)
                indices.push_back(base + (uint32_t)cgltf_accessor_read_index(prim.indices, i));
        }
    }
    bufVertices = enviarParaGPU(vertices.data(), vertices.size() * sizeof(Vertice), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    bufIndices  = enviarParaGPU(indices.data(), indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    // --- Lista de desenho: uma entrada para cada parte de cada objeto posicionado no mapa.
    for (size_t n = 0; n < mapa->nodes_count; n++) {
        if (!mapa->nodes[n].mesh) continue;
        glm::mat4 modelo;
        cgltf_node_transform_world(&mapa->nodes[n], &modelo[0][0]);
        for (const Parte& parte : partesDaMalha[cgltf_mesh_index(mapa, mapa->nodes[n].mesh)])
            desenhos.push_back({modelo, parte.primeiroIndice, parte.numIndices, parte.textura});
    }
    printf("\nMapa carregado: %zu objetos, %zu vertices, %zu texturas\n", desenhos.size(), vertices.size(), texturas.size());
    cgltf_free(mapa);
}

// ============================ 5. DESCRITORES E PIPELINES ============================

// Aponta o conjunto global de descritores para as imagens atuais do G-buffer e da
// oclusao. E chamada de novo ao redimensionar, pois essas imagens sao recriadas.
void ligarImagensDaTela() {
    VkDescriptorImageInfo info[4] = {
        {amostradorTela, gAlbedo.vista,       VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL},  // binding 1
        {amostradorTela, gNormal.vista,       VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL},  // binding 2
        {amostradorTela, gProfundidade.vista, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL},  // binding 3
        {amostradorTela, aoBruto.vista,       VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL}}; // binding 4
    VkDescriptorImageInfo infoSuave{amostradorTela, aoSuave.vista, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL}; // binding 5
    VkWriteDescriptorSet w[5];
    for (uint32_t i = 0; i < 5; i++)
        w[i] = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = setGlobal, .dstBinding = i + 1, .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = i < 4 ? &info[i] : &infoSuave};
    vkUpdateDescriptorSets(dev, 5, w, 0, nullptr);
}

void criarDescritores() {
    // Amostradores: como os shaders leem as imagens.
    //  - texturas do mapa: filtro suave, repeticao, mipmaps e anisotropia
    //  - imagens de tela (G-buffer/oclusao): pixel exato, sem repetir nas bordas
    VkSamplerCreateInfo st{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = VK_FILTER_LINEAR, .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, .anisotropyEnable = VK_TRUE, .maxAnisotropy = 8, .maxLod = VK_LOD_CLAMP_NONE};
    vkCreateSampler(dev, &st, nullptr, &amostradorTextura);
    VkSamplerCreateInfo sa{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .magFilter = VK_FILTER_NEAREST, .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
    vkCreateSampler(dev, &sa, nullptr, &amostradorTela);

    // Buffer dos parametros, mapeado permanentemente: a CPU escreve nele todo quadro.
    bufParametros = criarBuffer(sizeof(Parametros), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkMapMemory(dev, bufParametros.mem, 0, sizeof(Parametros), 0, &memParametros);

    // Layout global (set 0), visivel em todos os shaders:
    //   0 = parametros | 1..5 = albedo, normal, profundidade, AO bruto, AO suave | 6 = 10 camadas do chao
    VkShaderStageFlags estagios = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutBinding bg[7];
    bg[0] = {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, estagios, nullptr};
    for (uint32_t i = 1; i <= 5; i++) bg[i] = {i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, estagios, nullptr};
    bg[6] = {6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10, estagios, nullptr};
    VkDescriptorSetLayoutCreateInfo lg{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 7, .pBindings = bg};
    vkCreateDescriptorSetLayout(dev, &lg, nullptr, &layoutGlobal);

    // Layout de material (set 1): apenas a textura do objeto que esta sendo desenhado.
    VkDescriptorSetLayoutBinding bm{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
    VkDescriptorSetLayoutCreateInfo lm{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 1, .pBindings = &bm};
    vkCreateDescriptorSetLayout(dev, &lm, nullptr, &layoutMaterial);

    // Reserva os conjuntos: 1 global + 1 por textura.
    uint32_t numTex = (uint32_t)texturas.size();
    VkDescriptorPoolSize tamanhos[] = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 15 + numTex}};
    VkDescriptorPoolCreateInfo dp{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = 1 + numTex, .poolSizeCount = 2, .pPoolSizes = tamanhos};
    vkCreateDescriptorPool(dev, &dp, nullptr, &poolDescritores);
    std::vector<VkDescriptorSetLayout> layouts(numTex, layoutMaterial);
    setsMaterial.resize(numTex);
    VkDescriptorSetAllocateInfo ag{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = poolDescritores,
        .descriptorSetCount = 1, .pSetLayouts = &layoutGlobal};
    conferir(vkAllocateDescriptorSets(dev, &ag, &setGlobal), "reservar descritor global");
    VkDescriptorSetAllocateInfo am{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = poolDescritores,
        .descriptorSetCount = numTex, .pSetLayouts = layouts.data()};
    conferir(vkAllocateDescriptorSets(dev, &am, setsMaterial.data()), "reservar descritores de material");

    // Preenche: parametros (binding 0), camadas do chao (binding 6 = texturas 1 a 10 do mapa)...
    VkDescriptorBufferInfo infoParametros{bufParametros.buf, 0, sizeof(Parametros)};
    VkDescriptorImageInfo infoCamadas[10];
    for (int i = 0; i < 10; i++) infoCamadas[i] = {amostradorTextura, texturas[i + 1].vista, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL};
    VkWriteDescriptorSet w[2] = {
        {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = setGlobal, .dstBinding = 0, .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .pBufferInfo = &infoParametros},
        {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = setGlobal, .dstBinding = 6, .descriptorCount = 10,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = infoCamadas}};
    vkUpdateDescriptorSets(dev, 2, w, 0, nullptr);
    // ...as imagens de tela (bindings 1 a 5) e um conjunto para cada textura do mapa.
    ligarImagensDaTela();
    for (uint32_t i = 0; i < numTex; i++) {
        VkDescriptorImageInfo info{amostradorTextura, texturas[i].vista, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL};
        VkWriteDescriptorSet wm{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = setsMaterial[i], .dstBinding = 0,
            .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = &info};
        vkUpdateDescriptorSets(dev, 1, &wm, 0, nullptr);
    }

    limpezaGeral.push_back([] {
        vkDestroyDescriptorPool(dev, poolDescritores, nullptr);
        vkDestroyDescriptorSetLayout(dev, layoutMaterial, nullptr);
        vkDestroyDescriptorSetLayout(dev, layoutGlobal, nullptr);
        destruir(bufParametros);
        vkDestroySampler(dev, amostradorTela, nullptr);
        vkDestroySampler(dev, amostradorTextura, nullptr);
    });
}

// Carrega um shader ja compilado (.spv) gerado pelo CMake a partir da pasta shaders/.
VkShaderModule carregarShader(const std::string& nome) {
    std::ifstream arquivo(std::string(PASTA_SHADERS) + "/" + nome + ".spv", std::ios::binary);
    std::vector<char> codigo((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    if (codigo.empty()) { fprintf(stderr, "Shader nao encontrado: %s\n", nome.c_str()); exit(1); }
    VkShaderModuleCreateInfo ci{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = codigo.size(), .pCode = (const uint32_t*)codigo.data()};
    VkShaderModule modulo;
    conferir(vkCreateShaderModule(dev, &ci, nullptr, &modulo), "criar shader");
    return modulo;
}

// Cria um pipeline: a "receita" completa de um passo de desenho (quais shaders, formato
// dos vertices, teste de profundidade, formato das imagens de saida).
//   geometria = true : passo do G-buffer (le vertices do mapa e usa profundidade)
//   geometria = false: passos de tela cheia (SSAO, blur, final), sem vertices
VkPipeline criarPipeline(const char* vert, const char* frag, VkPipelineLayout layout, std::vector<VkFormat> formatosCor, bool geometria) {
    VkShaderModule mv = carregarShader(vert), mf = carregarShader(frag);
    VkPipelineShaderStageCreateInfo estagios[2] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = mv, .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = mf, .pName = "main"}};

    // Formato dos vertices (struct Vertice): posicao, normal e coordenada de textura.
    VkVertexInputBindingDescription vb{0, sizeof(Vertice), VK_VERTEX_INPUT_RATE_VERTEX};
    VkVertexInputAttributeDescription va[3] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertice, pos)},
        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertice, normal)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertice, uv)}};
    VkPipelineVertexInputStateCreateInfo entrada{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    if (geometria) { entrada.vertexBindingDescriptionCount = 1; entrada.pVertexBindingDescriptions = &vb;
                     entrada.vertexAttributeDescriptionCount = 3; entrada.pVertexAttributeDescriptions = va; }

    VkPipelineInputAssemblyStateCreateInfo montagem{.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    // Viewport e recorte sao definidos na hora de desenhar (mudam com o tamanho da janela).
    VkPipelineViewportStateCreateInfo viewport{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};
    VkDynamicState dinamicos[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dinamico{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = dinamicos};
    // Sem descarte de faces: as folhas da vegetacao sao visiveis pelos dois lados.
    VkPipelineRasterizationStateCreateInfo raster{.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = VK_CULL_MODE_NONE, .frontFace = VK_FRONT_FACE_CLOCKWISE, .lineWidth = 1};
    VkPipelineMultisampleStateCreateInfo amostragem{.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    // Teste de profundidade: so o pixel mais proximo da camera sobrevive.
    VkPipelineDepthStencilStateCreateInfo profundidade{.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = geometria, .depthWriteEnable = geometria, .depthCompareOp = VK_COMPARE_OP_LESS};
    // Uma configuracao de escrita por imagem de saida (sem transparencia).
    std::vector<VkPipelineColorBlendAttachmentState> mistura(formatosCor.size(), VkPipelineColorBlendAttachmentState{.colorWriteMask = 0xF});
    VkPipelineColorBlendStateCreateInfo cores{.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = (uint32_t)mistura.size(), .pAttachments = mistura.data()};
    // Formatos das imagens em que este pipeline desenha (dynamic rendering).
    VkPipelineRenderingCreateInfo saida{.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = (uint32_t)formatosCor.size(), .pColorAttachmentFormats = formatosCor.data(),
        .depthAttachmentFormat = geometria ? VK_FORMAT_D32_SFLOAT : VK_FORMAT_UNDEFINED};

    VkGraphicsPipelineCreateInfo ci{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, .pNext = &saida, .stageCount = 2,
        .pStages = estagios, .pVertexInputState = &entrada, .pInputAssemblyState = &montagem, .pViewportState = &viewport,
        .pRasterizationState = &raster, .pMultisampleState = &amostragem, .pDepthStencilState = &profundidade,
        .pColorBlendState = &cores, .pDynamicState = &dinamico, .layout = layout};
    VkPipeline pipeline;
    conferir(vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &ci, nullptr, &pipeline), "criar pipeline");
    vkDestroyShaderModule(dev, mv, nullptr);
    vkDestroyShaderModule(dev, mf, nullptr);
    limpezaGeral.push_back([pipeline] { vkDestroyPipeline(dev, pipeline, nullptr); });
    return pipeline;
}

void criarPipelines() {
    // Layout do G-buffer: set 0 (global) + set 1 (material) + dados por objeto (matriz e flag de terreno).
    VkDescriptorSetLayout sets[] = {layoutGlobal, layoutMaterial};
    VkPushConstantRange porObjeto{VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4) + sizeof(int)};
    VkPipelineLayoutCreateInfo lg{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 2, .pSetLayouts = sets,
        .pushConstantRangeCount = 1, .pPushConstantRanges = &porObjeto};
    vkCreatePipelineLayout(dev, &lg, nullptr, &layoutGeometria);
    // Layout dos passos de tela cheia: so o set 0 (global).
    VkPipelineLayoutCreateInfo lt{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = sets};
    vkCreatePipelineLayout(dev, &lt, nullptr, &layoutTela);
    limpezaGeral.push_back([] {
        vkDestroyPipelineLayout(dev, layoutTela, nullptr);
        vkDestroyPipelineLayout(dev, layoutGeometria, nullptr);
    });

    pipeGBuffer = criarPipeline("gbuffer.vert", "gbuffer.frag", layoutGeometria, {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R16G16B16A16_SFLOAT}, true);
    pipeSSAO    = criarPipeline("tela.vert", "ssao.frag",  layoutTela, {VK_FORMAT_R8_UNORM}, false);
    pipeBlur    = criarPipeline("tela.vert", "blur.frag",  layoutTela, {VK_FORMAT_R8_UNORM}, false);
    pipeFinal   = criarPipeline("tela.vert", "final.frag", layoutTela, {formatoTela}, false);
}

// ============================ 6. CAMERA LIVRE E INTERFACE ============================

// Biblioteca Dear ImGui: desenha a janela de controles por cima da imagem final.
void iniciarInterface() {
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // nao grava arquivo de configuracao da interface
    ImGui_ImplGlfw_InitForVulkan(janela, true);
    ImGui_ImplVulkan_InitInfo ii{};
    ii.ApiVersion = VK_API_VERSION_1_3;
    ii.Instance = instancia;
    ii.PhysicalDevice = gpu;
    ii.Device = dev;
    ii.QueueFamily = familiaFila;
    ii.Queue = fila;
    ii.MinImageCount = 2;
    ii.ImageCount = (uint32_t)imagensTela.size();
    ii.DescriptorPoolSize = 8; // a propria ImGui cria seus descritores
    ii.UseDynamicRendering = true;
    ii.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1, .pColorAttachmentFormats = &formatoTela};
    ImGui_ImplVulkan_Init(&ii);
    limpezaGeral.push_back([] { ImGui_ImplVulkan_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); });
}

// Camera livre: segure o botao DIREITO do mouse para olhar; W A S D andam;
// Espaco sobe, Ctrl desce, Shift acelera, Esc sai. Ao final calcula as matrizes da camera.
void atualizarCamera(float dt) {
    static double mouseX = 0, mouseY = 0;
    double x, y;
    glfwGetCursorPos(janela, &x, &y);
    bool olhando = glfwGetMouseButton(janela, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    glfwSetInputMode(janela, GLFW_CURSOR, olhando ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL); // esconde o cursor ao olhar
    if (olhando) {
        camGiro       += float(x - mouseX) * 0.003f;
        camInclinacao  = glm::clamp(camInclinacao - float(y - mouseY) * 0.003f, -1.5f, 1.5f); // nao deixa virar de ponta-cabeca
    }
    mouseX = x; mouseY = y;

    glm::vec3 frente(cos(camInclinacao) * cos(camGiro), sin(camInclinacao), cos(camInclinacao) * sin(camGiro));
    glm::vec3 direita = glm::normalize(glm::cross(frente, glm::vec3(0, 1, 0)));
    auto tecla = [](int t) { return glfwGetKey(janela, t) == GLFW_PRESS; };
    float passo = camVelocidade * dt * (tecla(GLFW_KEY_LEFT_SHIFT) ? 5.0f : 1.0f);
    if (tecla(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(janela, GLFW_TRUE); // Esc fecha o programa
    if (!ImGui::GetIO().WantCaptureKeyboard) { // nao anda enquanto se digita na interface
        if (tecla(GLFW_KEY_W)) camPos += frente * passo;
        if (tecla(GLFW_KEY_S)) camPos -= frente * passo;
        if (tecla(GLFW_KEY_D)) camPos += direita * passo;
        if (tecla(GLFW_KEY_A)) camPos -= direita * passo;
        if (tecla(GLFW_KEY_SPACE))        camPos.y += passo;
        if (tecla(GLFW_KEY_LEFT_CONTROL)) camPos.y -= passo;
    }

    // Matriz de camera (mundo -> olho) e de projecao em perspectiva (olho -> tela).
    glm::mat4 camera   = glm::lookAt(camPos, camPos + frente, glm::vec3(0, 1, 0));
    glm::mat4 projecao = glm::perspective(glm::radians(60.0f), (float)tamanho.width / tamanho.height, 1.0f, 12000.0f);
    projecao[1][1] *= -1; // no Vulkan o eixo Y da tela aponta para baixo
    params.viewProj    = projecao * camera;
    params.invViewProj = glm::inverse(params.viewProj);
    params.posCamera   = glm::vec4(camPos, 1);
}

// Janela de controles: cada item altera um campo de "params", que os shaders leem.
void montarInterface() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({430, 0}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Oclusão de Ambiente (SSAO)");
    ImGui::Text("%.0f quadros por segundo", ImGui::GetIO().Framerate);
    ImGui::TextDisabled("Botão direito + mouse: olhar | WASD: andar\nEspaço/Ctrl: subir/descer | Shift: acelerar | Esc: sair");

    ImGui::SeparatorText("Oclusão");
    ImGui::CheckboxFlags("SSAO ligado", &params.ssaoLigado, 1);
    ImGui::SliderFloat("Raio", &params.raio, 1, 60, "%.1f");                 // alcance da busca (unidades de mundo)
    ImGui::SliderFloat("Intensidade", &params.intensidade, 0, 4, "%.2f");    // forca do escurecimento
    ImGui::SliderFloat("Contraste", &params.contraste, 0.5f, 4, "%.2f");     // expoente do resultado
    ImGui::SliderFloat("Bias", &params.bias, 0, 0.5f, "%.2f");               // tolerancia contra auto-oclusao
    ImGui::SliderInt("Amostras", &params.amostras, 4, 64);                   // qualidade x custo
    ImGui::CheckboxFlags("Suavizar (blur)", &params.blurLigado, 1);          // mostra o ruido quando desligado

    ImGui::SeparatorText("Visualização");
    ImGui::Combo("Modo", &params.modo, "Final\0Somente oclusão\0Argila (sem texturas)\0Normais\0Profundidade\0");
    ImGui::SliderFloat("Divisória", &params.divisoria, 0, 1, "%.2f");        // esquerda sem SSAO | direita com SSAO

    ImGui::SeparatorText("Luz");
    ImGui::SliderFloat("Luz ambiente", &params.luzAmbiente, 0, 2, "%.2f");
    ImGui::SliderFloat("Luz do sol", &params.luzSol, 0, 2, "%.2f");
    ImGui::SliderFloat("AO na luz direta", &params.aoNaLuzDireta, 0, 1, "%.2f"); // exagera o efeito para demonstrar

    ImGui::SeparatorText("Câmera e desempenho");
    ImGui::SliderFloat("Velocidade", &camVelocidade, 10, 1000, "%.0f");
    ImGui::SliderFloat("Alcance dos objetos", &distanciaObjetos, 200, 6000, "%.0f"); // menor = mais rapido
    ImGui::End();
    ImGui::Render();
}

// ============================ 7. DESENHO DE UM QUADRO ============================

// Inicia um passo de desenho nas imagens de cor informadas (e, opcionalmente, com
// profundidade), limpando-as antes. Tambem ajusta a area de desenho a janela inteira.
void iniciarPasso(std::vector<VkImageView> alvosCor, VkImageView alvoProfundidade = VK_NULL_HANDLE) {
    std::vector<VkRenderingAttachmentInfo> cores;
    for (auto vista : alvosCor)
        cores.push_back({.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, .imageView = vista, .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                         .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE});
    VkRenderingAttachmentInfo prof{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, .imageView = alvoProfundidade,
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.depthStencil = {1.0f, 0}}}; // 1.0 = o mais distante possivel
    VkRenderingInfo ri{.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, .renderArea = {{0, 0}, tamanho}, .layerCount = 1,
        .colorAttachmentCount = (uint32_t)cores.size(), .pColorAttachments = cores.data(),
        .pDepthAttachment = alvoProfundidade ? &prof : nullptr};
    vkCmdBeginRendering(cmd, &ri);
    VkViewport vp{0, 0, (float)tamanho.width, (float)tamanho.height, 0, 1};
    VkRect2D recorte{{0, 0}, tamanho};
    vkCmdSetViewport(cmd, 0, 1, &vp);
    vkCmdSetScissor(cmd, 0, 1, &recorte);
}

// Passo de tela cheia: roda o fragment shader do pipeline em todos os pixels de "alvo".
void passoDeTela(VkPipeline pipeline, VkImageView alvo) {
    iniciarPasso({alvo});
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layoutTela, 0, 1, &setGlobal, 0, nullptr);
    vkCmdDraw(cmd, 3, 1, 0, 0); // o triangulo unico de tela.vert
}

void desenharQuadro() {
    // Espera a GPU terminar o quadro anterior e pede a proxima imagem livre da janela.
    vkWaitForFences(dev, 1, &cercaQuadro, VK_TRUE, UINT64_MAX);
    uint32_t indice;
    VkResult r = vkAcquireNextImageKHR(dev, swapchain, UINT64_MAX, semImagemLivre, VK_NULL_HANDLE, &indice);
    if (r == VK_ERROR_OUT_OF_DATE_KHR) { criarTela(); ligarImagensDaTela(); return; } // janela mudou de tamanho
    vkResetFences(dev, 1, &cercaQuadro);

    memcpy(memParametros, &params, sizeof(params)); // envia os controles atuais aos shaders
    const VkImageLayout INDEFINIDO = VK_IMAGE_LAYOUT_UNDEFINED, ALVO = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, LEITURA = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    iniciarComandos();

    // ---- PASSO 1: G-buffer. Desenha toda a geometria guardando cor, normal e profundidade.
    transicao(gAlbedo.img, INDEFINIDO, ALVO);
    transicao(gNormal.img, INDEFINIDO, ALVO);
    transicao(gProfundidade.img, INDEFINIDO, ALVO, VK_IMAGE_ASPECT_DEPTH_BIT);
    iniciarPasso({gAlbedo.vista, gNormal.vista}, gProfundidade.vista);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeGBuffer);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layoutGeometria, 0, 1, &setGlobal, 0, nullptr);
    VkDeviceSize zero = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &bufVertices.buf, &zero);
    vkCmdBindIndexBuffer(cmd, bufIndices.buf, 0, VK_INDEX_TYPE_UINT32);
    for (const Desenho& d : desenhos) {
        struct { glm::mat4 modelo; int terreno; } obj{d.modelo, d.textura == 0}; // a textura 0 e a do chao
        // Objetos pequenos (plantas, pedras) muito longe da camera sao pulados para ganhar desempenho.
        if (!obj.terreno && glm::distance(camPos, glm::vec3(d.modelo[3])) > distanciaObjetos) continue;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layoutGeometria, 1, 1, &setsMaterial[d.textura], 0, nullptr);
        vkCmdPushConstants(cmd, layoutGeometria, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::mat4) + sizeof(int), &obj);
        vkCmdDrawIndexed(cmd, d.numIndices, 1, d.primeiroIndice, 0, 0);
    }
    vkCmdEndRendering(cmd);
    transicao(gAlbedo.img, ALVO, LEITURA);
    transicao(gNormal.img, ALVO, LEITURA);
    transicao(gProfundidade.img, ALVO, LEITURA, VK_IMAGE_ASPECT_DEPTH_BIT);

    // ---- PASSO 2: SSAO. Le normal e profundidade, escreve a oclusao bruta.
    transicao(aoBruto.img, INDEFINIDO, ALVO);
    passoDeTela(pipeSSAO, aoBruto.vista);
    vkCmdEndRendering(cmd);
    transicao(aoBruto.img, ALVO, LEITURA);

    // ---- PASSO 3: Blur. Le a oclusao bruta, escreve a oclusao suavizada.
    transicao(aoSuave.img, INDEFINIDO, ALVO);
    passoDeTela(pipeBlur, aoSuave.vista);
    vkCmdEndRendering(cmd);
    transicao(aoSuave.img, ALVO, LEITURA);

    // ---- PASSO 4: Final. Ilumina usando a oclusao e desenha a interface, direto na janela.
    transicao(imagensTela[indice], INDEFINIDO, ALVO);
    passoDeTela(pipeFinal, vistasTela[indice]);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRendering(cmd);
    transicao(imagensTela[indice], ALVO, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Envia os comandos para a GPU e manda a imagem pronta para a janela.
    vkEndCommandBuffer(cmd);
    VkPipelineStageFlags esperaEm = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo si{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .waitSemaphoreCount = 1, .pWaitSemaphores = &semImagemLivre,
        .pWaitDstStageMask = &esperaEm, .commandBufferCount = 1, .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1, .pSignalSemaphores = &semDesenhoPronto};
    conferir(vkQueueSubmit(fila, 1, &si, cercaQuadro), "enviar quadro");
    VkPresentInfoKHR pi{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, .waitSemaphoreCount = 1, .pWaitSemaphores = &semDesenhoPronto,
        .swapchainCount = 1, .pSwapchains = &swapchain, .pImageIndices = &indice};
    r = vkQueuePresentKHR(fila, &pi);
    if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR) { criarTela(); ligarImagensDaTela(); }
}

int main(int argc, char** argv) {
    iniciarVulkan();
    criarTela();
    carregarMapa(argc > 1 ? argv[1] : ARQUIVO_MAPA); // outro .glb pode ser passado como argumento
    criarDescritores();
    criarPipelines();
    iniciarInterface();

    // Laco principal: le teclado/mouse, atualiza camera e interface, desenha um quadro.
    double anterior = glfwGetTime();
    while (!glfwWindowShouldClose(janela)) {
        glfwPollEvents();
        double agora = glfwGetTime();
        montarInterface();
        atualizarCamera(float(agora - anterior));
        anterior = agora;
        desenharQuadro();
    }

    // Encerramento: espera a GPU e destroi tudo na ordem inversa da criacao.
    vkDeviceWaitIdle(dev);
    executar(limpezaGeral);
    executar(limpezaTela);
    vkDestroyDevice(dev, nullptr);
    vkDestroySurfaceKHR(instancia, superficie, nullptr);
    vkDestroyInstance(instancia, nullptr);
    glfwDestroyWindow(janela);
    glfwTerminate();
}
