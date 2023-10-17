#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <shader_m.h>
#include <camera.h>
#include <model.h>

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
unsigned int sphereTexCoordsFBO, sphereTexCoordsRBO;
unsigned int sphereTexCoordsMap;
std::vector<GLfloat> g_sphereTexCoordDatas;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderSphere();
void renderCube();
void renderQuad();
void renderPoint();
unsigned int pointVAO = 0;
unsigned int pointVBO = 0;
vector<glm::vec3> pointVertice = {
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec3(0.0f,  1.0f, 0.0f)
        };
void renderLine();

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
unsigned int ImGui_Width = 500;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	
float lastFrame = 0.0f;

// PBR texture
unsigned int envCubemap = 0;
unsigned int irradianceMap = 0;
unsigned int prefilterMap = 0;
unsigned int brdfLUTTexture = 0;
std::string hdrFileNameCurrent;
void processPBR(unsigned int captureFBO, unsigned int captureRBO, const char *hdrFileName);
std::vector<std::string> getHdrFileNames(std::string path);

// imGui Params
bool g_bCaptureCursor = true;
bool g_bIblDiffuse = true;
bool g_bIblSpecular = true;
float g_irradianceConst = 0.03f;
int g_lightNumber = 4;
float g_miplevel = 0.0f;
bool g_bFixMiplevel = false;
int g_pointVisualNum = 8;
float g_albedoScale = 1.0f;
float g_sphereTexCoords[2] = {0.0f, 0.0f};
float g_sphereTexCoordsCurrent[2] = {0.0f, 0.0f};
bool g_bShowImportanceSamples = true;
enum BackgroundOptions
{
    BackgroundOptions_None,
    BackgroundOptions_Env,
    BackgroundOptions_Irradiance,
    BackgroundOptions_Prefilter
};
int g_backgoundMode = BackgroundOptions_Env;
int g_hdrFileIndex = 0;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH + ImGui_Width, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    // set depth function to less than AND equal for skybox depth trick.
    glDepthFunc(GL_LEQUAL);
    // enable seamless cubemap sampling for lower mip levels in the pre-filter map.
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // build and compile shaders
    // -------------------------
    Shader pbrShader(VERTEX_PBR_FILE, FRAGMENT_PBR_FILE);
    Shader pbrGunShader(VERTEX_PBR_FILE, FRAGMENT_PBR_GUN_FILE);
    Shader pbrWineShader(VERTEX_PBR_FILE, FRAGMENT_PBR_WINE_FILE);

    Shader backgroundShader(VERTEX_BACKGROUND_FILE, FRAGMENT_BACKGROUND_FILE);
    Shader importanceSampleVisualShader(VERTEX_IMPORTANCE_VISUAL_FILE, FRAGMENT_IMPORTANCE_VISUAL_FILE, GEOMETRY_IMPORTANCE_VISUAL_FILE);
    Shader importanceMapShader(VERTEX_IMPORTANCE_LINE_FILE, FRAGMENT_IMPORTANCE_LINE_FILE);
    Shader importanceLineSampleShader(VERTEX_IMPORTANCE_LINE_FILE, FRAGMENT_IMPORTANCE_LINE_SAMPLE_FILE);
    Shader sphereTexcoordsShader(VERTEX_SPHERE_TEXCOORDS_FILE, FRAGMENT_SPHERE_TEXCOORDS_FILE);

    Model gun(RESOURCES_DIR"/textures/pbr/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
    Model wine(RESOURCES_DIR"/textures/pbr/wine_bottles_01_4k.gltf/wine_bottles_01_4k.gltf");

    pbrShader.use();
    pbrShader.setInt("irradianceMap", 0);
    pbrShader.setInt("prefilterMap", 1);
    pbrShader.setInt("brdfLUT", 2);
    pbrShader.setInt("albedoMap", 3);
    pbrShader.setInt("normalMap", 4);
    pbrShader.setInt("metallicMap", 5);
    pbrShader.setInt("roughnessMap", 6);
    pbrShader.setInt("aoMap", 7);
    pbrGunShader.use();
    pbrGunShader.setInt("irradianceMap", 3);//纹理ID从0改为3
    pbrGunShader.setInt("prefilterMap", 1);
    pbrGunShader.setInt("brdfLUT", 2);
    //pbrGunShader.setInt("albedoMap", 3);
    pbrGunShader.setInt("normalMap", 4);
    pbrGunShader.setInt("metallicMap", 5);
    pbrGunShader.setInt("roughnessMap", 6);
    pbrGunShader.setInt("aoMap", 7);
    pbrWineShader.use();
    pbrWineShader.setInt("irradianceMap", 6);//前面0~5被Model占用了
    pbrWineShader.setInt("prefilterMap", 7);
    pbrWineShader.setInt("brdfLUT", 8);

    backgroundShader.use();
    backgroundShader.setInt("environmentMap", 0);

    importanceMapShader.use();
    importanceMapShader.setInt("roughnessMap", 6);

    // load PBR material textures
    // --------------------------
    // rusted iron
    unsigned int ironAlbedoMap = loadTexture(RESOURCES_DIR"/textures/pbr/rusted_iron/albedo.png");
    unsigned int ironNormalMap = loadTexture(RESOURCES_DIR"/textures/pbr/rusted_iron/normal.png");
    unsigned int ironMetallicMap = loadTexture(RESOURCES_DIR"/textures/pbr/rusted_iron/metallic.png");
    unsigned int ironRoughnessMap = loadTexture(RESOURCES_DIR"/textures/pbr/rusted_iron/roughness.png");
    unsigned int ironAOMap = loadTexture(RESOURCES_DIR"/textures/pbr/rusted_iron/ao.png");

    // gold
    unsigned int goldAlbedoMap = loadTexture(RESOURCES_DIR"/textures/pbr/gold/albedo.png");
    unsigned int goldNormalMap = loadTexture(RESOURCES_DIR"/textures/pbr/gold/normal.png");
    unsigned int goldMetallicMap = loadTexture(RESOURCES_DIR"/textures/pbr/gold/metallic.png");
    unsigned int goldRoughnessMap = loadTexture(RESOURCES_DIR"/textures/pbr/gold/roughness.png");
    unsigned int goldAOMap = loadTexture(RESOURCES_DIR"/textures/pbr/gold/ao.png");

    // grass
    unsigned int grassAlbedoMap = loadTexture(RESOURCES_DIR"/textures/pbr/grass/albedo.png");
    unsigned int grassNormalMap = loadTexture(RESOURCES_DIR"/textures/pbr/grass/normal.png");
    unsigned int grassMetallicMap = loadTexture(RESOURCES_DIR"/textures/pbr/grass/metallic.png");
    unsigned int grassRoughnessMap = loadTexture(RESOURCES_DIR"/textures/pbr/grass/roughness.png");
    unsigned int grassAOMap = loadTexture(RESOURCES_DIR"/textures/pbr/grass/ao.png");

    // plastic
    unsigned int plasticAlbedoMap = loadTexture(RESOURCES_DIR"/textures/pbr/plastic/albedo.png");
    unsigned int plasticNormalMap = loadTexture(RESOURCES_DIR"/textures/pbr/plastic/normal.png");
    unsigned int plasticMetallicMap = loadTexture(RESOURCES_DIR"/textures/pbr/plastic/metallic.png");
    unsigned int plasticRoughnessMap = loadTexture(RESOURCES_DIR"/textures/pbr/plastic/roughness.png");
    unsigned int plasticAOMap = loadTexture(RESOURCES_DIR"/textures/pbr/plastic/ao.png");

    // wall
    unsigned int wallAlbedoMap = loadTexture(RESOURCES_DIR"/textures/pbr/wall/albedo.png");
    unsigned int wallNormalMap = loadTexture(RESOURCES_DIR"/textures/pbr/wall/normal.png");
    unsigned int wallMetallicMap = loadTexture(RESOURCES_DIR"/textures/pbr/wall/metallic.png");
    unsigned int wallRoughnessMap = loadTexture(RESOURCES_DIR"/textures/pbr/wall/roughness.png");
    unsigned int wallAOMap = loadTexture(RESOURCES_DIR"/textures/pbr/wall/ao.png");

    // gun
    unsigned int gunNormalMap = loadTexture(RESOURCES_DIR"/textures/pbr/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga");
    unsigned int gunMetallicMap = loadTexture(RESOURCES_DIR"/textures/pbr/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga");
    unsigned int gunRoughnessMap = loadTexture(RESOURCES_DIR"/textures/pbr/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga");
    unsigned int gunAOMap = loadTexture(RESOURCES_DIR"/textures/pbr/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga");

    // lights
    // ------
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    // pbr: setup framebuffer
    // ----------------------
    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // 新增一个帧缓冲，用于渲染重要性采样纹理贴图
    unsigned int importanceSampleFBO;
    glGenFramebuffers(1, &importanceSampleFBO);
    // importance sample texture
    unsigned int importanceSampleMap;
    glGenTextures(1, &importanceSampleMap);

    glBindTexture(GL_TEXTURE_2D, importanceSampleMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1024, 1, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, importanceSampleFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, importanceSampleMap, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "importanceSample Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 新增一个帧缓冲，用于渲染球体纹理坐标(x, y)纹理贴图
    glGenFramebuffers(1, &sphereTexCoordsFBO);
    glGenRenderbuffers(1, &sphereTexCoordsRBO);//需要用到深度测试

    glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, sphereTexCoordsRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, sphereTexCoordsRBO);
    // sphereTexCoords texture
    glGenTextures(1, &sphereTexCoordsMap);

    glBindTexture(GL_TEXTURE_2D, sphereTexCoordsMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sphereTexCoordsMap, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "sphereTexCoords Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    pbrShader.use();
    pbrShader.setMat4("projection", projection);
    backgroundShader.use();
    backgroundShader.setMat4("projection", projection);
    importanceSampleVisualShader.use();
    importanceSampleVisualShader.setMat4("projection", projection);
    importanceSampleVisualShader.setInt("importanceSampleMap", 8);
    importanceLineSampleShader.use();
    importanceLineSampleShader.setInt("importanceSampleMap", 8);

    // then before rendering, configure the viewport to the original framebuffer's screen dimensions
    int scrWidth, scrHeight;
    glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
    glViewport(0, 0, scrWidth, scrHeight);

    std::vector<std::string> hdrFiles = getHdrFileNames(RESOURCES_DIR"/textures/hdr/");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        if (memcmp(g_sphereTexCoordsCurrent, g_sphereTexCoords, 2 * sizeof(float)) != 0)
        {
            memcpy(g_sphereTexCoordsCurrent, g_sphereTexCoords, 2 * sizeof(float));
            float xSegment = g_sphereTexCoordsCurrent[0];
            float ySegment = g_sphereTexCoordsCurrent[1];
            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            float yPos = std::cos(ySegment * glm::pi<float>());
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            glm::vec3 spherePos = glm::vec3(xPos, yPos, zPos);
            pointVertice.clear();
            for (size_t i = 0; i < g_pointVisualNum; i++)
            {
                pointVertice.push_back(spherePos);
                pointVertice.push_back(spherePos);
            }
            glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pointVertice.size(), pointVertice.data(), GL_STATIC_DRAW);
        }
        if (g_hdrFileIndex < hdrFiles.size() && hdrFileNameCurrent != hdrFiles[g_hdrFileIndex])
        {
            hdrFileNameCurrent = hdrFiles[g_hdrFileIndex];
            std::string hdrfile = RESOURCES_DIR"/textures/hdr/" + hdrFileNameCurrent;
            processPBR(captureFBO, captureRBO, hdrfile.c_str());
        }
        

        // render
        // ------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene, supplying the convoluted irradiance map to the final shader.
        // ------------------------------------------------------------------------------------------
        pbrShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        pbrShader.setMat4("projection", projection);
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("camPos", camera.Position);
        pbrShader.setBool("enableIblDiffuse", g_bIblDiffuse);
        pbrShader.setFloat("irradianceConst", g_irradianceConst);
        pbrShader.setBool("enableIblSpecular", g_bIblSpecular);
        pbrShader.setInt("lightNumber", g_lightNumber);
        pbrShader.setBool("fixMiplevel", g_bFixMiplevel);
        pbrShader.setFloat("miplevel", g_miplevel);
        pbrShader.setFloat("albedoScale", g_albedoScale);
        importanceSampleVisualShader.use();
        importanceSampleVisualShader.setMat4("projection", projection);
        importanceSampleVisualShader.setMat4("view", view);
        sphereTexcoordsShader.use();
        sphereTexcoordsShader.setMat4("projection", projection);
        sphereTexcoordsShader.setMat4("view", view);
        pbrGunShader.use();
        pbrGunShader.setMat4("projection", projection);
        pbrGunShader.setMat4("view", view);
        pbrGunShader.setVec3("camPos", camera.Position);
        pbrGunShader.setBool("enableIblDiffuse", g_bIblDiffuse);
        pbrGunShader.setFloat("irradianceConst", g_irradianceConst);
        pbrGunShader.setBool("enableIblSpecular", g_bIblSpecular);
        pbrGunShader.setInt("lightNumber", g_lightNumber);
        pbrGunShader.setBool("fixMiplevel", g_bFixMiplevel);
        pbrGunShader.setFloat("miplevel", g_miplevel);
        pbrGunShader.setFloat("albedoScale", g_albedoScale);
        pbrWineShader.use();
        pbrWineShader.setMat4("projection", projection);
        pbrWineShader.setMat4("view", view);
        pbrWineShader.setVec3("camPos", camera.Position);
        pbrWineShader.setBool("enableIblDiffuse", g_bIblDiffuse);
        pbrWineShader.setFloat("irradianceConst", g_irradianceConst);
        pbrWineShader.setBool("enableIblSpecular", g_bIblSpecular);
        pbrWineShader.setInt("lightNumber", g_lightNumber);
        pbrWineShader.setBool("fixMiplevel", g_bFixMiplevel);
        pbrWineShader.setFloat("miplevel", g_miplevel);
        pbrWineShader.setFloat("albedoScale", g_albedoScale);
        pbrShader.use();

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // rusted iron
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ironNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, ironAOMap);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, importanceSampleMap);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-5.0, 0.0, -2.0));
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        renderSphere();

        glBindFramebuffer(GL_FRAMEBUFFER, importanceSampleFBO);
            glViewport(0, 0, 1024, 1);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            importanceMapShader.use();
            importanceMapShader.setVec2("sphereTexCoords", glm::vec2(g_sphereTexCoordsCurrent[0], g_sphereTexCoordsCurrent[1]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, importanceSampleMap, 0);
            renderLine();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        importanceSampleVisualShader.use();
        importanceSampleVisualShader.setMat4("model", model);
        if (g_bShowImportanceSamples)
            renderPoint();

        /*for (int i = 0; i < 200; i++)
        {
            glViewport(0, i, 1024, 1);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            importanceMapShader.use();
            importanceMapShader.setVec2("sphereTexCoords", glm::vec2(g_sphereTexCoordsCurrent[0], g_sphereTexCoordsCurrent[1]));
            renderLine();
        }*/
        /*glViewport(0, 0, 1024, 100);
        importanceLineSampleShader.use();
        renderQuad();
        glViewport(0, 100, 1024, 100);
        importanceMapShader.use();
        renderQuad();*/
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            sphereTexcoordsShader.use();
            sphereTexcoordsShader.setMat4("model", model);
            renderSphere();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // gold
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, goldNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, goldAOMap);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0, 0.0, -2.0));
        pbrShader.use();
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        renderSphere();

        glBindFramebuffer(GL_FRAMEBUFFER, importanceSampleFBO);
            glViewport(0, 0, 1024, 1);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            importanceMapShader.use();
            importanceMapShader.setVec2("sphereTexCoords", glm::vec2(g_sphereTexCoordsCurrent[0], g_sphereTexCoordsCurrent[1]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, importanceSampleMap, 0);
            renderLine();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        importanceSampleVisualShader.use();
        importanceSampleVisualShader.setMat4("model", model);
        if (g_bShowImportanceSamples)
            renderPoint();

        glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            sphereTexcoordsShader.use();
            sphereTexcoordsShader.setMat4("model", model);
            renderSphere();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // grass
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, grassNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, grassAOMap);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0, 0.0, -2.0));
        pbrShader.use();
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        renderSphere();

        glBindFramebuffer(GL_FRAMEBUFFER, importanceSampleFBO);
            glViewport(0, 0, 1024, 1);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            importanceMapShader.use();
            importanceMapShader.setVec2("sphereTexCoords", glm::vec2(g_sphereTexCoordsCurrent[0], g_sphereTexCoordsCurrent[1]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, importanceSampleMap, 0);
            renderLine();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        importanceSampleVisualShader.use();
        importanceSampleVisualShader.setMat4("model", model);
        if (g_bShowImportanceSamples)
            renderPoint();

        glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            sphereTexcoordsShader.use();
            sphereTexcoordsShader.setMat4("model", model);
            renderSphere();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // plastic
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, plasticAOMap);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0, 0.0, -2.0));
        pbrShader.use();
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        renderSphere();

        glBindFramebuffer(GL_FRAMEBUFFER, importanceSampleFBO);
            glViewport(0, 0, 1024, 1);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            importanceMapShader.use();
            importanceMapShader.setVec2("sphereTexCoords", glm::vec2(g_sphereTexCoordsCurrent[0], g_sphereTexCoordsCurrent[1]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, importanceSampleMap, 0);
            renderLine();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        importanceSampleVisualShader.use();
        importanceSampleVisualShader.setMat4("model", model);
        if (g_bShowImportanceSamples)
            renderPoint();

        glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            sphereTexcoordsShader.use();
            sphereTexcoordsShader.setMat4("model", model);
            renderSphere();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // wall
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, wallNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, wallAOMap);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0, 0.0, -2.0));
        pbrShader.use();
        pbrShader.setMat4("model", model);
        pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        renderSphere();

        glBindFramebuffer(GL_FRAMEBUFFER, importanceSampleFBO);
            glViewport(0, 0, 1024, 1);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            importanceMapShader.use();
            importanceMapShader.setVec2("sphereTexCoords", glm::vec2(g_sphereTexCoordsCurrent[0], g_sphereTexCoordsCurrent[1]));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, importanceSampleMap, 0);
            renderLine();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        importanceSampleVisualShader.use();
        importanceSampleVisualShader.setMat4("model", model);
        if (g_bShowImportanceSamples)
            renderPoint();

        glBindFramebuffer(GL_FRAMEBUFFER, sphereTexCoordsFBO);
            //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            sphereTexcoordsShader.use();
            sphereTexcoordsShader.setMat4("model", model);
            renderSphere();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // render light source (simply re-render sphere at light positions)
        // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
        // keeps the codeprint small.
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]) && i < g_lightNumber; ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            pbrShader.use();
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.setMat4("model", model);
            pbrShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere();

            pbrGunShader.use();
            pbrGunShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrGunShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
            pbrWineShader.use();
            pbrWineShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrWineShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }

        // render gun mdoel
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-3.0, 2.2, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.02f));
        pbrGunShader.use();
        pbrGunShader.setMat4("model", model);
        pbrGunShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, gunNormalMap);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gunMetallicMap);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, gunRoughnessMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, gunAOMap);
        gun.Draw(pbrGunShader.ID);

        // render wine mdoel
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0, 1.5, 0.0));
        model = glm::scale(model, glm::vec3(10.0f));
        pbrWineShader.use();
        pbrWineShader.setMat4("model", model);
        pbrWineShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        wine.Draw(pbrWineShader.ID);

        // render skybox (render as last to prevent overdraw)
        backgroundShader.use();

        backgroundShader.setMat4("view", view);
        backgroundShader.setFloat("miplevel", g_miplevel);
        glActiveTexture(GL_TEXTURE0);
        if (g_backgoundMode == BackgroundOptions_Env)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            renderCube();
        }
        else if (g_backgoundMode == BackgroundOptions_Irradiance)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
            renderCube();
        }
        else if (g_backgoundMode == BackgroundOptions_Prefilter)
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
            renderCube();
        }

        // render BRDF map to screen
        //brdfShader.Use();
        //renderQuad();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Param Setting");
        ImGui::Text("Press \"1\" to show cursor and switch to setting mode.");
        ImGui::Text("Press \"2\" to hide cursor and finishe setting mode.");
        ImGui::Text("Current cursor mode: %d", g_bCaptureCursor ? 2 : 1);
        if (ImGui::CollapsingHeader("HDR files Options", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (size_t i = 0; i < hdrFiles.size(); i++)
                ImGui::RadioButton(hdrFiles[i].c_str(), &g_hdrFileIndex, i);
            if (ImGui::Button("Update HDR file list"))
            {
                hdrFiles = getHdrFileNames(RESOURCES_DIR"/textures/hdr/");
                auto it = std::find(hdrFiles.begin(), hdrFiles.end(), hdrFileNameCurrent);
                if (it != hdrFiles.end()) {
                    // 找到了目标字符串，it 指向了该字符串的位置
                    g_hdrFileIndex = int(std::distance(hdrFiles.begin(), it));
                    std::cout << "当前选中的HDR文件编号： " << g_hdrFileIndex << std::endl;
                } else {
                    std::cout << "在当前文件列表中找不到文件： " << hdrFileNameCurrent << "， 将会默认选中第一个HDR文件" << std::endl;
                    g_hdrFileIndex = 0;
                }
            }
        }
        if (ImGui::CollapsingHeader("Light setting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("albedo scale", &g_albedoScale, 0.0f, 1.0f);
            ImGui::SliderInt("light number", &g_lightNumber, 0, 4);
            ImGui::Checkbox("IBL diffuse", &g_bIblDiffuse);
            if (!g_bIblDiffuse)
                ImGui::SliderFloat("const irradiance", &g_irradianceConst, 0.0f, 0.2f, "%.2f");
            ImGui::Checkbox("IBL specular", &g_bIblSpecular);
            if (g_bIblSpecular)
                ImGui::Checkbox("IBL fix miplevel from prefilter map", &g_bFixMiplevel);
            ImGui::RadioButton("Skybox: None", &g_backgoundMode, BackgroundOptions_None);
            ImGui::RadioButton("Skybox: envCubemap", &g_backgoundMode, BackgroundOptions_Env);
            ImGui::RadioButton("Skybox: irradianceMap", &g_backgoundMode, BackgroundOptions_Irradiance);
            ImGui::RadioButton("Skybox: prefilterMap", &g_backgoundMode, BackgroundOptions_Prefilter);
            ImGui::SliderFloat("Enviroment miplevel", &g_miplevel, 0.0f, 4.0, "%.1f");
        }
        if (ImGui::CollapsingHeader("Importance sample visual setting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Show importance samples", &g_bShowImportanceSamples);
            if (g_bShowImportanceSamples)
            {
                ImGui::SliderInt("point visual number", &g_pointVisualNum, 1, 8);
                ImGui::SliderFloat("Sphere tex x-coords", g_sphereTexCoords, 0.0f, 1.0f);
                ImGui::SliderFloat("Sphere tex y-coords", g_sphereTexCoords + 1, 0.0f, 1.0f);
            }
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        if (g_bCaptureCursor)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            g_bCaptureCursor = false;
        }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        if (!g_bCaptureCursor)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            g_bCaptureCursor = true;
            firstMouse = true;
        }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    SCR_WIDTH = width - ImGui_Width;
    SCR_HEIGHT = height;
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!g_bCaptureCursor)
        return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            float viewportX = x;
            float viewportY = SCR_HEIGHT - y;
            if (viewportX <= SCR_WIDTH && viewportY <= SCR_HEIGHT)
            {
                printf("Mouse clicked at windows (%f, %f)\n", x, y);
                printf("Mouse clicked at glViewport (%f, %f)\n", viewportX, viewportY);
                unsigned int bufferX = viewportX;
                unsigned int bufferY = viewportY;
                printf("Mouse clicked at sphereTexCoords-Buffer (%d, %d)\n", bufferX, bufferY);
                //gPosition
                GLint width, height;
                glBindTexture(GL_TEXTURE_2D, sphereTexCoordsMap);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);  
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
                //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_DEPTH, &depth);
                std::cout << "width: " << width << ", height: " << height << ", sizeof(GLfloat): " << sizeof(GLfloat) << std::endl;
                g_sphereTexCoordDatas.resize(width * height * 3);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, g_sphereTexCoordDatas.data());
                unsigned int dataIndex = (bufferX + bufferY * width) * 3;
                g_sphereTexCoords[0] = g_sphereTexCoordDatas[dataIndex];
                g_sphereTexCoords[1] = g_sphereTexCoordDatas[dataIndex + 1];
            }
        }
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
 
// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
GLsizei indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<GLsizei>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void renderPoint()
{
    if (pointVAO == 0)
    {
        glGenVertexArrays(1, &pointVAO);
        glGenBuffers(1, &pointVBO);
        glBindVertexArray(pointVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * pointVertice.size(), pointVertice.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)(sizeof(glm::vec3)));
    }
    glBindVertexArray(pointVAO);
    glDrawArrays(GL_POINTS, 0, g_pointVisualNum);
    glBindVertexArray(0);
}

unsigned int lineVAO = 0;
unsigned int lineVBO = 0;
void renderLine()
{
    if (lineVAO == 0)
    {
        float lineVertices[] = {
            -1.0f, 0.0f, 0.0f,
             1.0f, 0.0f, 0.0f
        };
        glGenVertexArrays(1, &lineVAO);
        glGenBuffers(1, &lineVBO);
        glBindVertexArray(lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), &lineVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINE_STRIP, 0, 2);
    glBindVertexArray(0);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// PBR process
void processPBR(unsigned int captureFBO, unsigned int captureRBO, const char *hdrFileName)
{
    Shader equirectangularToCubemapShader(VERTEX_CUBE_FILE, FRAGMENT_EQUIRE_CUBE_FILE);
    Shader irradianceShader(VERTEX_CUBE_FILE, FRAGMENT_IRRADIANCE_CUBE_FILE);
    Shader prefilterShader(VERTEX_CUBE_FILE, FRAGMENT_PREFILTER_BACKGROUND_FILE);
    Shader brdfShader(VERTEX_BRDF_FILE, FRAGMENT_BRDF_FILE);
    // pbr: load the HDR environment map
    // ---------------------------------
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float *data = stbi_loadf(hdrFileName, &width, &height, &nrComponents, 0);
    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    // pbr: setup cubemap to render to and attach to framebuffer
    // ---------------------------------------------------------
    if (envCubemap == 0)
        glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    equirectangularToCubemapShader.use();
    equirectangularToCubemapShader.setInt("equirectangularMap", 0);
    equirectangularToCubemapShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
    // --------------------------------------------------------------------------------
    if (irradianceMap == 0)
        glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
    // -----------------------------------------------------------------------------
    irradianceShader.use();
    irradianceShader.setInt("environmentMap", 0);
    irradianceShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
    // --------------------------------------------------------------------------------
    if (prefilterMap == 0)
        glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    // ----------------------------------------------------------------------------------------------------
    prefilterShader.use();
    prefilterShader.setInt("environmentMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // pbr: generate a 2D LUT from the BRDF equations used.
    // ----------------------------------------------------
    if (brdfLUTTexture == 0)
        glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "captureFBO Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<std::string> getHdrFileNames(std::string path)
{
    std::vector<std::string> hdrFiles;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() == ".hdr") {
            hdrFiles.push_back(entry.path().filename().string());
        }
    }

    std::cout << "HDR files in " << path << ":\n";
    for (const auto& hdrFile : hdrFiles) {
        std::cout << hdrFile << "\n";
    }
    return hdrFiles;
}