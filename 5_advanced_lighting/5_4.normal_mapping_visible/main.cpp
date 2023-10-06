#include <glad/glad.h>
#include <GLFW/glfw3.h>
//#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>
#include <model.h>

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderQuad();
void computeTangents(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uv, std::vector<glm::vec3>& normals
, const std::vector<unsigned int>& indices, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents);
void createSegmentPlane(int segmentNumber);
void renderSegmentQuad();

// segment plane param
struct SegmentPlaneData
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    std::vector<unsigned int> indices;
    std::vector<float> data;
} g_segmentPlaneVertexData;
unsigned int segmentQuadVbo, segmentQuadEbo;

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
unsigned int ImGui_Width = 500;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//imGui Param
bool g_bCaptureCursor = true;
bool g_showVertexNormal = false;
bool g_showTextureNormal = true;
bool g_bTBN = true;
int g_planeSegmentNumber = 3;
int g_planeSegmentNumberCurrent = 0;
float g_dotMax = 0.9f;

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
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader(VERTEX_FILE, FRAGMENT_FILE);
    Shader tbnShader(VERTEX_TBN_FILE, FRAGMENT_NORMAL_FILE, GEOMETRY_TBN_FILE);
    Shader normalShader(VERTEX_NORMAL_FILE, FRAGMENT_NORMAL_FILE, GEOMETRY_NORMAL_FILE);

    // load textures
    // -------------
    unsigned int diffuseMap = loadTexture(RESOURCES_DIR"/textures/brickwall.jpg");
    unsigned int normalMap  = loadTexture(RESOURCES_DIR"/textures/brickwall_normal.jpg");

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseMap", 0);
    shader.setInt("normalMap", 1);
    normalShader.use();
    normalShader.setInt("normalMap", 1);

    // lighting info
    // -------------
    glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

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

        if (g_planeSegmentNumberCurrent != g_planeSegmentNumber)
        {
            g_planeSegmentNumberCurrent = g_planeSegmentNumber;
            createSegmentPlane(g_planeSegmentNumberCurrent);
            glBindBuffer(GL_ARRAY_BUFFER, segmentQuadVbo);
            glBufferData(GL_ARRAY_BUFFER, g_segmentPlaneVertexData.data.size() * sizeof(float), &g_segmentPlaneVertexData.data[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, segmentQuadEbo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_segmentPlaneVertexData.indices.size() * sizeof(unsigned int), &g_segmentPlaneVertexData.indices[0], GL_STATIC_DRAW);
        }
        

        // render
        // ------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // configure view/projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // render normal-mapped quad
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-70.0f), glm::normalize(glm::vec3(1.0, 0.0, 0.0))); // rotate the quad to show normal mapping from multiple directions
        shader.setMat4("model", model);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        renderQuad();
        //renderSegmentQuad();

        tbnShader.use();
        tbnShader.setMat4("projection", projection);
        tbnShader.setMat4("view", view);
        tbnShader.setMat4("model", model);

        normalShader.use();
        normalShader.setMat4("projection", projection);
        normalShader.setMat4("view", view);
        normalShader.setMat4("model", model);

        // render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
        shader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);
        renderQuad();

        //Normal visualization
        if (g_showVertexNormal)
        {
            tbnShader.use();
            renderQuad();
            //renderSegmentQuad();
        }
        if (g_showTextureNormal)
        {
            normalShader.use();
            normalShader.setFloat("dotMax", g_dotMax);
            normalShader.setBool("enableTBN", g_bTBN);
            renderSegmentQuad();
        }
        

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Param Setting");
        ImGui::Text("Press \"1\" to show cursor and switch to setting mode.");
        ImGui::Text("Press \"2\" to hide cursor and finishe setting mode.");
        ImGui::Text("Current cursor mode: %d", g_bCaptureCursor ? 2 : 1);
        ImGui::SliderInt("Plane segment number", &g_planeSegmentNumber, 1, 500);
        ImGui::Checkbox("Show vertex normal", &g_showVertexNormal);
        ImGui::Checkbox("Show texture normal", &g_showTextureNormal);
        if (g_showTextureNormal)
            ImGui::Checkbox("TBN Matrix", &g_bTBN);
        if (g_bTBN)
            ImGui::SliderFloat("Filter: dot(normalMap, N) Maximen", &g_dotMax, 0.0f, 1.0f);
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

    glfwTerminate();
    return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
        glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);  
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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

void computeTangents(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uv, std::vector<glm::vec3>& normals
, const std::vector<unsigned int>& indices, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents)
{
    for (unsigned int i = 0; i + 2 < indices.size(); i++) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3& v0 = vertices[i0];
        glm::vec3& v1 = vertices[i1];
        glm::vec3& v2 = vertices[i2];

        glm::vec2& uv0 = uv[i0];
        glm::vec2& uv1 = uv[i1];
        glm::vec2& uv2 = uv[i2];

        glm::vec3& t0 = tangents[i0];
        glm::vec3& t1 = tangents[i1];
        glm::vec3& t2 = tangents[i2];

        glm::vec3 edge1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
        glm::vec3 edge2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

        float deltaU1 = uv1.x - uv0.x;
        float deltaV1 = uv1.y - uv0.y;
        float deltaU2 = uv2.x - uv0.x;
        float deltaV2 = uv2.y - uv0.y;

        float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

        glm::vec3 tangent;
        tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
        tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
        tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

        t0 += tangent;
        t1 += tangent;
        t2 += tangent;
    }

    for (unsigned int i = 0; i < vertices.size(); i++) 
    {
        glm::vec3 normal(normals[i]);
        glm::vec3 tangent(tangents[i]);

        //if (fabs(glm::dot(normal, tangent)) > 0.01)
        //    std::cout << "glm::dot(normal, tangent) = " << glm::dot(normal, tangent) << endl;
        // Gram-Schmidt orthogonalize
        tangent = glm::normalize(tangent - glm::dot(normal, tangent) * normal);
        //tangent = glm::normalize(tangent);

        glm::vec3 bitangent = glm::cross(normal, tangent);
        bitangents.push_back(bitangent);

        tangents[i] = tangent;
    }
}

void createSegmentPlane(int segmentNumber)
{
    std::vector<glm::vec3> &positions = g_segmentPlaneVertexData.positions;
    std::vector<glm::vec2> &uv = g_segmentPlaneVertexData.uv;
    std::vector<glm::vec3> &normals = g_segmentPlaneVertexData.normals;
    std::vector<glm::vec3> &tangents = g_segmentPlaneVertexData.tangents;
    std::vector<glm::vec3> &bitangents = g_segmentPlaneVertexData.bitangents;
    std::vector<unsigned int> &indices = g_segmentPlaneVertexData.indices;

    positions.clear();
    uv.clear();
    normals.clear();
    tangents.clear();
    bitangents.clear();
    indices.clear();

    const unsigned int X_SEGMENTS = segmentNumber;
    const unsigned int Y_SEGMENTS = segmentNumber;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        float xSegment = (float)x / (float)X_SEGMENTS;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = 2.0 * xSegment - 1.0;
            float yPos = 2.0 * ySegment - 1.0;
            float zPos = 0.0f;

            g_segmentPlaneVertexData.positions.push_back(glm::vec3(xPos, yPos, zPos));
            uv.push_back(glm::vec2(xSegment, ySegment));
            glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
            normals.push_back(normal);
            tangents.push_back(glm::vec3(0.0f));
        }
    }

    //bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        //if (!oddRow) // even rows: y == 0, y == 2; and so on
        //{
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        //}
        //else
        //{
            //for (int x = X_SEGMENTS; x >= 0; --x)
            //{
                //indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                //indices.push_back(y * (X_SEGMENTS + 1) + x);
            //}
       // }
        //oddRow = !oddRow;
    }

    computeTangents(positions, uv, normals, indices, tangents, bitangents);

    std::vector<float> &data = g_segmentPlaneVertexData.data;
    data.clear();
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
        if (tangents.size() > 0)
        {
            data.push_back(tangents[i].x);
            data.push_back(tangents[i].y);
            data.push_back(tangents[i].z);
        }
        if (bitangents.size() > 0)
        {
            data.push_back(bitangents[i].x);
            data.push_back(bitangents[i].y);
            data.push_back(bitangents[i].z);
        }
    }
}

unsigned int segmentQuad = 0;
void renderSegmentQuad()
{
    if (segmentQuad == 0)
    {
        //createSegmentPlane(g_planeSegmentNumberCurrent);
        glGenVertexArrays(1, &segmentQuad);

        glGenBuffers(1, &segmentQuadVbo);
        glGenBuffers(1, &segmentQuadEbo);

        glBindVertexArray(segmentQuad);
        glBindBuffer(GL_ARRAY_BUFFER, segmentQuadVbo);
        glBufferData(GL_ARRAY_BUFFER, g_segmentPlaneVertexData.data.size() * sizeof(float), &g_segmentPlaneVertexData.data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, segmentQuadEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_segmentPlaneVertexData.indices.size() * sizeof(unsigned int), &g_segmentPlaneVertexData.indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3 + 3 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
    }

    glBindVertexArray(segmentQuad);
    glDrawElements(GL_TRIANGLE_STRIP, g_segmentPlaneVertexData.indices.size(), GL_UNSIGNED_INT, 0);
}