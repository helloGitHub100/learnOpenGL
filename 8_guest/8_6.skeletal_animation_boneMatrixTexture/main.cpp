#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader_m.h>
#include <camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>



#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float timePerFrame =0.0f;

// animation control
bool g_bStop = false;

bool g_bCaptureCursor = true;
GLFWwindow* window = nullptr;

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
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
	glfwSetKeyCallback(window, keyCallback);

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

	// build and compile shaders
	// -------------------------
	Shader ourShader(VERTEX_ANIMATION_FILE, FRAGMENT_ANIMATION_FILE);

	
	// load models
	// -----------
	Model ourModel(RESOURCES_DIR"/objects/babala/babala.fbx");
	Animation danceAnimation(RESOURCES_DIR"/objects/babala/babala.fbx",&ourModel);
	Animator animator(&danceAnimation);
	int numBones = danceAnimation.GetBoneIDMap().size();

	std::vector<float> pixelData(numBones * 16, 0.f);
	unsigned int boneMatrixTexture;
	glGenTextures(1, &boneMatrixTexture);
	glBindTexture(GL_TEXTURE_2D, boneMatrixTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, numBones, 0, GL_RGBA, GL_FLOAT, NULL);

	ourShader.use();
	ourShader.setInt("boneMatrixImage", 1);
	ourShader.setInt("MAX_BONES", numBones);

	// 计算每帧动画的时间间隔
	timePerFrame = 1.0 / danceAnimation.GetTicksPerSecond();

	// std::vector<vector<glm::mat4>> boneMatricesAllFrames;
	// float t1 = glfwGetTime();
	// animator.getBoneMatricesForAllFrames(boneMatricesAllFrames);
	// float t2 = glfwGetTime();
	// cout << "Time for getBoneMatricesForAllFrames: " << t2 - t1 << endl;
	// int numFrames = (int)danceAnimation.GetDuration();
	// int frameIndex = -1;

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		if (!g_bStop && deltaTime >= 0.0f)
			deltaTime += (currentFrame - lastFrame);

		// input
		// -----
		processInput(window);
		if (deltaTime >= timePerFrame || deltaTime <= -timePerFrame)
		{
			animator.UpdateAnimation(deltaTime);
			deltaTime = 0.0f;
			// ++frameIndex;
			// frameIndex = frameIndex % numFrames;
		}
		lastFrame = currentFrame;
		
		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

        //vector<glm::mat4> &transforms = boneMatricesAllFrames[frameIndex];
		vector<glm::mat4> &transforms = animator.GetFinalBoneMatrices();
		numBones = (int)transforms.size();
		pixelData.resize(numBones * 16);
		ourShader.setInt("MAX_BONES", numBones);
		int elementIndex = 0;
		for (int i = 0; i < transforms.size(); ++i)
		{
			//ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
			glm::mat4 boneMatrix = transforms[i];
			//loop row and column
			for (int j = 0; j < 4; ++j)
			{
				for (int k = 0; k < 4; ++k, ++elementIndex)
				{
					pixelData[elementIndex] = boneMatrix[j][k];
				}
			}
		}
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, boneMatrixTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, numBones, GL_RGBA, GL_FLOAT, &pixelData[0]);


		// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, -1.4f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);


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
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	// "->" next frame
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		deltaTime += timePerFrame;
	// "<-" previous frame
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		deltaTime = -timePerFrame;
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
	if (!g_bCaptureCursor)
        return;
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
	camera.ProcessMouseScroll(yoffset);
}

// keyCallback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//GLFW_KEY_SPACE
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		g_bStop =!g_bStop;
	}
	// "ctrl"
	else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
	{
		g_bCaptureCursor =!g_bCaptureCursor;
		glfwSetInputMode(window, GLFW_CURSOR, g_bCaptureCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		if (g_bCaptureCursor)
			firstMouse = true;
	}
}