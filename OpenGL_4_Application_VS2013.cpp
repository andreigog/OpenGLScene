//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(glm::vec3(0.0f, 1.0f, 2.5f), glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat cameraSpeed = 0.04f;

GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLfloat lastX = glWindowWidth / 2.0f;
GLfloat lastY = glWindowHeight / 2.0f;
bool firstMouse;

bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle;
GLfloat steeringAngle;

gps::Model3D ground;
gps::Model3D plants;
gps::Model3D boxes;
gps::Model3D ammo;
gps::Model3D barrels;
gps::Model3D chests;
gps::Model3D dog;
gps::Model3D cottage;
gps::Model3D character;
gps::Model3D surround;
gps::Model3D boats;
gps::Model3D sunk;

gps::Model3D chest1;
gps::Model3D chest2;
gps::Model3D chest3;
gps::Model3D sitDog;
gps::Model3D sitMale;
gps::Model3D standMale;
gps::Model3D lightCube;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

std::vector<const GLchar*> faces;

float movX;
float movY;
float movXS;
float movYS;
double lastTimeStamp = glfwGetTime();
float sunkZ;
bool zMov = true;
bool sitting = false;
bool chest1C = true;
bool chest2C = true;
bool chest3C = true;
bool wireFrame = false;
int collectedChests = 0;
float cX;
float cY;
float movement;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (GLfloat)xpos;
		lastY = (GLfloat)ypos;
		firstMouse = false;
	}

	GLfloat xoffset = (GLfloat)xpos - lastX;
	GLfloat yoffset = lastY - (GLfloat)ypos;
	lastX = (GLfloat)xpos;
	lastY = (GLfloat)ypos;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_T]) {
		if (cX > 8.0f)
			movYS += 0.05f;
	}
	if (pressedKeys[GLFW_KEY_G]) {
		if (cX > 8.0f)
			movYS -= 0.05f;
	}
	if (pressedKeys[GLFW_KEY_F]) {
		if (cX > 8.0f)
			movXS += 0.05f;
	}
	if (pressedKeys[GLFW_KEY_H]) {
		if (cX > 8.0f)
			movXS -= 0.05f;
	}
	if (pressedKeys[GLFW_KEY_UP]) {
		cX += 0.05f*glm::sin(glm::radians(steeringAngle));
		cY += 0.05f*glm::cos(glm::radians(steeringAngle));
	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		cX -= 0.05f*glm::sin(glm::radians(steeringAngle));
		cY -= 0.05f*glm::cos(glm::radians(steeringAngle));
	}
	if (pressedKeys[GLFW_KEY_RIGHT]) {
		steeringAngle -= 0.2f;
		if (steeringAngle < 0.0f)
			steeringAngle += 360.0f;
	}
	if (pressedKeys[GLFW_KEY_LEFT]) {
		steeringAngle += 0.2f;
		if (steeringAngle > 360.0f)
			steeringAngle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_P]) {
		if (movYS<-1.5f && movYS>-4.0f && movXS<1.7f && movXS>0.5f)
			sitting = !sitting;
	}

	if (pressedKeys[GLFW_KEY_M]) {
		wireFrame = !wireFrame;
	}


	if (glfwGetKey(glWindow, GLFW_KEY_Q) == GLFW_PRESS) {
		angle += 0.1f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.1f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwMaximizeWindow(glWindow);
	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
						  //glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	lightCube = gps::Model3D("objects/sphere/sphere.obj", "objects/sphere/");
	//lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	plants = gps::Model3D("objects/plants/plants.obj", "objects/plants/");
	boxes = gps::Model3D("objects/boxes/boxes.obj", "objects/boxes/");
	ammo = gps::Model3D("objects/ammo/ammo.obj", "objects/ammo/");
	barrels = gps::Model3D("objects/barrels/barrels.obj", "objects/barrels/");
	chests = gps::Model3D("objects/chests/chests.obj", "objects/chests/");
	dog = gps::Model3D("objects/dog/dog.obj", "objects/dog/");
	cottage = gps::Model3D("objects/cottage/cottage.obj", "objects/cottage/");
	character = gps::Model3D("objects/character/character.obj", "objects/character/");
	surround = gps::Model3D("objects/surround/surround.obj", "objects/surround/");
	boats = gps::Model3D("objects/boats/boats.obj", "objects/boats/");
	sunk = gps::Model3D("objects/sunk/sunk.obj", "objects/sunk/");

	chest1 = gps::Model3D("objects/chest1/chest1.obj", "objects/chest1/");
	chest2 = gps::Model3D("objects/chest2/chest2.obj", "objects/chest2/");
	chest3 = gps::Model3D("objects/chest3/chest3.obj", "objects/chest3/");
	sitDog = gps::Model3D("objects/sitDog/sitDog.obj", "objects/sitDog/");
	sitMale = gps::Model3D("objects/sitMale/sitMale.obj", "objects/sitMale/");
	standMale = gps::Model3D("objects/standMale/standMale.obj", "objects/standMale/");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(3.0f, 3.2f, 2.5f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initSkyboxFaces()
{
	faces.push_back("textures/skybox/right.jpg");
	faces.push_back("textures/skybox/left.jpg");
	faces.push_back("textures/skybox/top.jpg");
	faces.push_back("textures/skybox/bottom.jpg");
	faces.push_back("textures/skybox/back.jpg");
	faces.push_back("textures/skybox/front.jpg");
	mySkyBox.Load(faces);
}

void collectChests() {

	if (movXS > 2.3f && movXS < 4.1f && movYS<-6.8f && movYS>-7.6f) {
		chest1C = false;
		collectedChests++;
	}
	if (movXS > 1.1f && movXS < 1.9f && movYS<-7.2f && movYS>-8.0f) {
		chest2C = false;
		collectedChests++;
	}
	if (movXS > 0.1f && movXS < 0.9f && movYS<-8.6f && movYS>-9.4f) {
		chest3C = false;
		collectedChests++;
	}
}

void renderScene()
{
	if (wireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();

	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	ground.Draw(depthMapShader);
	plants.Draw(depthMapShader);
	boxes.Draw(depthMapShader);
	barrels.Draw(depthMapShader);
	ammo.Draw(depthMapShader);
	cottage.Draw(depthMapShader);
	surround.Draw(depthMapShader);
	boats.Draw(depthMapShader);

	//rotating chests
	collectChests();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.7f, 0.1f, -4.5f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-5.7f, -0.1f, 4.5f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	if (chest1C)
		chest1.Draw(depthMapShader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(4.6f, 0.3f, -5.1f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-4.6f, -0.3f, 5.1f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	if (chest2C)
		chest2.Draw(depthMapShader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.3f, 0.4f, -6.4f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-3.5f, -0.4f, 6.4f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	if (chest3C)
		chest3.Draw(depthMapShader);
	//doggo
	model = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.4f, 5.8f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-7.0f, -0.4f, -5.8f));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	if (!sitting)
		dog.Draw(depthMapShader);

	//floating animation
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));

	float currentTime = glfwGetTime();
	if (currentTime - lastTimeStamp > 1) {
		lastTimeStamp = currentTime;
		zMov = !zMov;
	}
	if (zMov) {
		sunkZ += 0.0009;
		model = glm::translate(model, glm::vec3(0.0f, sunkZ, 0.0f));
	}
	else {
		sunkZ -= 0.0009;
		model = glm::translate(model, glm::vec3(0.0f, sunkZ, 0.0f));
	}
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	sunk.Draw(depthMapShader);
	model = glm::translate(model, glm::vec3(movX, 0.09f, movY));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	
	model = glm::translate(model, glm::vec3(-5.8f + cX, 0.31f, -4.7f + cY));
	model = glm::rotate(model, glm::radians(steeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(5.8f - cX, -0.31f, 4.7f - cY));

	model = glm::translate(model, glm::vec3(cX, 0.09f, cY));
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	if (cX<8.0f)
		character.Draw(depthMapShader);
	else {
		if (!sitting) {
			model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));
			model = glm::translate(model, glm::vec3(movXS, 0.09f, movYS));
			glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
				1,
				GL_FALSE,
				glm::value_ptr(model));
			standMale.Draw(depthMapShader);
		}
		else
		{
			model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));
			glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
				1,
				GL_FALSE,
				glm::value_ptr(model));
			sitMale.Draw(depthMapShader);
			sitDog.Draw(depthMapShader);
		}
	}

	///////////////////////////////////////////////////////////////////
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	//scene
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	ground.Draw(myCustomShader);
	plants.Draw(myCustomShader);
	boxes.Draw(myCustomShader);
	barrels.Draw(myCustomShader);
	ammo.Draw(myCustomShader);
	cottage.Draw(myCustomShader);
	surround.Draw(myCustomShader);
	boats.Draw(myCustomShader);

	//rotating chests
	collectChests();


	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(5.7f, 0.1f, -4.5f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-5.7f, -0.1f, 4.5f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (chest1C)
		chest1.Draw(myCustomShader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(4.6f, 0.3f, -5.1f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-4.6f, -0.3f, 5.1f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (chest2C)
		chest2.Draw(myCustomShader);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.3f, 0.4f, -6.4f));
	model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-3.5f, -0.4f, 6.4f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (chest3C)
		chest3.Draw(myCustomShader);

	//doggo
	model = glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, 0.4f, 5.8f));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-7.0f, -0.4f, -5.8f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	if (!sitting)
		dog.Draw(myCustomShader);

	//floating animation
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));

	currentTime = glfwGetTime();
	if (currentTime - lastTimeStamp > 1) {
		lastTimeStamp = currentTime;
		zMov = !zMov;
	}
	if (zMov) {
		sunkZ += 0.0009;
		model = glm::translate(model, glm::vec3(0.0f, sunkZ, 0.0f));
	}
	else {
		sunkZ -= 0.0009;
		model = glm::translate(model, glm::vec3(0.0f, sunkZ, 0.0f));
	}
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	sunk.Draw(myCustomShader);

	model = glm::translate(model, glm::vec3(-5.8f+cX, 0.31f, -4.7f+cY));
	model = glm::rotate(model, glm::radians(steeringAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(5.8f-cX, -0.31f, 4.7f-cY));

	model = glm::translate(model, glm::vec3(cX, 0.09f, cY));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	if (cX < 8.0f) {
		character.Draw(myCustomShader);
		normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	else {
		if (!sitting) {
			model = glm::translate(glm::mat4(1.0f), glm::vec3(movXS, 0.09f, movYS));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
			standMale.Draw(myCustomShader);
		}
		else
		{
			model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.00001f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
			sitMale.Draw(myCustomShader);
			sitDog.Draw(myCustomShader);
		}
	}

	//draw a white cube around the light

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, lightDir);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(lightShader);

	mySkyBox.Draw(skyboxShader, view, projection);
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();
	initSkyboxFaces();
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
