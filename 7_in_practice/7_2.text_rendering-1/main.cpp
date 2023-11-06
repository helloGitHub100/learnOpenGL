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

#include <locale>
#include <codecvt>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void RenderText(Shader &shader, std::wstring text, float x, float y, float scale, glm::vec3 color, FT_Face face);
void RenderTextSingleTexture(Shader &shader, std::wstring text, int x, int y, float scale, glm::vec3 color, FT_Face face);
std::wstring utf8_to_wstring(const std::string &utf8_str);
std::string wstring_to_utf8(const std::wstring& wstr);
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

struct CharacterBuffer
{
    std::vector<unsigned char> bitmap; // bitmap data
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

struct textTextureInfo
{
    GLuint textureID = 0;
    int width = 0;
    int height = 0;
    int dx = 0;
};

Character getCharFromFreeType(FT_Face face,
                              FT_ULong c,
                              FT_Int32 load_flags);

void addCharBufferMapFromFreeType(FT_Face face,
                                     FT_ULong c,
                                     FT_Int32 load_flags);

std::map<FT_ULong, Character> Characters;
std::map<FT_ULong, CharacterBuffer> CharacterBuffers;
std::map<std::wstring, textTextureInfo> wstringToTextureMap;

unsigned int VAO, VBO;

enum SDF_Mode
{
    SDF_Mode_Close,
    SDF_Mode_Soft_Edges,
    SDF_Mode_Hard_Edges,
    SDF_Mode_Shadow,
    SDF_Mode_Outline
};

// imGui Params
int g_fileIndex = 0;
int g_fontHeight = 48;
int g_fontHeightCur = 0;
float g_fontScale = 1.0f;
float g_fontMatrixScale = 1.0f;
float g_fontMatrixScaleCur = 1.0f;
float g_fontMatrixAngle = 0.0f;
float g_fontMatrixAngleCur = 0.0f;
bool g_bBlend = true;
std::string g_inputText;
glm::vec2 g_minmax1 = glm::vec2(0.4f, 0.5f);
glm::vec2 g_minmax2 = glm::vec2(0.4f, 0.5f);
glm::vec3 g_outlineColor = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec2 g_pixel_offset = glm::vec2(-10.0f, -7.3f);
int g_sdfMode = SDF_Mode_Close;
int g_sdfModeCur = SDF_Mode_Close;
float g_hardEdgeMin = 0.01f;

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
    Shader shaderSDF_SoftEdges(VERTEX_FILE, FRAGMENT_SDF_SOFT_EDGES_FILE);
    Shader shaderSDF_HardEdges(VERTEX_FILE, FRAGMENT_SDF_HARD_EDGES_FILE);
    Shader shaderSDF_Shadow(VERTEX_FILE, FRAGMENT_SDF_SHADOW_FILE);
    Shader shaderSDF_Outline(VERTEX_FILE, FRAGMENT_SDF_OUTLINE_FILE);

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
    g_inputText.resize(256);
    
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
    ImFont *font = io.Fonts->AddFontFromFileTTF(font_name.c_str(), 15, nullptr,
                                                io.Fonts->GetGlyphRangesChineseFull());
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int inputLength = 0;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        if (g_bBlend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);

        if ((g_fileIndex < fontFiles.size() && fontFileNameCurrent != fontFiles[g_fileIndex]) ||
        g_fontHeight != g_fontHeightCur ||
        g_fontMatrixScale != g_fontMatrixScaleCur ||
        g_fontMatrixAngle != g_fontMatrixAngleCur ||
        g_sdfModeCur != g_sdfMode)
        {
            g_fontHeightCur = g_fontHeight;
            fontFileNameCurrent = fontFiles[g_fileIndex];
            g_fontMatrixScaleCur = g_fontMatrixScale;
            g_fontMatrixAngleCur = g_fontMatrixAngle;
            g_sdfModeCur = g_sdfMode;

            font_name = RESOURCES_DIR"/fonts/" + fontFileNameCurrent;
            //std::cout << "Debug: " << fontFileNameCurrent << std::endl;
            if (face != nullptr)
                FT_Done_Face(face);
            Characters.clear();
            CharacterBuffers.clear();
            for (auto it = wstringToTextureMap.begin(); it != wstringToTextureMap.end(); ++it)
            {
                textTextureInfo &texInfo = it->second;
                glDeleteTextures(1, &texInfo.textureID);
            }
            wstringToTextureMap.clear();
            if (FT_New_Face(ft, font_name.c_str(), 0, &face))
            {
                std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
                return -1;
            }
            else
            {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, g_fontHeightCur);
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::scale(model, glm::vec3(g_fontMatrixScaleCur));
                model = glm::rotate(model, glm::radians(g_fontMatrixAngleCur), glm::vec3(0.0f, 0.0f, 1.0f));
                FT_Matrix ftMatrix;
                ftMatrix.xx = (FT_Fixed)(model[0][0] * 0x10000L);
                ftMatrix.xy = (FT_Fixed)(model[0][1] * 0x10000L);
                ftMatrix.yx = (FT_Fixed)(model[1][0] * 0x10000L);
                ftMatrix.yy = (FT_Fixed)(model[1][1] * 0x10000L);
                /*matrix.xx = (1 << 16) * g_fontMatrixScaleCur;  // 设置缩放比例
                matrix.yy = (1 << 16) * g_fontMatrixScaleCur;  // 设置缩放比例
                float radianAngle = g_fontMatrixAngleCur * glm::pi<float>() / 180.0f;
                matrix.xy = -radianAngle * (1 << 16); // 设置旋转角度
                matrix.yx = radianAngle * (1 << 16);*/  // 设置旋转角度
                FT_Set_Transform(face, &ftMatrix, 0);
            }
        }

        // render
        // ------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
        Shader *ShaderTemp = &shader;
        if (g_sdfModeCur == SDF_Mode_Close)
        {
            ShaderTemp = &shader;
            ShaderTemp->use();
        }
        else if (g_sdfModeCur == SDF_Mode_Soft_Edges)
        {
            ShaderTemp = &shaderSDF_SoftEdges;
            ShaderTemp->use();
            ShaderTemp->setVec2("MinMax", g_minmax1);
        }
        else if (g_sdfModeCur == SDF_Mode_Hard_Edges)
        {
            ShaderTemp = &shaderSDF_HardEdges;
            ShaderTemp->use();
            ShaderTemp->setVec2("MinMax", g_minmax1);
            ShaderTemp->setFloat("hardEdgeMin", g_hardEdgeMin);
        }
        else if (g_sdfModeCur == SDF_Mode_Shadow)
        {
            ShaderTemp = &shaderSDF_Shadow;
            ShaderTemp->use();
            ShaderTemp->setVec2("MinMax", g_minmax1);
            ShaderTemp->setVec2("MinMax_shadow", g_minmax2);
            ShaderTemp->setVec2("uv_Offset", g_pixel_offset);
            ShaderTemp->setVec3("shadowColor", g_outlineColor);
        }
        else if (g_sdfModeCur == SDF_Mode_Outline)
        {
            ShaderTemp = &shaderSDF_Outline;
            ShaderTemp->use();
            ShaderTemp->setVec2("outsideMinMax", g_minmax1);
        }
        ShaderTemp->setMat4("projection", projection);

        float t0 = static_cast<float>(glfwGetTime());
        RenderText(*ShaderTemp, L"Oh my god! 你好，世界！", 25.0f, float(g_fontHeightCur), g_fontScale, glm::vec3(0.5, 0.8f, 0.2f), face);
        float t1 = static_cast<float>(glfwGetTime());
        RenderTextSingleTexture(*ShaderTemp, L"Oh my god! 你好，世界！", 25, g_fontHeightCur + g_fontHeightCur * g_fontScale * g_fontMatrixScaleCur, g_fontScale, glm::vec3(0.5, 0.8f, 0.2f), face);
        float t2 = static_cast<float>(glfwGetTime());
        RenderText(*ShaderTemp, L"(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f), face);

        int length = strcspn(g_inputText.c_str(), "\0");
        std::string inputTextTemp = g_inputText.substr(0, length);
        RenderTextSingleTexture(*ShaderTemp, utf8_to_wstring(inputTextTemp), 50, 400, g_fontScale, glm::vec3(0.8, 0.5f, 0.2f), face);
       
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(ImGui_Width - 10, SCR_HEIGHT - 10));
        ImGui::Begin("Param Setting", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
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
        ImGui::SliderInt("Font height", &g_fontHeight, 8, 1000);
        ImGui::SliderFloat("Font scale", &g_fontScale, 0.1f, 20.0f, "%.1f");
        ImGui::SliderFloat("Font transform scale", &g_fontMatrixScale, -20.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("Font transform angle", &g_fontMatrixAngle, -180.0f, 180.0f, "%.1f");
        ImGui::Checkbox("Blend", &g_bBlend);
        ImGui::RadioButton("Normal", &g_sdfMode, SDF_Mode_Close);
        ImGui::RadioButton("Soft edges", &g_sdfMode, SDF_Mode_Soft_Edges);
        ImGui::RadioButton("Hard edges", &g_sdfMode, SDF_Mode_Hard_Edges);
        ImGui::RadioButton("Shadow/Outer glow", &g_sdfMode, SDF_Mode_Shadow);
        ImGui::RadioButton("Outline", &g_sdfMode, SDF_Mode_Outline);
        if (g_sdfModeCur == SDF_Mode_Soft_Edges)
        {
            ImGui::SliderFloat("smoothstep min", &g_minmax1.x, 0.0f, g_minmax1.y, "%.2f");
            ImGui::SliderFloat("smoothstep max", &g_minmax1.y, g_minmax1.x, 1.0f, "%.2f");
        }
        else if (g_sdfModeCur == SDF_Mode_Hard_Edges)
        {
            ImGui::SliderFloat("smoothstep min", &g_minmax1.x, 0.0f, g_minmax1.y, "%.2f");
            ImGui::SliderFloat("smoothstep max", &g_minmax1.y, g_minmax1.x, 1.0f, "%.2f");
            ImGui::SliderFloat("Hard edge min", &g_hardEdgeMin, 0.01f, 1.0f, "%.2f");
        }
        else if (g_sdfModeCur == SDF_Mode_Shadow)
        {
            ImGui::SliderFloat("smoothstep min1", &g_minmax1.x, 0.0f, g_minmax1.y, "%.2f");
            ImGui::SliderFloat("smoothstep max1", &g_minmax1.y, g_minmax1.x, 1.0f, "%.2f");
            ImGui::SliderFloat("smoothstep min2", &g_minmax2.x, 0.0f, g_minmax2.y, "%.2f");
            ImGui::SliderFloat("smoothstep max2", &g_minmax2.y, g_minmax2.x, 1.0f, "%.2f");
            ImGui::SliderFloat2("UV Offset", &g_pixel_offset.x, -100.0f, 100.0f, "%.0f");
            ImGui::ColorEdit3("Shadow color", &g_outlineColor.r);
        }
        else if (g_sdfModeCur == SDF_Mode_Outline)
        {
            ImGui::SliderFloat("step min", &g_minmax1.x, 0.0f, g_minmax1.y, "%.2f");
            ImGui::SliderFloat("step max", &g_minmax1.y, g_minmax1.x, 1.0f, "%.2f");
        }
        
        ImGui::PushID("InputText"); // 将字符串作为ID推入栈
        ImGui::InputText(wstring_to_utf8(L"输入框").c_str(), g_inputText.data(), g_inputText.size());
        ImGui::PopID(); // 弹出ID
        //ImGui::Text(g_inputText.data());//测试输入框的内容
        float dt01 = (t1 - t0) * 100000.0f;
        float dt12 = (t2 - t1) * 100000.0f;
        ImGui::Text("Render Text with a texture %.3f e-05 ms", dt12);
        if (dt12 > 100.0f)
            printf_s("Render Text with a texture %.3f e-05 ms\n", dt12);
        ImGui::Text("Render Text separately %.3f e-05 ms", dt01);
        if (dt01 > 100.0f)
            printf_s("Render Text separately %.3f e-05 ms\n", dt01);
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
    if (text.empty())
        return;
    // activate corresponding render state	
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    for (std::wstring::const_iterator c = text.begin(); c != text.end(); c++) 
    {
        auto it = Characters.find(*c);
        Character ch;
        if (it != Characters.end())
            ch = Characters[*c];
        else
        {
            ch = getCharFromFreeType(face, *c, FT_LOAD_RENDER);
        }
        int xposChar = ch.Bearing.x * scale;
        float xpos = x + xposChar;
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
        int xInterval = (ch.Advance >> 6) * scale * g_fontMatrixScaleCur; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        int dw = w - (xInterval - xposChar);
        if (dw > 0)
            xInterval += dw;
        x += xInterval;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// render line of text
// -------------------
void RenderTextSingleTexture(Shader &shader, std::wstring text, int x, int y, float scale, glm::vec3 color, FT_Face face)
{
    if (text.empty())
        return;
    textTextureInfo &texInfo = wstringToTextureMap[text];
    if (texInfo.textureID == 0 || CharacterBuffers.empty())
    {
        // iterate through all CharacterBuffers
        texInfo.width = 0;
        texInfo.height = 0;
        int advanceX = 0;
        int advanceY = 0;
        for (std::wstring::const_iterator c = text.begin(); c != text.end(); c++) 
        {
            auto it = CharacterBuffers.find(*c);
            if (it == CharacterBuffers.end())
                addCharBufferMapFromFreeType(face, *c, FT_LOAD_RENDER);
            CharacterBuffer &ch = CharacterBuffers[*c];
            int h = ch.Size.y;
            int bottom_h = h - ch.Bearing.y;
            advanceY = glm::max(advanceY, h - bottom_h);
        }
        int xChar = 0;
        std::vector<int> xIntervalVec;
        for (std::wstring::const_iterator c = text.begin(); c != text.end(); c++) 
        {
            CharacterBuffer &ch = CharacterBuffers[*c];
            int w = ch.Size.x;
            int h = ch.Size.y;
            int xposChar = ch.Bearing.x;
            int xpos = xChar + ch.Bearing.x;
            int advanceXtemp = 0;
            if (xpos < 0)
            {
                advanceXtemp = - xpos;
                advanceX += advanceXtemp;
            }
            int bottom_h = h - ch.Bearing.y;
            int ypos = advanceY - h + bottom_h;
            // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            int xInterval = (ch.Advance >> 6) * g_fontMatrixScaleCur;
            int dw = w - (xInterval - xposChar);
            if (dw > 0)
                xInterval += dw;
            //xInterval *= g_fontMatrixScaleCur;
            xIntervalVec.push_back(xInterval);
            xChar += (xInterval + advanceXtemp);
            texInfo.height = glm::max(texInfo.height, ypos + h);
        }
        texInfo.width = xChar;
        texInfo.dx = -advanceX;
        // generate a blank texture
        std::vector<unsigned char> pixels(texInfo.width * texInfo.height);//空白的背景图片
        // 生成一个纹理对象
        if (texInfo.textureID == 0)
            glGenTextures(1, &texInfo.textureID);
        // 绑定生成的纹理对象
        glBindTexture(GL_TEXTURE_2D, texInfo.textureID);
        // 分配内存并传递图像数据
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texInfo.width, texInfo.height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        xChar = advanceX;
        int i = 0;
        for (std::wstring::const_iterator c = text.begin(); c != text.end(); c++, i++) 
        {
            CharacterBuffer &ch = CharacterBuffers[*c];
            int w = ch.Size.x;
            int h = ch.Size.y;
            int xpos = xChar + ch.Bearing.x;
            int bottom_h = h - ch.Bearing.y;
            int ypos = advanceY - h + bottom_h;
            
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            int xInterval = xIntervalVec[i];
            xChar += xInterval;
            glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos, w, h, GL_RED, GL_UNSIGNED_BYTE, ch.bitmap.data());
        }
    }
    float w = texInfo.width * scale;
    float h = texInfo.height * scale;
    x += texInfo.dx * scale;
    float vertices[6][4] = {
            { x,     y + h,   0.0f, 0.0f },            
            { x,     y,       0.0f, 1.0f },
            { x + w, y,       1.0f, 1.0f },

            { x,     y + h,   0.0f, 0.0f },
            { x + w, y,       1.0f, 1.0f },
            { x + w, y + h,   1.0f, 0.0f }           
        };
    // render glyph texture over quad
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texInfo.textureID);
    // render quad
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    if (g_sdfModeCur != SDF_Mode_Close)
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
    
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
        static_cast<unsigned int>(face->glyph->metrics.horiAdvance)
    };
    Characters.insert(std::pair<FT_ULong, Character>(c, character));
    return character;
}

void addCharBufferMapFromFreeType(FT_Face face,
                                     FT_ULong c,
                                     FT_Int32 load_flags)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (FT_Load_Char(face, c, load_flags))
    {
        std::cout << "ERROR::getCharFromFreeType: Failed to load Glyph" << std::endl;
        return;
    }
    if (g_sdfModeCur != SDF_Mode_Close)
        FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF);
    // now store character buffer for later use
    unsigned int bufferSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;
    CharacterBuffer character;
    character.bitmap.resize(bufferSize);
    memcpy(character.bitmap.data(), face->glyph->bitmap.buffer, bufferSize);
    character.Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
    character.Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
    character.Advance = static_cast<unsigned int>(face->glyph->metrics.horiAdvance);

    CharacterBuffers.insert(std::pair<FT_ULong, CharacterBuffer>(c, character));
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

std::wstring utf8_to_wstring(const std::string &utf8_str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(utf8_str);
}

std::string wstring_to_utf8(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}