#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>
#include <learnopengl/model.h>
#include <learnopengl/entity.h>

#ifndef ENTITY_H
#define ENTITY_H

#include <list> //std::list
#include <memory> //std::unique_ptr

class Entity : public Model
{
public:
	list<unique_ptr<Entity>> children;
	Entity* parent;
};
#endif


#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawVisualizerFrustums(Shader* shader);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 1000;
int viewportSeperate = 20;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float cameraNearPlane = 0.1f;
float cameraFarPlane = 500.0f;

// camera 1
Camera camera1(glm::vec3(0.0f, 10.0f, 3.0f));
float lastX1 = (float)SCR_WIDTH / 2.0;
float lastY1 = (float)SCR_HEIGHT / 2.0;
bool firstMouse1 = true;

int g_currentCamera = 0;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH * 2 + viewportSeperate, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	camera.MovementSpeed = 20.f;
	camera1.MovementSpeed = 20.f;

	// build and compile shaders
	// -------------------------
	Shader ourShader(VERTEX_FILE, FRAGMENT_FILE);
	Shader debugFrustumShader(VERTEX_DEBUG_FRUSTUM_FILE, FRAGMENT_DEBUG_FRUSTUM_FILE);

	// load entities
	// -----------
	Model model(RESOURCES_DIR"/objects/planet/planet.obj");
	Entity ourEntity(model);
	ourEntity.transform.setLocalPosition({ 0, 0, 0 });
	const float scale = 1.0;
	ourEntity.transform.setLocalScale({ scale, scale, scale });

	{
		Entity* lastEntity = &ourEntity;

		for (unsigned int x = 0; x < 20; ++x)
		{
			for (unsigned int z = 0; z < 20; ++z)
			{
				ourEntity.addChild(model);
				lastEntity = ourEntity.children.back().get();

				//Set transform values
				lastEntity->transform.setLocalPosition({ x * 10.f - 100.f,  0.f, z * 10.f - 100.f });
			}
		}
	}
	ourEntity.updateSelfAndChild();

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNearPlane, cameraFarPlane);
		const Frustum camFrustum = createFrustumFromCamera(camera, (float)SCR_WIDTH / (float)SCR_HEIGHT, glm::radians(camera.Zoom), cameraNearPlane, cameraFarPlane);

		//static float acc = 0;
		//acc += deltaTime * 0.0001;
		glm::mat4 view = camera.GetViewMatrix();

		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// draw our scene graph
		unsigned int total = 0, display = 0;
		ourEntity.drawSelfAndChild(camFrustum, ourShader, display, total);
		//std::cout << "Total process in CPU : " << total << " / Total send to GPU : " << display << std::endl;

		// camera1 viewport
		glViewport(SCR_WIDTH + viewportSeperate, 0, SCR_WIDTH, SCR_HEIGHT);
		projection = glm::perspective(glm::radians(camera1.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNearPlane, cameraFarPlane);
		view = camera1.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		total = 0;
		display = 0;
		ourEntity.drawSelfAndChild(camFrustum, ourShader, display, total);

		// debug frustum visualization
		debugFrustumShader.use();
		debugFrustumShader.setMat4("projection", projection);
		debugFrustumShader.setMat4("view", view);
		glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawVisualizerFrustums(&debugFrustumShader);
		glDisable(GL_BLEND);

		//ourEntity.transform.setLocalRotation({ 0.f, ourEntity.transform.getLocalRotation().y + 20 * deltaTime, 0.f });
		ourEntity.updateSelfAndChild();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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
	glViewport(0, 0, width, height);
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

    const glm::vec4 color = {1.0, 1.0, 1.0, 0.5f};
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