#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <filesystem>
#include <shader_m.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(Shader &shader, std::wstring text, float x, float y, float scale, glm::vec3 color, FT_Face face);

// fonts
std::string fontFileNameCurrent;
std::vector<std::string> getFileNames(std::string path);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
unsigned int ImGui_Width = 500;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
Character getCharFromFreeType(FT_Face face,
                              FT_ULong c,
                              FT_Int32 load_flags);

std::map<FT_ULong, Character> Characters;
unsigned int VAO, VBO;

// imGui Params
bool g_bCaptureCursor = true;
int g_fileIndex = 0;

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

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // compile and setup the shader
    // ----------------------------
    Shader shader(VERTEX_FILE, FRAGMENT_FILE);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

	// find path to font
    std::vector<std::string> fontFiles = getFileNames(RESOURCES_DIR"/fonts/");
    std::vector<char *> fontFilesChar(fontFiles.size());
    for (int i = 0; i < fontFiles.size(); i++)
        fontFilesChar[i] = const_cast<char *>(fontFiles[i].c_str());
    std::string font_name = "simhei.ttf";
    {
        auto it = std::find(fontFiles.begin(), fontFiles.end(), font_name);
        int index = it == fontFiles.end() ? -1 : std::distance(fontFiles.begin(), it);
        if (font_name.empty() || index < 0)
        {
            std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
            return -1;
        }
        g_fileIndex = index;
        font_name = RESOURCES_DIR"/fonts/" + font_name;
    }
	// load font as face
    FT_Face face = nullptr;
    
    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
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
        // input
        // -----
        processInput(window);

        if (g_fileIndex < fontFiles.size() && fontFileNameCurrent != fontFiles[g_fileIndex])
        {
            fontFileNameCurrent = fontFiles[g_fileIndex];
            font_name = RESOURCES_DIR"/fonts/" + fontFileNameCurrent;
            //std::cout << "Debug: " << fontFileNameCurrent << std::endl;
            if (face != nullptr)
                FT_Done_Face(face);
            Characters.clear();
            if (FT_New_Face(ft, font_name.c_str(), 0, &face))
            {
                std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
                return -1;
            }
            else
            {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, 48);
            }
        }

        // render
        // ------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderText(shader, L"hello world 你好，世界！", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f), face);
        RenderText(shader, L"(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f), face);
       
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Param Setting");
        ImGui::Text("Press \"1\" to show cursor and switch to setting mode.");
        ImGui::Text("Press \"2\" to hide cursor and finishe setting mode.");
        ImGui::Text("Current cursor mode: %d", g_bCaptureCursor ? 2 : 1);
        ImGui::Combo("Font files", &g_fileIndex, fontFilesChar.data(), fontFiles.size());
        if (ImGui::Button("Update file list"))
        {
            fontFiles = getFileNames(RESOURCES_DIR"/fonts/");
            fontFilesChar.resize(fontFiles.size());
            for (int i = 0; i < fontFiles.size(); i++)
                fontFilesChar[i] = const_cast<char *>(fontFiles[i].c_str());
            auto it = std::find(fontFiles.begin(), fontFiles.end(), fontFileNameCurrent);
            if (it != fontFiles.end()) {
                // 找到了目标字符串，it 指向了该字符串的位置
                g_fileIndex = int(std::distance(fontFiles.begin(), it));
                std::cout << "当前选中的字体文件编号： " << g_fileIndex << std::endl;
            } else {
                std::cout << "在当前文件列表中找不到文件： " << fontFileNameCurrent << "， 将会默认选中第一个字体文件" << std::endl;
                g_fileIndex = 0;
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
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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


// render line of text
// -------------------
void RenderText(Shader &shader, std::wstring text, float x, float y, float scale, glm::vec3 color, FT_Face face)
{
    // activate corresponding render state	
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    std::wstring::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        auto it = Characters.find(*c);
        Character ch;
        if (it != Characters.end())
            ch = Characters[*c];
        else
        {
            ch = getCharFromFreeType(face, *c, FT_LOAD_RENDER);
        }

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Character getCharFromFreeType(FT_Face face,
                              FT_ULong c,
                              FT_Int32 load_flags)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (FT_Load_Char(face, c, load_flags))
    {
        std::cout << "ERROR::getCharFromFreeType: Failed to load Glyph" << std::endl;
        return Character();
    }
    // generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // now store character for later use
    Character character = {
        texture,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        static_cast<unsigned int>(face->glyph->advance.x)
    };
    Characters.insert(std::pair<FT_ULong, Character>(c, character));
    return character;
}

std::vector<std::string> getFileNames(std::string path)
{
    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().extension() == ".ttf" ||
            entry.path().extension() == ".ttc" ||
            entry.path().extension() == ".otf" ||
            entry.path().extension() == ".pfb" ||
            entry.path().extension() == ".bdf" ||
            entry.path().extension() == ".pcf" ||
            entry.path().extension() == ".fon" ||
            entry.path().extension() == ".fnt" ||
            entry.path().extension() == ".pfr")
        {
            files.push_back(entry.path().filename().string());
        }
    }

    std::cout << "files in " << path << ":\n";
    /*for (const auto& fileCur : files) {
        std::cout << fileCur << "\n";
    }*/
    return files;
}