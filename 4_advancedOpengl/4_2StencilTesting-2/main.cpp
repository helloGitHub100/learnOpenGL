#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>
#include <model.h>

#include <iostream>
#include "codeConvert.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool g_bCaptureCursor = true;

int main()
{
    float cubeVertices[] = {
        // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  -1.0f,  -1.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  -1.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  -1.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  -1.0f,  -1.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  -1.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  1.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  1.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  1.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  -1.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  1.0f, 1.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  -1.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  -1.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  1.0f, 1.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  1.0f, 1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  -1.0f, -1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  -1.0f, -1.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  1.0f, 1.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  -1.0f,  -1.0f, -1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  -1.0f, -1.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  -1.0f, 1.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  -1.0f, 1.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  -1.0f, 1.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  -1.0f, -1.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  -1.0f,  1.0f, -1.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  1.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  1.0f, 1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  1.0f, 1.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  1.0f, 1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  1.0f, -1.0f,  0.0f, 1.0f
    };

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // build and compile shaders
    // -------------------------
    Shader humanShader(VERTEX_FILE, FRAGMENT_FILE);
    Shader humanShaderSingleColor(VERTEX_SINGLE_COLOR_FILE, FRAGMENT_SINGLE_COLOR_FILE);
    Shader humanShader1(VERTEX_FILE, FRAGMENT_FILE);
    Shader humanShader1SingleColor(VERTEX_SINGLE_COLOR_FILE, FRAGMENT_SINGLE_COLOR_FILE);
    Shader humanShader2(VERTEX_FILE, FRAGMENT_FILE);
    Shader humanShader2SingleColor(VERTEX_SINGLE_COLOR_FILE, FRAGMENT_SINGLE_COLOR_FILE);
    Shader shaderCube(VERTEX_FILE, FRAGMENT_FILE);
    Shader shaderCubeSingleColor(VERTEX_SINGLE_COLOR_FILE, FRAGMENT_SINGLE_COLOR_FILE);

    // load models
    // -----------
    Model humanModel(RESOURCES_MODEL_DIR"/babala/babala.pmx");
    Model humanModelSingleColor(RESOURCES_MODEL_DIR"/babala/babala.pmx");
    Model humanModel1(RESOURCES_MODEL_DIR"/lisa/Lisa.obj");
    Model humanModel1SingleColor(RESOURCES_MODEL_DIR"/lisa/Lisa.obj");
    Model humanModel2(RESOURCES_MODEL_DIR"/jean/jean.pmx");
    Model humanModel2SingleColor(RESOURCES_MODEL_DIR"/jean/jean.pmx");

    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int cubeTexture = loadTexture(RESOURCES_DIR"/textures/marble.jpg");
    
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shaderCube.use();
    shaderCube.setInt("texture_diffuse1", 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    float scale = 0.1f;
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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        // don't forget to enable shader before setting uniforms
        humanShader.use();
        humanShader.setMat4("projection", projection);
        humanShader.setMat4("view", view);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        
        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-7.5f, -12.0f, -30.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        humanShader.setMat4("model", model);
        humanModel.Draw(humanShader.ID);

        humanShaderSingleColor.use();
        humanShaderSingleColor.setMat4("view", view);
        humanShaderSingleColor.setMat4("projection", projection);
        humanShaderSingleColor.setMat4("model", model);
        humanShaderSingleColor.setFloat("scale", scale);

        humanShader1.use();

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        humanShader1.setMat4("projection", projection);
        humanShader1.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(7.5f, -12.0f, -30.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        humanShader1.setMat4("model", model);
        humanModel1.Draw(humanShader1.ID);

        humanShader1SingleColor.use();
        humanShader1SingleColor.setMat4("view", view);
        humanShader1SingleColor.setMat4("projection", projection);
        humanShader1SingleColor.setMat4("model", model);
        humanShader1SingleColor.setFloat("scale", scale);

        humanShader2.use();

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        humanShader2.setMat4("projection", projection);
        humanShader2.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -12.0f, -40.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        humanShader2.setMat4("model", model);
        humanModel2.Draw(humanShader2.ID);

        humanShader2SingleColor.use();
        humanShader2SingleColor.setMat4("view", view);
        humanShader2SingleColor.setMat4("projection", projection);
        humanShader2SingleColor.setMat4("model", model);
        humanShader2SingleColor.setFloat("scale", scale);

        shaderCube.use();
        shaderCube.setMat4("view", view);
        shaderCube.setMat4("projection", projection);
        model = glm::translate(model, glm::vec3(0.0f, 7.0f, 30.0f));
        shaderCube.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        shaderCubeSingleColor.use();
        shaderCubeSingleColor.setMat4("view", view);
        shaderCubeSingleColor.setMat4("projection", projection);
        shaderCubeSingleColor.setMat4("model", model);
        shaderCubeSingleColor.setFloat("scale", scale);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);
        humanShaderSingleColor.use();
        humanModelSingleColor.Draw(humanShaderSingleColor.ID);
        humanShader1SingleColor.use();
        humanModel1SingleColor.Draw(humanShaderSingleColor.ID);
        humanShader2SingleColor.use();
        humanModel2SingleColor.Draw(humanShaderSingleColor.ID);
        shaderCubeSingleColor.use();
        glBindVertexArray(cubeVAO);
        glBindTexture(GL_TEXTURE_2D, cubeTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Param Setting");
        ImGui::Text("Press \"1\" to show cursor and switch to setting mode.");
        ImGui::Text("Press \"2\" to hide cursor and finishe setting mode.");
        ImGui::Text("Current cursor mode: %d", g_bCaptureCursor ? 2 : 1);
        ImGui::SliderFloat("scale", &scale, 0.0f, 2.0f, "%.3f");

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
    glViewport(0, 0, width, height);
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