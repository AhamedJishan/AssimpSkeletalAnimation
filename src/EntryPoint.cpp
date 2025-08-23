#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Mesh.h"
#include "SkinnedMesh.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(int argc, char* argv[])
{
	const char* filename = "Assets/archer/Hip Hop Dancing.dae";

	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!\n" << glfwGetVersionString();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader("Assets/skinned.vert", "Assets/skinned.frag");
	SkinnedMesh* mesh = new SkinnedMesh();
	mesh->LoadMesh(filename);

	glm::mat4 projection = glm::perspective(glm::radians(80.0f), SCREEN_WIDTH / (float)(SCREEN_HEIGHT), 0.1f, 1000.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 15), glm::vec3(0), glm::vec3(0, 1.0f, 0));
	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0, -10, 0));
	model = glm::scale(model, glm::vec3(0.1f));

	int activeBoneId = 0;

	while (!glfwWindowShouldClose(window))
	{
		if(glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();
		shader.SetMat4("uProjection", projection);
		shader.SetMat4("uView", view);
		shader.SetMat4("uModel", model);

		static bool wasPressed;
		bool isPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
		if (!wasPressed && isPressed)
			activeBoneId = (++activeBoneId) % mesh->GetNumBones();
		wasPressed = isPressed;

		shader.SetInt("uActiveBoneId", activeBoneId);

		model = glm::rotate(model, glm::radians(0.05f), glm::vec3(0, 1, 0));

		std::vector <glm::mat4> boneTransforms;
		mesh->GetBoneTransforms(boneTransforms);
		shader.SetMat4s("uBones", boneTransforms);

		mesh->Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;

	return 0;
}