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
void computeTangents(std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uv, std::vector<glm::vec3>& normals
, const std::vector<unsigned int>& indices, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents);
void renderSphere();

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 720;
unsigned int ImGui_Width = 500;
float heightScale = 0.1f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//imGui Param
bool g_bCaptureCursor = true;
bool g_showNormal = false;
bool g_showVertexNormal = false;
bool g_showTextureNormal = true;
bool g_bTBN = true;

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
    //glEnable(GL_CULL_FACE);

    // build and compile shaders
    // -------------------------
    Shader shader(VERTEX_FILE, FRAGMENT_FILE);
    Shader normalShader(VERTEX_NORMAL_FILE, FRAGMENT_NORMAL_FILE, GEOMETRY_FILE);
    Shader normalTextureShader(VERTEX_NORMAL_TEXTURE_FILE, FRAGMENT_NORMAL_FILE, GEOMETRY_FILE);

    shader.use();
    shader.setInt("albedoMap", 0);
    shader.setInt("normalMap", 1);
    shader.setInt("metallicMap", 2);
    shader.setInt("roughnessMap", 3);
    shader.setInt("aoMap", 4);
    shader.setInt("heightMap", 5);
    shader.setInt("emission", 6);

    normalShader.use();
    normalShader.setInt("heightMap", 5);
    normalShader.setVec4("normalColor", glm::vec4(1.0, 1.0, 0.0, 1.0));

    normalTextureShader.use();
    normalTextureShader.setInt("normalMap", 1);
    normalTextureShader.setInt("heightMap", 5);
    normalTextureShader.setVec4("normalColor", glm::vec4(0.0, 0.0, 1.0, 1.0));

    // load PBR material textures
    // --------------------------
    stbi_set_flip_vertically_on_load(true);//网上找的图需要翻转Y坐标
    unsigned int albedo    = loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_albedo.png");
    unsigned int normal    = loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_normal-ogl.png");
    unsigned int metallic  = loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_metallic.png");
    unsigned int roughness = loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_roughness.png");
    unsigned int ao        = loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_ao.png");
    unsigned int heightTex = loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_height.png");
    unsigned int emission =  loadTexture(RESOURCES_DIR"/textures/pbr/columned-lava-rock-bl/columned-lava-rock_emissive.png");

    // lights
    // ------
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(250.0f, 250.0f, 250.0f),
    };
    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 3.0;

    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    shader.use();
    shader.setMat4("projection", projection);

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

        // render
        // ------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setVec3("camPos", camera.Position);
        shader.setFloat("heightScale", heightScale); // adjust with Q and E keys
        

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallic);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, roughness);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, ao);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, heightTex);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, emission);

        // render rows*column number of spheres with material properties defined by textures (they all have the same material properties)
        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row)
        {
            for (int col = 0; col < nrColumns; ++col)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (float)(col - (nrColumns / 2)) * spacing,
                    (float)(row - (nrRows / 2)) * spacing,
                    0.0f
                ));
                shader.setMat4("model", model);
                shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
                renderSphere();
            }
        }

        // render light source (simply re-render sphere at light positions)
        // this looks a bit off as we use the same shader, but it'll make their positions obvious and 
        // keeps the codeprint small.
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
            newPos = lightPositions[i];
            shader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            shader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            shader.setMat4("model", model);
            shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
            renderSphere();
        }

        // 法向量可视化
        if (g_showNormal && (g_showVertexNormal || g_showTextureNormal))
        {
            normalShader.use();
            normalShader.setMat4("projection", projection);
            normalShader.setMat4("view", view);
            normalShader.setFloat("heightScale", heightScale); // adjust with Q and E keys

            normalTextureShader.use();
            normalTextureShader.setMat4("projection", projection);
            normalTextureShader.setMat4("view", view);
            normalTextureShader.setFloat("heightScale", heightScale); // adjust with Q and E keys
            for (int row = 0; row < nrRows; ++row)
            {
                for (int col = 0; col < nrColumns; ++col)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(
                        (float)(col - (nrColumns / 2)) * spacing,
                        (float)(row - (nrRows / 2)) * spacing,
                        0.0f
                    ));
                    if (g_showVertexNormal)
                    {
                        normalShader.use();
                        normalShader.setMat4("model", model);
                        renderSphere();
                    }
                    if (g_showTextureNormal)
                    {
                        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
                        normalTextureShader.use();
                        normalTextureShader.setMat4("model", model);
                        normalTextureShader.setBool("bTBN", g_bTBN);
                        renderSphere();
                    }
                }
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Param Setting");
        ImGui::Text("Press \"1\" to show cursor and switch to setting mode.");
        ImGui::Text("Press \"2\" to hide cursor and finishe setting mode.");
        ImGui::Text("Current cursor mode: %d", g_bCaptureCursor ? 2 : 1);
        ImGui::SliderFloat("height map scale", &heightScale, 0.0f, 1.0f);
        ImGui::Checkbox("Show normal for debugging", &g_showNormal);
        if (g_showNormal)
        {
            ImGui::Checkbox("Show vertex normal", &g_showVertexNormal);
            ImGui::Checkbox("Show texture normal", &g_showTextureNormal);
            if (g_showTextureNormal)
                ImGui::Checkbox("TBN Matrix", &g_bTBN);
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
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
        if (tangent.x == 0.0f && tangent.y == 0.0f && tangent.z == 0.0f)
            std::cout << "The tangent is 0.0, index = " << i << endl;

        //if (fabs(glm::dot(normal, tangent)) > 0.01)
        //    std::cout << "glm::dot(normal, tangent) = " << glm::dot(normal, tangent) << endl;
        // Gram-Schmidt orthogonalize
        tangent = glm::normalize(tangent - glm::dot(normal, tangent) * normal);
        //tangent = glm::normalize(tangent);

        glm::vec3 bitangent = glm::cross(normal, tangent);
        bitangents.push_back(bitangent);

        tangents[i] = tangent;
        if (std::isnan(tangent.x) || std::isnan(tangent.y) || std::isnan(tangent.z))
            std::cout << "The tangent is NaN, index = " << i << endl;
        
    }
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount;
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
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        float radius = 1.0;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float phi = xSegment * 2.0f * PI;
                float theta = ySegment * PI;
                float xPos = radius * std::cos(phi) * std::sin(theta);
                float yPos = radius * std::cos(theta);
                float zPos = radius * std::sin(phi) * std::sin(theta);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment * 3, ySegment * 3));
                glm::vec3 normal = glm::vec3(xPos, yPos, zPos);
                normals.push_back(normal);
                //glm::vec3 tangent = glm::vec3(-radius * sin(phi), 0.0f, radius * cos(phi));
                //glm::vec3 tangent = glm::normalize(glm::vec3(-radius * cos(theta) * sin(phi), radius * cos(theta) * cos(phi), 0.0f));
                //std::cout << "glm::dot(normal, tangent) = " << glm::dot(normal, tangent) << endl;
                //tangents.push_back(tangent);
                tangents.push_back(glm::vec3(0.0f));
                //glm::vec3 bitangent = glm::cross(normal, tangent);
                //glm::vec3 bitangent = glm::vec3(-radius * sin(theta) * cos(phi), -radius * sin(theta) * sin(phi), radius * cos(theta));
                //std::cout << "glm::dot(bitangent, tangent) = " << glm::dot(bitangent, tangent) << endl;
                //bitangents.push_back(bitangent);
            }
        }

        //bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            //if (!oddRow) // even rows: y == 0, y == 2; and so on//计算切线需要把该判断删掉，否则有些切线会计算成0
            //{
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            //}
            /*else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;*/
        }
        indexCount = static_cast<unsigned int>(indices.size());

        computeTangents(positions, uv, normals, indices, tangents, bitangents);

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
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
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

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
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
