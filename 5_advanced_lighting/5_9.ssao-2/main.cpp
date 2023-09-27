#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>
#include <model.h>

#include <iostream>
#include <random>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path, bool gammaCorrection);
void renderQuad();
void renderCube();

// settings
unsigned int SCR_WIDTH = 1600;
unsigned int SCR_HEIGHT = 1200;

unsigned int BUFFER_WIDTH = 3200;
unsigned int BUFFER_HEIGHT = 2400;

unsigned int ImGui_Width = 500;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float far_plane = 150.0f;

// lighting info
vector<glm::vec3> lightsPos;// = glm::vec3(2.0, 4.0, -2.0);
glm::vec3 lightColor = glm::vec3(0.069, 0.069, 0.069);
float lightColorArray[3];

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}

// g-buffer
unsigned int gPosition, gNormal, gAlbedo;
std::vector<GLfloat> g_positionDatas;
std::vector<GLfloat> g_normalDatas;

//imGui Param
bool g_bCaptureCursor = true;
float g_textureScale = 0.80f;
float g_bufferScreenScale = 2.0;
float g_bufferScreenScaleCurrent = 2.0;
int g_kernelSize = 64;
int g_kernelSizeCurrent = 64;
float g_radius = 0.5f;
float g_bias = 0.025f;
float linear    = 0.015f;
float quadratic = 0.015f;
bool g_showNormal = false;
bool g_lightSetting = false;
float g_normalOffsetScale = 1.0f;
float g_ambientRatio = 0.3f;
// person param
float g_personPos[3] = {0.0f, -10.053f, 0.0f};
float g_personRotateVec[3] = {0.0f, 1.0f, 0.0f};
float g_personRotateAngle = 90.0f;
float g_personScale = 1.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH + ImGui_Width, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetWindowSizeLimits(window, ImGui_Width + 200, 200, 4000, 4000);
    glfwMakeContextCurrent(window);
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
    glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader shaderGeometryPass(VERTEX_SSAO_GEOMETRY_FILE, FRAGMENT_SSAO_GEOMETRY_FILE);
    Shader shaderLightingPass(VERTEX_SSAO_FILE, FRAGMENT_SSAO_LIGHTING_FILE);
    Shader shaderSSAO(VERTEX_SSAO_FILE, FRAGMENT_SSAO_FILE);
    Shader shaderSSAOBlur(VERTEX_SSAO_FILE, FRAGMENT_SSAO_BLUR_FILE);
    Shader shaderLightBox(VERTEX_LIGHT_BOX_FILE, FRAGMENT_LIGHT_BOX_FILE);
    Shader normalShader(VERTEX_NORMAL_FILE, FRAGMENT_NORMAL_FILE, GEOMETRY_FILE);

    // load models
    // -----------
    Model backpack(RESOURCES_DIR"/objects/backpack/backpack.obj");
    Model person(RESOURCES_DIR"/objects/babala/babala.pmx");
    Model room(RESOURCES_DIR"/objects/room37/Stage0514.pmx");

    // configure g-buffer framebuffer
    // ------------------------------
    BUFFER_WIDTH = SCR_WIDTH * g_bufferScreenScaleCurrent;
    BUFFER_HEIGHT = SCR_HEIGHT * g_bufferScreenScaleCurrent;
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, BUFFER_WIDTH, BUFFER_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // also create framebuffer to hold SSAO processing stage 
    // -----------------------------------------------------
    unsigned int ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // generate sample kernel
    // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < g_kernelSizeCurrent; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / (float)g_kernelSizeCurrent;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // generate noise texture
    // ----------------------
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture; 
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    lightColorArray[0] = lightColor.r;
    lightColorArray[1] = lightColor.g;
    lightColorArray[2] = lightColor.b;
    lightsPos.push_back(glm::vec3(-7.045096, -4.860858, -13.793019));
    lightsPos.push_back(glm::vec3(-5.553995, -4.988711, -15.074287));
    lightsPos.push_back(glm::vec3(-6.762054, -4.990257, -16.520481));
    lightsPos.push_back(glm::vec3(-8.298204, -4.890038, -15.289470));
    lightsPos.push_back(glm::vec3(-6.925618, -3.362771, -15.212321));
    lightsPos.push_back(glm::vec3(15.676228, 8.019284, -34.981667));
    lightsPos.push_back(glm::vec3(13.933311, 7.707386, -36.053696));
    lightsPos.push_back(glm::vec3(10.224646, 10.475569, -37.603985));
    lightsPos.push_back(glm::vec3(12.429892, 10.345181, -37.679760));
    lightsPos.push_back(glm::vec3(16.988745, 8.100528, -35.378098));
    lightsPos.push_back(glm::vec3(7.207140, 6.926700, -38.543453));
    lightsPos.push_back(glm::vec3(6.349365, 8.974488, -37.632160));
    lightsPos.push_back(glm::vec3(9.195130, 7.147511, -37.787212));
    lightsPos.push_back(glm::vec3(4.243202, 11.341250, -37.603897));
    lightsPos.push_back(glm::vec3(2.685416, 11.536051, -37.842297));
    lightsPos.push_back(glm::vec3(-5.515371, 9.305811, -37.784550));
    lightsPos.push_back(glm::vec3(-7.527062, 10.242709, -37.699825));
    lightsPos.push_back(glm::vec3(-11.973897, 11.045294, -37.884388));
    lightsPos.push_back(glm::vec3(-13.197551, 9.489014, -36.871658));
    lightsPos.push_back(glm::vec3(-9.013243, 5.775447, -37.733181));
    lightsPos.push_back(glm::vec3(-7.171374, 5.786111, -38.560143));
    lightsPos.push_back(glm::vec3(-14.290340, 4.172616, -33.761803));
    lightsPos.push_back(glm::vec3(-14.812913, 4.721153, -35.966843));
    lightsPos.push_back(glm::vec3(-12.877807, 2.631161, -36.812515));
    lightsPos.push_back(glm::vec3(-22.450239, 6.394677, -29.338604));
    lightsPos.push_back(glm::vec3(-23.504669, 4.839966, -25.702143));
    lightsPos.push_back(glm::vec3(-28.993561, 5.641595, -16.833120));
    lightsPos.push_back(glm::vec3(-28.047653, 4.639610, -18.843315));
    lightsPos.push_back(glm::vec3(-30.360165, 7.571958, -14.239196));
    lightsPos.push_back(glm::vec3(-30.479036, 10.383608, -13.966002));
    lightsPos.push_back(glm::vec3(-31.647856, 13.104560, -13.326150));
    lightsPos.push_back(glm::vec3(-33.382217, 12.042684, -10.655349));
    lightsPos.push_back(glm::vec3(-32.793568, 12.787537, -6.542967));
    lightsPos.push_back(glm::vec3(-33.441780, 13.047886, -8.249807));
    lightsPos.push_back(glm::vec3(-33.367992, 11.988689, 0.044874));
    lightsPos.push_back(glm::vec3(-33.317863, 12.029139, 2.161880));
    lightsPos.push_back(glm::vec3(-30.420286, -4.172046, -10.924128));
    lightsPos.push_back(glm::vec3(-31.670586, -4.449285, -12.687109));
    lightsPos.push_back(glm::vec3(-32.893528, -4.468543, -11.073335));
    lightsPos.push_back(glm::vec3(-31.236433, -3.920790, 7.515086));
    lightsPos.push_back(glm::vec3(-33.051868, -4.054377, 5.785084));
    lightsPos.push_back(glm::vec3(29.157068, -5.738323, -25.703356));
    lightsPos.push_back(glm::vec3(28.418732, -5.813434, -27.864910));
    lightsPos.push_back(glm::vec3(29.881533, -4.316147, -27.176771));
    lightsPos.push_back(glm::vec3(21.942091, 3.422779, -26.478367));
    lightsPos.push_back(glm::vec3(21.061813, 3.820476, -28.699600));
    lightsPos.push_back(glm::vec3(15.218609, 13.982447, -18.948248));
    lightsPos.push_back(glm::vec3(13.360641, 14.311971, -20.154787));
    lightsPos.push_back(glm::vec3(15.037455, 14.351202, -21.976988));
    lightsPos.push_back(glm::vec3(16.717236, 14.461295, -20.138376));
    lightsPos.push_back(glm::vec3(15.147987, 13.423956, -20.179867));
    lightsPos.push_back(glm::vec3(25.382650, -4.730032, 28.717518));
    lightsPos.push_back(glm::vec3(26.319933, -4.708989, 26.727564));
    lightsPos.push_back(glm::vec3(27.954140, -4.878076, 28.309433));
    lightsPos.push_back(glm::vec3(26.806595, -3.210355, 28.211967));
    lightsPos.push_back(glm::vec3(-15.359021, 15.661595, -14.043450));
    lightsPos.push_back(glm::vec3(-13.887574, 15.472223, -12.329331));
    lightsPos.push_back(glm::vec3(-11.803184, 15.207483, -13.742558));
    lightsPos.push_back(glm::vec3(-13.185013, 15.442325, -15.870077));
    lightsPos.push_back(glm::vec3(-13.327675, 14.226155, -14.230773));
    lightsPos.push_back(glm::vec3(12.880352, 15.060414, 10.433838));
    lightsPos.push_back(glm::vec3(14.236390, 15.054748, 8.506654));
    lightsPos.push_back(glm::vec3(12.318768, 15.280012, 7.139258));
    lightsPos.push_back(glm::vec3(10.946584, 15.240225, 8.859324));
    lightsPos.push_back(glm::vec3(19.853434, 12.626831, 21.435753));
    lightsPos.push_back(glm::vec3(20.361788, 12.483920, 23.746565));
    lightsPos.push_back(glm::vec3(22.533958, 12.134308, 23.132845));
    lightsPos.push_back(glm::vec3(21.895451, 11.993309, 21.164570));
    lightsPos.push_back(glm::vec3(19.982374, 13.730858, 16.642439));
    lightsPos.push_back(glm::vec3(17.819757, 13.356644, 17.378925));
    lightsPos.push_back(glm::vec3(18.224199, 12.899446, 18.969336));
    lightsPos.push_back(glm::vec3(20.046370, 12.942888, 19.164808));
    lightsPos.push_back(glm::vec3(15.864333, 13.469475, 19.604994));
    lightsPos.push_back(glm::vec3(14.567337, 13.449442, 20.612753));
    lightsPos.push_back(glm::vec3(15.472089, 13.728711, 22.016474));
    lightsPos.push_back(glm::vec3(16.925488, 13.743783, 21.158609));
    lightsPos.push_back(glm::vec3(22.403194, 7.025841, 17.399317));
    lightsPos.push_back(glm::vec3(21.112549, 6.918302, 18.530117));
    lightsPos.push_back(glm::vec3(22.089083, 7.037092, 19.827230));
    lightsPos.push_back(glm::vec3(23.325922, 6.843616, 18.656767));
    lightsPos.push_back(glm::vec3(14.073922, -5.318523, 28.994984));
    lightsPos.push_back(glm::vec3(15.313214, -5.315862, 27.484316));
    lightsPos.push_back(glm::vec3(16.645580, -5.316422, 28.692030));
    lightsPos.push_back(glm::vec3(-6.354994, 15.448309, -2.904307));
    lightsPos.push_back(glm::vec3(8.237203, 13.167086, 1.770059));
    lightsPos.push_back(glm::vec3(-2.253086, 13.405657, 7.222772));
    lightsPos.push_back(glm::vec3(2.718040, 15.510720, -8.353910));
    lightsPos.push_back(glm::vec3(0.147286, 15.142395, -3.602446));
    lightsPos.push_back(glm::vec3(-0.838741, 12.651252, -0.985295));
    lightsPos.push_back(glm::vec3(0.091495, 15.020788, 2.978199));
    lightsPos.push_back(glm::vec3(2.394860, 13.234718, 0.486012));
    lightsPos.push_back(glm::vec3(-2.816199, 14.456532, -0.775940));
    lightsPos.push_back(glm::vec3(-3.577841, 16.102932, -6.938037));
    lightsPos.push_back(glm::vec3(-6.969079, 14.770109, 2.830968));
    lightsPos.push_back(glm::vec3(4.645925, 12.319436, 6.044405));
    lightsPos.push_back(glm::vec3(7.662771, 14.391157, -4.189649));
    lightsPos.push_back(glm::vec3(-32.762825, 11.771997, 0.493767));
    lightsPos.push_back(glm::vec3(-33.220024, 12.100765, -12.567150));
    lightsPos.push_back(glm::vec3(-6.332181, 14.306339, 6.465876));
    lightsPos.push_back(glm::vec3(-9.648393, 15.589612, -0.141336));
    lightsPos.push_back(glm::vec3(12.781223, 14.225384, 8.946852));
    lightsPos.push_back(glm::vec3(19.298357, 12.496197, 18.121078));
    lightsPos.push_back(glm::vec3(15.845408, 12.936720, 20.699841));
    lightsPos.push_back(glm::vec3(21.411831, 11.357573, 22.192160));
    lightsPos.push_back(glm::vec3(22.113304, 6.380415, 18.553059));
    lightsPos.push_back(glm::vec3(-1.359086, 16.805197, -9.154959));
    lightsPos.push_back(glm::vec3(6.683021, 15.192085, -7.379675));
    lightsPos.push_back(glm::vec3(1.012856, 12.715869, 7.182148));
    lightsPos.push_back(glm::vec3(-5.958708, 14.312449, 6.308965));
    lightsPos.push_back(glm::vec3(-25.967623, -4.809686, 26.419350));
    lightsPos.push_back(glm::vec3(-26.918743, 1.663134, 25.485203));
    
    // shader configuration
    // --------------------
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedo", 2);
    shaderLightingPass.setInt("ssao", 3);
    shaderSSAO.use();
    shaderSSAO.setInt("gPosition", 0);
    shaderSSAO.setInt("gNormal", 1);
    shaderSSAO.setInt("texNoise", 2);
    shaderSSAOBlur.use();
    shaderSSAOBlur.setInt("ssaoInput", 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    camera.MovementSpeed = 20.0f;
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

        if (g_bufferScreenScale != g_bufferScreenScaleCurrent)
        {
            g_bufferScreenScaleCurrent = g_bufferScreenScale;
            BUFFER_WIDTH = SCR_WIDTH * g_bufferScreenScaleCurrent;
            BUFFER_HEIGHT = SCR_HEIGHT * g_bufferScreenScaleCurrent;
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, gAlbedo);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, BUFFER_WIDTH, BUFFER_HEIGHT);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
        }
        if (g_kernelSize != g_kernelSizeCurrent)
        {
            g_kernelSizeCurrent = g_kernelSize;
            ssaoKernel.clear();
            for (unsigned int i = 0; i < g_kernelSizeCurrent; ++i)
            {
                glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
                sample = glm::normalize(sample);
                sample *= randomFloats(generator);
                float scale = float(i) / (float)g_kernelSizeCurrent;

                // scale samples s.t. they're more aligned to center of kernel
                scale = ourLerp(0.1f, 1.0f, scale * scale);
                sample *= scale;
                ssaoKernel.push_back(sample);
            }
        }
        if (lightColorArray[0] != lightColor.r ||
            lightColorArray[1] != lightColor.g ||
            lightColorArray[2] != lightColor.b)
        {
            lightColor.r = lightColorArray[0];
            lightColor.g = lightColorArray[1];
            lightColor.b = lightColorArray[2];
        }
        

        // render
        // ------
        glViewport(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)BUFFER_WIDTH / (float)BUFFER_HEIGHT, 0.1f, far_plane);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);
            shaderGeometryPass.use();
            shaderGeometryPass.setMat4("projection", projection);
            shaderGeometryPass.setMat4("view", view);
            // room cube
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
            model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));
            shaderGeometryPass.setMat4("model", model);
            shaderGeometryPass.setInt("invertedNormals", 1); // invert normals as we're inside the cube
            //renderCube();
            shaderGeometryPass.setInt("invertedNormals", 0); 
            // backpack model on the floor
            /*model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
            model = glm::scale(model, glm::vec3(1.0f));
            shaderGeometryPass.setMat4("model", model);
            backpack.Draw(shaderGeometryPass.ID);*/
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(g_personPos[0], g_personPos[1], g_personPos[2]));
            model = glm::rotate(model, glm::radians(g_personRotateAngle), glm::vec3(g_personRotateVec[0], g_personRotateVec[1], g_personRotateVec[2]));
            model = glm::scale(model, glm::vec3(g_personScale));
            shaderGeometryPass.setMat4("model", model);
            person.Draw(shaderGeometryPass.ID);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0, -10.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
            shaderGeometryPass.setMat4("model", model);
            room.Draw(shaderGeometryPass.ID);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        float offsetScale = (1.0 - g_textureScale) / 2;
        float offsetWidth = BUFFER_WIDTH * offsetScale;
        float offsetHeight = BUFFER_HEIGHT * offsetScale;
        glBlitFramebuffer((GLint)offsetWidth, (GLint)offsetHeight, BUFFER_WIDTH * g_textureScale + offsetWidth, BUFFER_HEIGHT * g_textureScale + offsetHeight
        , 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        // 2. generate SSAO texture
        // ------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAO.use();
            // Send kernel + rotation 
            for (unsigned int i = 0; i < g_kernelSizeCurrent; ++i)
                shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
            shaderSSAO.setMat4("projection", projection);
            shaderSSAO.setInt("kernelSize", g_kernelSize);
            shaderSSAO.setFloat("radius", g_radius);
            shaderSSAO.setFloat("bias", g_bias);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 3. blur SSAO texture to remove noise
        // ------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderSSAOBlur.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
        // -----------------------------------------------------------------------------------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderLightingPass.use();
        shaderLightingPass.setInt("currentLightNumber", lightsPos.size());
        // send light relevant uniforms
        for (unsigned int i = 0; i < lightsPos.size(); i++)
        {
            glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightsPos[i], 1.0));
            shaderLightingPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPosView);
        }
        shaderLightingPass.setVec3("lightColor", lightColor);
        // Update attenuation parameters
        shaderLightingPass.setFloat("Linear", linear);
        shaderLightingPass.setFloat("Quadratic", quadratic);
        shaderLightingPass.setFloat("textureScale", g_textureScale);
        shaderLightingPass.setFloat("ambientRatio", g_ambientRatio);
        shaderLightingPass.setVec3("viewPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        renderQuad();

        // 5. render light boxs for debugging visually
        glEnable(GL_DEPTH_TEST);
        if (g_lightSetting)
        {
            shaderLightBox.use();
            shaderLightBox.setMat4("projection", projection);
            shaderLightBox.setMat4("view", view);
            for (unsigned int i = 0; i < lightsPos.size(); i++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, lightsPos[i]);
                model = glm::scale(model, glm::vec3(0.125f));
                shaderLightBox.setMat4("model", model);
                shaderLightBox.setVec3("lightColor", lightColor);
                shaderLightBox.setFloat("textureScale", g_textureScale);
                renderCube();
            }
        }

        // 6. normal visualizing
        //glViewport(SCR_WIDTH, 0, SCR_WIDTH, SCR_HEIGHT);
        //glClear(GL_COLOR_BUFFER_BIT);
        if (g_showNormal)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            normalShader.use();
            normalShader.setMat4("projection", projection);
            normalShader.setMat4("view", view);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0, -10.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
            normalShader.setMat4("model", model);
            normalShader.setFloat("textureScale", g_textureScale);
            room.Draw(normalShader.ID);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Param Setting");
        ImGui::Text("Press \"1\" to show cursor and switch to setting mode.");
        ImGui::Text("Press \"2\" to hide cursor and finishe setting mode.");
        ImGui::Text("Current cursor mode: %d", g_bCaptureCursor ? 2 : 1);
        ImGui::SliderFloat("Texture scale", &g_textureScale, 0.7f, 1.0f);
        ImGui::SliderFloat("Buffer screen scale", &g_bufferScreenScale, 1.0f, 5.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::SliderInt("Kernel size", &g_kernelSize, 32, 300);
        ImGui::SliderFloat("radius", &g_radius, 0.1f, 2.0f);
        ImGui::SliderFloat("bias", &g_bias, 0.0f, 0.2f);
        ImGui::SliderFloat("far plane", &far_plane, 50.0f, 1000.f, "%.1f");
        ImGui::SliderFloat("Movement speed", &camera.MovementSpeed, 2.5f, 50.f, "%.1f");
        ImGui::SliderFloat("linear", &linear, 0.001f, 0.1f);
        ImGui::SliderFloat("quadratic", &quadratic, 0.001f, 0.1f);
        ImGui::SliderFloat("Ambient Ratio", &g_ambientRatio, 0.1f, 1.0f);
        ImGui::ColorEdit3("light color", lightColorArray);
        ImGui::Checkbox("Show normal for debugging", &g_showNormal);
        ImGui::Checkbox("light setting", &g_lightSetting);
        if (g_lightSetting)
            ImGui::SliderFloat("lightting normal offset scale", &g_normalOffsetScale, 0.001f, 5.0f);
        ImGui::SliderFloat3("person position", g_personPos, -100.0, 100.0);
        ImGui::SliderFloat3("person rotate vector", g_personRotateVec, 0.0, 1.0);
        ImGui::SliderFloat("Person rotate angle", &g_personRotateAngle, -180.0f, 180.0f);
        ImGui::SliderFloat("Person scale", &g_personScale, 0.1, 2.0);

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

    glfwTerminate();
    return 0;
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
    //glViewport(0, 0, width, height);
    SCR_WIDTH = width - ImGui_Width;
    SCR_HEIGHT = height;
    g_bufferScreenScaleCurrent = (float)BUFFER_WIDTH / (float)SCR_WIDTH;
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
        if (action == GLFW_PRESS && g_lightSetting)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            float viewportX = x;
            float viewportY = SCR_HEIGHT - y;
            if (viewportX <= SCR_WIDTH && viewportY <= SCR_HEIGHT)
            {
                printf("Mouse clicked at windows (%f, %f)\n", x, y);
                printf("Mouse clicked at glViewport (%f, %f)\n", viewportX, viewportY);
                float viewPortOffsetX = SCR_WIDTH * (1 - g_textureScale) / 2;
                float viewPortOffsetY = SCR_HEIGHT * (1 - g_textureScale) / 2;
                printf("Mouse clicked at viewPortOffset (%f, %f)\n", viewPortOffsetX, viewPortOffsetY);
                viewportX = viewportX * g_textureScale + viewPortOffsetX;
                viewportY = viewportY * g_textureScale + viewPortOffsetY;
                printf("Mouse clicked at glViewport (%f, %f)\n", viewportX, viewportY);
                unsigned int gBufferX = viewportX * g_bufferScreenScale;
                unsigned int gBufferY = viewportY * g_bufferScreenScale;
                printf("Mouse clicked at g-Buffer (%d, %d)\n", gBufferX, gBufferY);
                //gPosition
                GLint width, height;
                glBindTexture(GL_TEXTURE_2D, gPosition);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);  
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
                //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_DEPTH, &depth);
                std::cout << "width: " << width << ", height: " << height << ", sizeof(GLfloat): " << sizeof(GLfloat) << std::endl;
                g_positionDatas.resize(width * height * 4);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, g_positionDatas.data());
                glBindTexture(GL_TEXTURE_2D, gNormal);
                g_normalDatas.resize(width * height * 4);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, g_normalDatas.data());
                unsigned int dataIndex = (gBufferX + gBufferY * width) * 4;
                glm::vec4 mouseViewSpacePos = glm::vec4(g_positionDatas[dataIndex], g_positionDatas[dataIndex + 1], g_positionDatas[dataIndex + 2], 1.0f);
                glm::vec3 mouseViewSpaceNormal = glm::vec3(g_normalDatas[dataIndex], g_normalDatas[dataIndex + 1], g_normalDatas[dataIndex + 2]);
                mouseViewSpacePos.x += mouseViewSpaceNormal.x * g_normalOffsetScale;
                mouseViewSpacePos.y += mouseViewSpaceNormal.y * g_normalOffsetScale;
                mouseViewSpacePos.z += mouseViewSpaceNormal.z * g_normalOffsetScale;
                glm::vec3 lightPos = glm::inverse(camera.GetViewMatrix()) * mouseViewSpacePos;
                lightsPos[lightsPos.size() - 1] = lightPos;
                printf("Mouse clicked at view space (%f, %f, %f)\n", mouseViewSpacePos.x, mouseViewSpacePos.y, mouseViewSpacePos.z);
                printf("Mouse clicked at model space (%f, %f, %f)\n", lightPos.x, lightPos.y, lightPos.z);
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
