#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <shader_m.h>
#include <camera.h>

#include <iostream>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void drawVisualizerFrustums(Shader* shader);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int viewportSeperate = 20;
const unsigned int NUM_PATCH_PTS = 4;

// camera - give pretty starting point
Camera camera(glm::vec3(67.0f, 627.5f, 169.9f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              -128.1f, -42.4f);
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float cameraNearPlane = 0.1f;
float cameraFarPlane = 3000.0f;

// camera 1
Camera camera1(glm::vec3(67.0f, 627.5f, 179.9f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              -128.1f, -42.4f);
float lastX1 = (float)SCR_WIDTH / 2.0;
float lastY1 = (float)SCR_HEIGHT / 2.0;
bool firstMouse1 = true;

int g_currentCamera = 0;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// imGui Params
int ImGui_Width = 600;
bool g_bUseWireframe = false;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH * 2 + viewportSeperate + ImGui_Width, SCR_HEIGHT, "LearnOpenGL: Terrain GPU", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
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

    GLint maxTessLevel;
    glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
    std::cout << "Max available tess level: " << maxTessLevel << std::endl;

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    // ------------------------------------
    Shader tessHeightMapShader(VERTEX_FILE,FRAGMENT_FILE, nullptr,            // if wishing to render as is
                               TCS_FILE, TES_FILE);
    Shader debugFrustumShader(VERTEX_DEBUG_FRUSTUM_FILE, FRAGMENT_DEBUG_FRUSTUM_FILE);

    // load and create a texture
    // -------------------------
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load(RESOURCES_DIR"/textures/heightmaps/heightmap.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        tessHeightMapShader.use();
        tessHeightMapShader.setInt("heightMap", 0);
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // color texture
    unsigned int colorTexture;
    glGenTextures(1, &colorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load(RESOURCES_DIR"/textures/heightmaps/color.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        tessHeightMapShader.use();
        tessHeightMapShader.setInt("colorMap", 1);
        std::cout << "Loaded color texture of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load color texture" << std::endl;
    }
    stbi_image_free(data);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    std::vector<float> vertices;

    unsigned rez = 20;
    for(unsigned i = 0; i <= rez-1; i++)
    {
        for(unsigned j = 0; j <= rez-1; j++)
        {
            vertices.push_back(2.6f * i / rez - 1.3f); // v.x
            vertices.push_back(2.6f * j / rez - 1.3f); // v.y
            vertices.push_back(0.0f); // v.z

            vertices.push_back(2.6f * (i+1) / rez - 1.3f); // v.x
            vertices.push_back(2.6f * j / rez - 1.3f); // v.y
            vertices.push_back(0.0f); // v.z

            vertices.push_back(2.6f * i / rez - 1.3f); // v.x
            vertices.push_back(2.6f * (j+1) / rez - 1.3f); // v.y
            vertices.push_back(0.0f); // v.z

            vertices.push_back(2.6f * (i+1) / rez - 1.3f); // v.x
            vertices.push_back(2.6f * (j+1) / rez - 1.3f); // v.y
            vertices.push_back(0.0f); // v.z
        }
    }
    tessHeightMapShader.use();
    tessHeightMapShader.setVec2("xMinMax", glm::vec2(-width/2.0f, width/2.0f));
    tessHeightMapShader.setVec2("zMinMax", glm::vec2(-height/2.0f, height/2.0f));
    std::cout << "Loaded " << rez*rez << " patches of 4 control points each" << std::endl;
    std::cout << "Processing " << rez*rez*4 << " vertices in vertex shader" << std::endl;

    // first, configure the cube's VAO (and terrainVBO)
    unsigned int terrainVAO, terrainVBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texCoord attribute
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
    //glEnableVertexAttribArray(1);

    glPatchParameteri(GL_PATCH_VERTICES, NUM_PATCH_PTS);

    camera.MovementSpeed = 100.0f;
    camera1.MovementSpeed = 100.0f;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // ImFont *font = io.Fonts->AddFontFromFileTTF(font_name.c_str(), 15, nullptr,
    //                                             io.Fonts->GetGlyphRangesChineseFull());
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // imGui param
    int outOfFrustumTessLevel = 0;
    float power = 2.0f;
    float MAX_DISTANCE = 2400.0;
    bool bUseColorMap = false;
    float heightScale = 1.0f;

    // fps param
    int nbFrames = 0;
    char title[256];
    float deltaTimeFPS = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // calc FPS
		deltaTimeFPS += deltaTime;
        if (deltaTimeFPS >= 1.0 && nbFrames > 0)
        {
            sprintf_s(title, 256, "LearnOpenGL @ %.3f ms/frame (%.1f FPS)", deltaTimeFPS * 1000 / float(nbFrames), float(nbFrames) / deltaTimeFPS);
            glfwSetWindowTitle(window, title);
            nbFrames = 0;
            deltaTimeFPS = 0.0f;
        }
        nbFrames++;

        // input
        // -----
        processInput(window);

        if (g_bUseWireframe)
        {
            // uncomment this call to draw in wireframe polygons.
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        tessHeightMapShader.use();

        // view/projection transformations
        float fov = glm::radians(camera.Zoom);
        glm::mat4 projection = glm::perspective(fov, (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNearPlane, cameraFarPlane);
        glm::mat4 view = camera.GetViewMatrix();
        tessHeightMapShader.setMat4("projection", projection);
        tessHeightMapShader.setMat4("inverseProjection", glm::inverse(projection));
        tessHeightMapShader.setMat4("view", view);
        tessHeightMapShader.setMat4("mainView", view);
        tessHeightMapShader.setMat4("inverseMainView", glm::inverse(view));
        tessHeightMapShader.setVec3("cameraPos", camera.Position);
        tessHeightMapShader.setInt("outOfFrustumTessLevel", outOfFrustumTessLevel);
        tessHeightMapShader.setFloat("power", power);
        tessHeightMapShader.setFloat("MAX_DISTANCE", MAX_DISTANCE);
        tessHeightMapShader.setBool("bUseColorMap", bUseColorMap);
        tessHeightMapShader.setFloat("heightScale", heightScale);

        // world transformation
        // glm::mat4 model = glm::mat4(1.0f);
        // tessHeightMapShader.setMat4("model", model);

        // render the terrain
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS*rez*rez);

        // camera1 viewport
		glViewport(SCR_WIDTH + viewportSeperate, 0, SCR_WIDTH, SCR_HEIGHT);
		projection = glm::perspective(glm::radians(camera1.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNearPlane, 5000.0f);
		view = camera1.GetViewMatrix();
		tessHeightMapShader.setMat4("projection", projection);
		tessHeightMapShader.setMat4("view", view);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS*rez*rez);

        // debug frustum visualization
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		debugFrustumShader.use();
		debugFrustumShader.setMat4("projection", projection);
		debugFrustumShader.setMat4("view", view);
		glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawVisualizerFrustums(&debugFrustumShader);
		glDisable(GL_BLEND);

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH * 2 + viewportSeperate, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(ImGui_Width - 10, SCR_HEIGHT - 10));
        ImGui::Begin("Param Setting", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::SliderFloat("camera Far Plane", &cameraFarPlane, 500.0f, 10000.0f);
        ImGui::SliderInt("Tess Level out of frustum" , &outOfFrustumTessLevel, 0, 4);
        ImGui::Checkbox("Wireframe", &g_bUseWireframe);
        ImGui::Checkbox("Use Color Map", &bUseColorMap);
        ImGui::SliderFloat("power", &power, 0.1f, 5.0f, "%.1f");
        ImGui::SliderFloat("MAX_DISTANCE", &MAX_DISTANCE, 100.0f, 10000.0f, "%.1f");
        ImGui::SliderFloat("Height Scale", &heightScale, 0.1f, 10.0f, "%.1f");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &terrainVAO);
    glDeleteBuffers(1, &terrainVBO);

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
    {
        if (g_currentCamera == 0)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        else if (g_currentCamera == 1)
            camera1.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if (g_currentCamera == 0)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        else if (g_currentCamera == 1)
            camera1.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        if (g_currentCamera == 0)
            camera.ProcessKeyboard(LEFT, deltaTime);
        else if (g_currentCamera == 1)
            camera1.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        if (g_currentCamera == 0)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        else if (g_currentCamera == 1)
            camera1.ProcessKeyboard(RIGHT, deltaTime);
    }

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (g_currentCamera < 0)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_currentCamera = 0;
        firstMouse = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (g_currentCamera < 0)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_currentCamera = 1;
        firstMouse1 = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
    {
        g_currentCamera = -1;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    //glViewport(0, 0, width, height);
}

// glfw: whenever a key event occurs, this callback is called
// ---------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int modifiers)
{
    if(action == GLFW_PRESS)
    {
        switch(key)
        {
            case GLFW_KEY_SPACE:
                g_bUseWireframe = !g_bUseWireframe;
                break;
            case GLFW_KEY_LEFT_ALT:
                g_currentCamera = -1;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
            case GLFW_KEY_1:
                g_currentCamera = 0;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
            case GLFW_KEY_2:
                g_currentCamera = 1;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
            default:
                break;
        }
    }
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (g_currentCamera == 0)
    {
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
    else if (g_currentCamera == 1)
    {
        if (firstMouse1)
        {
            lastX1 = xpos;
            lastY1 = ypos;
            firstMouse1 = false;
        }

        float xoffset = xpos - lastX1;
        float yoffset = lastY1 - ypos; // reversed since y-coordinates go from bottom to top

        lastX1 = xpos;
        lastY1 = ypos;

        camera1.ProcessMouseMovement(xoffset, yoffset);
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_currentCamera == 0)
        camera.ProcessMouseScroll(yoffset);
    else if (g_currentCamera == 1)
        camera1.ProcessMouseScroll(yoffset);
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview)
{
    const auto inv = glm::inverse(projview);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    return getFrustumCornersWorldSpace(proj * view);
}

// draw Cascade Frustums Visualizers
GLuint visualizerFrustumVAO;
GLuint visualizerFrustumVBO;
GLuint visualizerFrustumEBO;

void drawVisualizerFrustums(Shader* shader)
{
    const GLuint indices[] = {
        0, 2, 3,
        0, 3, 1,
        4, 6, 2,
        4, 2, 0,
        5, 7, 6,
        5, 6, 4,
        1, 3, 7,
        1, 7, 5,
        6, 7, 3,
        6, 3, 2,
        1, 5, 4,
        0, 1, 4
    };

    const glm::vec4 color = {0.0, 1.0, 0.0, 0.2f};
	const auto proj = glm::perspective(
		glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNearPlane,
		cameraFarPlane);
	const auto corners = getFrustumCornersWorldSpace(proj, camera.GetViewMatrix());
	std::vector<glm::vec3> vec3s;
	for (const auto &v : corners)
	{
		vec3s.push_back(glm::vec3(v));
	}

	glGenVertexArrays(1, &visualizerFrustumVAO);
	glGenBuffers(1, &visualizerFrustumVBO);
	glGenBuffers(1, &visualizerFrustumEBO);

	glBindVertexArray(visualizerFrustumVAO);

	glBindBuffer(GL_ARRAY_BUFFER, visualizerFrustumVBO);
	glBufferData(GL_ARRAY_BUFFER, vec3s.size() * sizeof(glm::vec3), &vec3s[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, visualizerFrustumEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

	glBindVertexArray(visualizerFrustumVAO);
	shader->setVec4("color", color);
	glDrawElements(GL_TRIANGLES, GLsizei(36), GL_UNSIGNED_INT, 0);

	glDeleteBuffers(1, &visualizerFrustumVBO);
	glDeleteBuffers(1, &visualizerFrustumEBO);
	glDeleteVertexArrays(1, &visualizerFrustumVAO);

	glBindVertexArray(0);
}