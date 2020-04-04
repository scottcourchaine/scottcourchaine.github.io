/*
 * FinalProject.cpp
 *
 *  Created on: Aug 12, 2019
 *      Author: scott
 */

/* Header Inclusions */
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <glm/gtx/string_cast.hpp>

// SOIL Image loader Inclusion
#include "SOIL2/SOIL2.h"

using namespace std; // Standard namespace

#define WINDOW_TITLE "CS 499 - Capstone Enhancement" // Window Title Macro

/* Shader program Macro */
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

/* Variable declarations for shaders, window size initialization, buffer, and array objects */
GLint chairShaderProgram, tableShaderProgram, keyLightShaderProgram, fillLightShaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint ChairVBO, LightVBO, ChairVAO, KeyLightVAO, FillLightVAO, texture;
GLfloat degrees = glm::radians(-45.0f); // Converts float to degrees

// Movement speed per frame
GLfloat keyCameraZoom = 0.005;
GLfloat keyCameraSpeed = 0.05;
GLfloat mouseCameraSpeed = 2.0f;

GLchar currentKey; // Will store key pressed

GLfloat lastX = 400, lastY = 300, lastZ = 0; // Locks mouse cursor at the center of the screen
GLfloat xOffset, yOffset, yaw = 0.0f, pitch = 0.0f; // mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.003f; // Used for mouse / camera rotation sensitivity

bool keyDetected = true; // Initially true when specific key for movement is pressed
bool mouseDetected = true; // Initially true when mouse movement is detected
bool orbit = false; // Variable for orbiting
bool zoom = false; // Variable for zoom speed
bool perspectiveViewOn = true; // Variable for perspective view on

GLfloat camDistance = 15.0f;
GLfloat zoomSpeed = 8.0f; // Variable for zoom speed

// Chair and light color
glm::vec3 objectColor(0.65f, 0.5f, 0.39f);
glm::vec3 keyLightColor(0.52f, 0.37f, 0.26f);
glm::vec3 fillLightColor(1.0f, 1.0f, 1.0f);

// Key Light position and scale
glm::vec3 keyLightPosition(-0.5f, 0.5f, -8.0f);
glm::vec3 keyLightScale(0.3f);

// Fill Light position and scale
glm::vec3 fillLightPosition(-0.5f, -2.5f, 40.0f);
glm::vec3 fillLightScale(0.3f);

// Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f); // Initial camera position.
glm::vec3 CameraUpY = glm::vec3(0.0f, 1.0f, 0.0f); // Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, -0.1f); // Temporary z unit vector
glm::vec3 front; // Temporary z unit vector for mouse

/* Function prototypes */
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UKeyboard(unsigned char key, int x, int y);
void UKeyReleased(unsigned char key, int x, int y);
void UMousePR(int button, int state, int x, int y);
void UMouseMove(int x, int y);
void UGenerateTexture(void);

/* Chair Vertex Shader Source Code */
const GLchar * chairVertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position 0 for vertex position data
		layout (location = 1) in vec3 normal;// VAP position 1 for normals
		layout (location = 2) in vec2 textureCoordinate; // VAP position 2 for textures

		out vec3 Normal;// For outgoing normals to fragment shader
		out vec3 FragmentPos;// For outgoing color / pixels to fragment shader
		out vec2 mobileTextureCoordinate;

		// Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

	void main(){

		gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

		FragmentPos = vec3(model * vec4(position, 1.0f));// Gets fragment / pixel position in world space only (exclude view and projection)

		Normal = mat3(transpose(inverse(model))) * normal;// get normal vectors in world space only and exclude normal translation properties

		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); // Flips the texture horizontally

	}
);

/* Chair Fragment Shader Source Code */
const GLchar * chairFragmentShaderSource = GLSL(330,

		in vec3 Normal; // For incoming normals
		in vec3 FragmentPos;// For incoming fragment position
		in vec2 mobileTextureCoordinate;

		out vec4 chairColor;// For outgoing chair color to the GPU
		out vec4 gpuTexture; // Variable to pass color data to the GPU

		uniform sampler2D uTexture; // Useful when working with multiple textures

		// Uniform / Global variables for object color, key light color, key light position, fill light color, fill light position, and camera/view position
		uniform vec3 objectColor;
		uniform vec3 keyLightColor;
		uniform vec3 keyLightPos;
		uniform vec3 fillLightColor;
		uniform vec3 fillLightPos;
		uniform vec3 viewPosition;

	void main(){

		/* Phong lighting model calculations to generate ambient, diffuse, and specular components */

		// Calculate Ambient key lighting
		float keyAmbientStrength = 0.1f;// Set ambient or global  key lighting strength
		vec3 keyAmbient = keyAmbientStrength * keyLightColor;// Generate ambient key light color

		// Calculate Diffuse key lighting
		vec3 keyNorm = normalize(Normal);// Normalize vectors to 1 unit
		vec3 keyLightDirection = normalize(keyLightPos - FragmentPos);// Calculate distance (key light direction) between light source and fragments/pixels
		float keyImpact = max(dot(keyNorm, keyLightDirection), 2.0);// Calculate diffuse impact by generating dot product of normal and light
		vec3 keyDiffuse = keyImpact * keyLightColor;// Generate diffuse key light color

		// Calculate Specular key lighting
		float keySpecularIntensity = 0.8f;// Set specular key light strength
		float keyHighlightSize = 16.0f;// Set specular highlight size
		vec3 keyViewDir = normalize(viewPosition - FragmentPos);// Calculate view direction
		vec3 keyReflectDir = reflect(-keyLightDirection, keyNorm);// Calculate reflection vector

		// Calculate specular component
		float keySpecularComponent = pow(max(dot(keyViewDir, keyReflectDir), 0.0), keyHighlightSize);
		vec3 keySpecular = keySpecularIntensity * keySpecularComponent * keyLightColor;

		// Calculate phong result
		vec3 keyPhong = (keyAmbient + keyDiffuse + keySpecular) * objectColor;

		// Calculate Ambient fill lighting
		float fillAmbientStrength = 0.1f;// Set ambient or global  fill lighting strength
		vec3 fillAmbient = fillAmbientStrength * fillLightColor;// Generate ambient fill light color

		// Calculate Diffuse fill lighting
		vec3 fillNorm = normalize(Normal);// Normalize vectors to 1 unit
		vec3 fillLightDirection = normalize(fillLightPos - FragmentPos);// Calculate distance (fill light direction) between light source and fragments/pixels
		float fillImpact = max(dot(fillNorm, fillLightDirection), 0.1);// Calculate diffuse impact by generating dot product of normal and light
		vec3 fillDiffuse = fillImpact * fillLightColor;// Generate diffuse fill light color

		// Calculate Specular fill lighting
		float fillSpecularIntensity = 0.8f;// Set specular fill light strength
		float fillHighlightSize = 16.0f;// Set specular highlight size
		vec3 fillViewDir = normalize(viewPosition - FragmentPos);// Calculate view direction
		vec3 fillReflectDir = reflect(-fillLightDirection, fillNorm);// Calculate reflection vector

		// Calculate specular component
		float fillSpecularComponent = pow(max(dot(fillViewDir, fillReflectDir), 0.0), fillHighlightSize);
		vec3 fillSpecular = fillSpecularIntensity * fillSpecularComponent * fillLightColor;

		// Calculate phong result
		vec3 fillPhong = (fillAmbient + fillDiffuse + fillSpecular) * objectColor;

		chairColor = (vec4(keyPhong, 1.0f) * texture(uTexture, mobileTextureCoordinate)) + (vec4(fillPhong, 1.0f) * texture(uTexture, mobileTextureCoordinate)); // Send lighting results to GPU

	}
);

/* Key Light Vertex Shader Source Code */
const GLchar * keyLightVertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

		// Uniform / Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {

			gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices to clip coordinates

		}
);

/* Key Light Fragment Shader Source Code */
const GLchar * keyLightFragmentShaderSource = GLSL(330,

		out vec4 color; // For outgoing key light color to the GPU

		void main() {

			color = vec4(1.0f); // Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0

		}
);

/* Fill Light Vertex Shader Source Code */
const GLchar * fillLightVertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

		// Uniform / Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main() {

			gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices to clip coordinates

		}
);

/* Fill Light Fragment Shader Source Code */
const GLchar * fillLightFragmentShaderSource = GLSL(330,

		out vec4 color; // For outgoing fill light color to the GPU

		void main() {

			color = vec4(1.0f); // Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0

		}
);

/* Main Program */
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW" << std::endl;
			return -1;
		}

	UCreateShader();

	UCreateBuffers();

	UGenerateTexture();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color

	//Set front to avoid initial blank screen
	front.x = camDistance * cos(yaw);
	front.y = camDistance * sin(pitch);
	front.z = camDistance * sin(yaw) * cos(pitch);

	glutDisplayFunc(URenderGraphics);

	glutKeyboardFunc(UKeyboard); // Detects key press

	glutKeyboardUpFunc(UKeyReleased); // Detects key release

	glutMouseFunc(UMousePR); // Press and release key

	glutMotionFunc(UMouseMove); // Detects mouse movement

	glutMainLoop();

	// Destroys Buffer objects once used
	glDeleteVertexArrays(1, &ChairVAO);
	glDeleteVertexArrays(1, &KeyLightVAO);
	glDeleteVertexArrays(1, &FillLightVAO);
	glDeleteBuffers(1, &ChairVBO);
	glDeleteBuffers(1, &LightVBO);

	return 0;
}

/* Resizes the window */
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

/* Renders graphics */
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	//*Camera Movement Logic*/
	if(currentKey == 'w')
		cameraPosition += keyCameraZoom * CameraForwardZ;

	if(currentKey == 's')
		cameraPosition -= keyCameraZoom * CameraForwardZ;

	if(currentKey == 'a')
		cameraPosition -= glm::normalize(glm::cross(CameraForwardZ, CameraUpY)) * keyCameraSpeed;

	if(currentKey == 'd')
		cameraPosition += glm::normalize(glm::cross(CameraForwardZ, CameraUpY)) * keyCameraSpeed;

	CameraForwardZ = front; // Replaces camera forward vector with Radians normalized as a unit vector

	GLint modelLoc, viewLoc, projLoc, objectColorLoc, keyLightColorLoc, keyLightPositionLoc, fillLightColorLoc, fillLightPositionLoc, viewPositionLoc;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	/* Use the Chair Shader and activate the Chair Vertex Array Object for rendering and transforming */
	glUseProgram(chairShaderProgram);
	glBindVertexArray(ChairVAO);

	// Transforms the object
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the object at the center of the viewport
	model = glm::rotate(model, 0.0f, glm::vec3(0.0, 1.0f, 0.0f)); // Rotate the object 45 degrees on the X
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f)); // Increase the object size by a scale of 2

	// Transforms the camera
	view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition,  CameraUpY);

	// Creates a perspective or ortho projection
	if(perspectiveViewOn == true)
	{
		projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
	}

	else
	{
		projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.f);
	}

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(chairShaderProgram, "model");
	viewLoc = glGetUniformLocation(chairShaderProgram, "view");
	projLoc = glGetUniformLocation(chairShaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// References matrix uniforms from the Chair Shader program for the chair color, key light color, key light position, fill light color, fill light position, and camera position
	objectColorLoc = glGetUniformLocation(chairShaderProgram, "objectColor");
	keyLightColorLoc = glGetUniformLocation(chairShaderProgram, "keyLightColor");
	keyLightPositionLoc = glGetUniformLocation(chairShaderProgram, "keyLightPos");
	fillLightColorLoc = glGetUniformLocation(chairShaderProgram, "fillLightColor");
	fillLightPositionLoc = glGetUniformLocation(chairShaderProgram, "fillLightPos");
	viewPositionLoc = glGetUniformLocation(chairShaderProgram, "viewPosition");

	// Pass color, key light, fill light, and camera data to the Chair Shader program's corresponding uniforms
	glUniform3f(objectColorLoc, objectColor.r, objectColor.g, objectColor.b);
	glUniform3f(keyLightColorLoc, keyLightColor.r, keyLightColor.g, keyLightColor.b);
	glUniform3f(keyLightPositionLoc, keyLightPosition.x, keyLightPosition.y, keyLightPosition.z);
	glUniform3f(fillLightColorLoc, fillLightColor.r, fillLightColor.g, fillLightColor.b);
	glUniform3f(fillLightPositionLoc, fillLightPosition.x, fillLightPosition.y, fillLightPosition.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	glutPostRedisplay();

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, 1000);

	glBindVertexArray(0); // Deactivate the Vertex Array Object

	glUseProgram(0);

	/* Use the Key Light Shader and activate the Key Light Vertex Array Object for rendering and transforming */
	glUseProgram(keyLightShaderProgram);
	glBindVertexArray(KeyLightVAO);

	// Transform the key light used as a visual cue for the light source
	model = glm::translate(model, keyLightPosition);
	model = glm::scale(model, keyLightScale);

	// Reference matrix uniforms from the Key Light Shader program
	modelLoc = glGetUniformLocation(keyLightShaderProgram, "model");
	viewLoc = glGetUniformLocation(keyLightShaderProgram, "view");
	projLoc = glGetUniformLocation(keyLightShaderProgram, "projection");

	// Pass matrix data to the Key Light Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glutPostRedisplay(); // Marks the current window to be redisplayed

	glBindTexture(GL_TEXTURE_2D, texture);

	// Next line is commented out so the key light isn't visible
	// glDrawArrays(GL_TRIANGLES, 0, 72); // Draw the primitives / key light

	glBindVertexArray(0); // Deactivate the Key Light Vertex Array Object

	glUseProgram(0);

	/* Use the Fill Light Shader and activate the Fill Light Vertex Array Object for rendering and transforming */
	glUseProgram(fillLightShaderProgram);
	glBindVertexArray(FillLightVAO);

	// Transform the fill light used as a visual cue for the light source
	model = glm::translate(model, fillLightPosition);
	model = glm::scale(model, fillLightScale);

	// Reference matrix uniforms from the Fill Light Shader program
	modelLoc = glGetUniformLocation(fillLightShaderProgram, "model");
	viewLoc = glGetUniformLocation(fillLightShaderProgram, "view");
	projLoc = glGetUniformLocation(fillLightShaderProgram, "projection");

	// Pass matrix data to the Fill Light Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glutPostRedisplay(); // Marks the current window to be redisplayed

	glBindTexture(GL_TEXTURE_2D, texture);

	// Next line is commented out so the fill light isn't visible
	// glDrawArrays(GL_TRIANGLES, 0, 72); // Draw the primitives / fill light

	glBindVertexArray(0); // Deactivate the Fill Light Vertex Array Object

	glUseProgram(0);

	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame. Similar to GL Flush

}

/* Creates the Shader program */
void UCreateShader()
{

	// Chair Vertex shader
	GLint chairVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(chairVertexShader, 1, &chairVertexShaderSource, NULL); // Attaches the Vertex Shader to the source code
	glCompileShader(chairVertexShader); // Compiles the Vertex shader

	// Chair Fragment shader
	GLint chairFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(chairFragmentShader, 1, &chairFragmentShaderSource, NULL); // Attaches the Fragment Shader to the source code
	glCompileShader(chairFragmentShader); // Compiles the Fragment shader

	// Chair Shader program
	chairShaderProgram = glCreateProgram(); // Creates the Shader and returns an id
	glAttachShader(chairShaderProgram, chairVertexShader); // Attach Vertex Shader to the Shader program
	glAttachShader(chairShaderProgram, chairFragmentShader); // Attach Fragment shader to the Shader program
	glLinkProgram(chairShaderProgram); // Link Vertex and Fragment shaders to Shader program

	// Delete the chair shaders once linked
	glDeleteShader(chairVertexShader);
	glDeleteShader(chairFragmentShader);

	// Key Light Vertex shader
	GLint keyLightVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(keyLightVertexShader, 1, &keyLightVertexShaderSource, NULL); // Attaches the Vertex shader to the source code
	glCompileShader(keyLightVertexShader); // Compiles the Vertex shader

	// Key Light Fragment shader
	GLint keyLightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(keyLightFragmentShader, 1, &keyLightFragmentShaderSource, NULL); // Attaches the Fragment shader source to the source code
	glCompileShader(keyLightFragmentShader); // Compiles the Fragment shader

	// Key Light Shader program
	keyLightShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(keyLightShaderProgram, keyLightVertexShader); // Attach Vertex shader to the Shader program
	glAttachShader(keyLightShaderProgram, keyLightFragmentShader); // Attach Fragment shader to the Shader program
	glLinkProgram(keyLightShaderProgram); // Link Vertex and Fragment shaders to Shader program

	// Delete the key light shaders once linked
	glDeleteShader(keyLightVertexShader);
	glDeleteShader(keyLightFragmentShader);

	// Fill Light Vertex shader
	GLint fillLightVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(fillLightVertexShader, 1, &fillLightVertexShaderSource, NULL); // Attaches the Vertex shader to the source code
	glCompileShader(fillLightVertexShader); // Compiles the Vertex shader

	// Fill Light Fragment shader
	GLint fillLightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(fillLightFragmentShader, 1, &fillLightFragmentShaderSource, NULL); // Attaches the Fragment shader source to the source code
	glCompileShader(fillLightFragmentShader); // Compiles the Fragment shader

	// Fill Light Shader program
	fillLightShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(fillLightShaderProgram, fillLightVertexShader); // Attach Vertex shader to the Shader program
	glAttachShader(fillLightShaderProgram, fillLightFragmentShader); // Attach Fragment shader to the Shader program
	glLinkProgram(fillLightShaderProgram); // Link Vertex and Fragment shaders to Shader program

	// Delete the fill light shaders once linked
	glDeleteShader(fillLightVertexShader);
	glDeleteShader(fillLightFragmentShader);

}

void UCreateBuffers()
{

	GLfloat vertices[] = {
			// Positions          // Normals          // Textures

			// Chair Seat Bottom
								  // Neg. Z
			-0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			 0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
			 0.5f,  0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			 0.5f,  0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			-0.5f,  0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
			-0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

								  // Pos. Z
			-0.5f, -0.1f,  0.5f,  0.0f,  1.0f,  1.0f,  1.0f,  0.0f,
			 0.5f, -0.1f,  0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.1f,  0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  1.0f,
			 0.5f,  0.1f,  0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  1.0f,
			-0.5f,  0.1f,  0.5f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,
			-0.5f, -0.1f,  0.5f,  0.0f,  1.0f,  1.0f,  1.0f,  0.0f,

								  // Neg. X
			-0.5f,  0.1f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f,  0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.1f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,  0.1f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

								  // Pos. X
			 0.5f,  0.1f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.5f,  0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f, -0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f, -0.1f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.1f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

			 	 	 	 	 	  // Neg. Y
			-0.5f, -0.1f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.1f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -0.1f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -0.1f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.1f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -0.1f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			//Chair Seat Top
									   // Neg. Z
			-0.505f, 0.1f,   -0.505f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.505f, 0.1f,   -0.505f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.505f, 0.105f, -0.505f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.505f, 0.105f, -0.505f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.505f, 0.105f, -0.505f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.505f, 0.1f,   -0.505f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

									  // Pos. Z
			-0.505f, 0.1f,   0.505f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.505f, 0.1f,   0.505f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.505f, 0.105f, 0.505f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.505f, 0.105f, 0.505f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.505f, 0.105f, 0.505f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.505f, 0.1f,   0.505f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,

									   // Neg. X
			-0.505f, 0.105f,  0.505f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.505f, 0.105f, -0.505f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.505f, 0.1f,   -0.505f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.505f, 0.1f,   -0.505f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.505f, 0.1f,    0.505f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.505f, 0.105f,  0.505f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

									   // Pos. X
			 0.505f, 0.105f,  0.505f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.505f, 0.105f, -0.505f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.505f, 0.1f,   -0.505f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.505f, 0.1f,   -0.505f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.505f, 0.1f,    0.505f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.505f, 0.105f,  0.505f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

			 	 	 	 	 	 	   // Pos. Y
			-0.505f, 0.105f, -0.505f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			 0.505f, 0.105f, -0.505f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.505f, 0.105f,  0.505f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.505f, 0.105f,  0.505f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			-0.505f, 0.105f,  0.505f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			-0.505f, 0.105f, -0.505f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,

									// Neg. Y
			-0.505f, 0.1f, 0.5f,    0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.505f, 0.1f, 0.5f,    0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.505f, 0.1f, 0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.505f, 0.1f, 0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.505f, 0.1f, 0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.505f, 0.1f, 0.5f,    0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

									 // Neg. Y
			-0.505f, 0.1f, -0.5f,    0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			 0.505f, 0.1f, -0.5f,    0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			-0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			-0.505f, 0.1f, -0.5f,    0.0f, -1.0f,  0.0f,  0.0f,  0.0f,

									 // Neg. Y
			-0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			-0.5f,   0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f,   0.1f,  0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f,   0.1f,  0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.505f, 0.1f,  0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			-0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,

									 // Neg. Y
			 0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f,   0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f,   0.1f,  0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f,   0.1f,  0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.505f, 0.1f,  0.505f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			 0.505f, 0.1f, -0.505f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,

			// Chair Leg 1  (Front Right)
			 	 	 	 	 	 // Neg. Z
			0.4f, -1.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			0.5f, -1.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			0.4f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			0.4f, -1.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

								 // Pos. Z
			0.4f, -1.1f, -0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			0.5f, -1.1f, -0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			0.5f, -0.1f, -0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			0.5f, -0.1f, -0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			0.4f, -0.1f, -0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			0.4f, -1.1f, -0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,

								 // Neg. X
			0.4f, -1.1f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.4f, -1.1f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.4f, -0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.4f, -0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.4f, -0.1f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.4f, -1.1f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

								 // Pos. X
			0.5f, -1.1f, -0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.5f, -1.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.5f, -0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.5f, -0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.5f, -0.1f, -0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.5f, -1.1f, -0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

								 // Neg. Y
			0.4f, -1.1f, -0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  1.0f,
			0.5f, -1.1f, -0.5f,  0.0f,  1.0f,  1.0f,  1.0f,  1.0f,
			0.5f, -1.1f, -0.4f,  0.0f,  1.0f,  1.0f,  1.0f,  0.0f,
			0.5f, -1.1f, -0.4f,  0.0f,  1.0f,  1.0f,  1.0f,  0.0f,
			0.4f, -1.1f, -0.4f,  0.0f,  1.0f,  1.0f,  0.0f,  0.0f,
			0.4f, -1.1f, -0.5f,  0.0f,  1.0f,  1.0f,  0.0f,  1.0f,

			// Chair Leg 2 (Front Left)
								  // Neg. Z
			-0.4f, -1.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			-0.5f, -1.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.4f, -0.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.4f, -1.1f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,

								  // Pos. Z
			-0.4f, -1.1f, -0.4f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			-0.5f, -1.1f, -0.4f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.5f, -0.1f, -0.4f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, -0.4f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.4f, -0.1f, -0.4f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.4f, -1.1f, -0.4f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

								  // Neg. X
			-0.5f, -1.1f, -0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -1.1f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -1.1f, -0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

								  // Pos. X
			-0.4f, -1.1f, -0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, -1.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.4f, -0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.4f, -0.1f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.4f, -0.1f, -0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.4f, -1.1f, -0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

								  // Neg. Y
			-0.4f, -1.1f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -1.1f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -1.1f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -1.1f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.4f, -1.1f, -0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, -1.1f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,

			// Chair Leg 3 (Back Left)
								 // Neg. Z
			-0.4f, -1.1f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			-0.5f, -1.1f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.1f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, -0.1f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.4f, -0.1f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.4f, -1.1f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,

								 // Pos. Z
			-0.4f, -1.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			-0.5f, -1.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.5f, -0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.4f, -0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.4f, -1.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

								 // Neg. X
			-0.5f, -1.1f, 0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -1.1f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, -0.1f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -0.1f, 0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -1.1f, 0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

								 // Pos. X
			-0.4f, -1.1f, 0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, -1.1f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.4f, -0.1f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.4f, -0.1f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.4f, -0.1f, 0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.4f, -1.1f, 0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

								 // Neg. Y
			-0.4f, -1.1f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, -1.1f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, -1.1f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, -1.1f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.4f, -1.1f, 0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, -1.1f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,

			// Chair Leg 4 (Back Right)
								 // Neg. Z
			 0.4f, -1.1f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.5f, -1.1f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.5f, -0.1f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.5f, -0.1f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.4f, -0.1f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			 0.4f, -1.1f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

			 	 	 	 	 	 // Pos. Z
			 0.4f, -1.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.5f, -1.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.5f, -0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.4f, -0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.4f, -1.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,

			 	 	 	 	 	 // Neg. X
			 0.4f, -1.1f, 0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.4f, -1.1f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.4f, -0.1f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.4f, -0.1f, 0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.4f, -0.1f, 0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.4f, -1.1f, 0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

			 	 	 	 	 	 // Pos. X
			 0.5f, -1.1f, 0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -1.1f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.5f, -0.1f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.1f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, -0.1f, 0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -1.1f, 0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

			 	 	 	 	 	 // Neg. Y
			 0.4f, -1.1f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			 0.5f, -1.1f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, -1.1f, 0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, -1.1f, 0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.4f, -1.1f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.4f, -1.1f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,

			// Chair Leg 2 & Leg 3 Support
			 	 	 	 	 	   // Neg. X
			-0.49f, -0.4f,  0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.49f, -0.4f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.49f, -0.3f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.49f, -0.3f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.49f, -0.3f,  0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.49f, -0.4f,  0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

								   // Pos. X
			-0.41f, -0.4f,  0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.41f, -0.4f, -0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.41f, -0.3f, -0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.41f, -0.3f, -0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.41f, -0.3f,  0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.41f, -0.4f,  0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

								   // Neg. Y
			-0.49f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			-0.41f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			-0.41f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.41f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.49f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.49f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

								   // Pos. Y
			-0.49f, -0.3f, -0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.41f, -0.3f, -0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-0.41f, -0.3f,  0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			-0.41f, -0.3f,  0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			-0.49f, -0.3f,  0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			-0.49f, -0.3f, -0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,

			// Chair Leg 1 & Leg 4 Support
								  // Neg. X
			0.41f, -0.4f,  0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.41f, -0.4f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.41f, -0.3f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.41f, -0.3f, -0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.41f, -0.3f,  0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.41f, -0.4f,  0.4f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

								  // Pos. X
			0.49f, -0.4f,  0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.49f, -0.4f, -0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.49f, -0.3f, -0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.49f, -0.3f, -0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.49f, -0.3f,  0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.49f, -0.4f,  0.4f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

								  // Neg. Y
			0.49f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			0.41f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			0.41f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			0.41f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			0.49f, -0.4f,  0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			0.49f, -0.4f, -0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,

								  // Pos. Y
			0.49f, -0.3f, -0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			0.41f, -0.3f, -0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			0.41f, -0.3f,  0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			0.41f, -0.3f,  0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			0.49f, -0.3f,  0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			0.49f, -0.3f, -0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,

			//Chair Back-Support 1
								 // Neg. Z
			0.4f, 1.5f,   0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			0.5f, 1.5f,   0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			0.5f, 0.105f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			0.5f, 0.105f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			0.4f, 0.105f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			0.4f, 1.5f,   0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

							   // Pos. Z
			0.4f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			0.5f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			0.5f, 0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			0.5f, 0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			0.4f, 0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			0.4f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,

								 // Neg. X
			0.4f, 1.5f,   0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.4f, 1.5f,   0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.4f, 0.105f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.4f, 0.105f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.4f, 0.105f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.4f, 1.5f,   0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

							   // Pos. X
			0.5f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			0.5f, 1.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			0.5f, 0.1f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.5f, 0.1f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			0.5f, 0.1f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			0.5f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

							   // Pos. Y
			0.4f, 1.5f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			0.5f, 1.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			0.5f, 1.5f, 0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			0.5f, 1.5f, 0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			0.4f, 1.5f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			0.4f, 1.5f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,

			// Chair Back-Support 2
								  // Neg. Z
			-0.4f, 1.5f,   0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.5f, 1.5f,   0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, 0.105f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			-0.5f, 0.105f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			-0.4f, 0.105f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			-0.4f, 1.5f,   0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,

								// Pos. Z
			-0.4f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.5f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f, 0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.5f, 0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.4f, 0.1f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			-0.4f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,

								// Neg. X
			-0.5f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, 1.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, 0.1f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, 0.1f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, 0.1f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

								  // Pos. X
			-0.4f, 1.5f,   0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.4f, 1.5f,   0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.4f, 0.105f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, 0.105f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, 0.105f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.4f, 1.5f,   0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

								// Pos. Y
			-0.4f, 1.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, 1.5f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, 1.5f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, 1.5f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			-0.4f, 1.5f, 0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			-0.4f, 1.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,

			// Chair Back Headrest
								 // Neg. Z
			-0.55f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.1f,  0.0f,
			 0.55f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.9f,  0.0f,
			 0.6f,  1.8f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.6f,  1.8f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.6f,  1.8f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.55f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.1f,  0.0f,

								 // Pos. Z
			-0.55f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.9f,  0.0f,
			 0.55f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.1f,  0.0f,
			 0.6f,  1.8f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.6f,  1.8f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.6f,  1.8f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.55f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.9f,  0.0f,

								 // Neg. X and Neg. Y
			-0.6f,  1.8f, 0.5f, -0.05f, -0.3f,  0.0f,  0.0f,  1.0f,
			-0.6f,  1.8f, 0.4f, -0.05f, -0.3f,  0.0f,  1.0f,  1.0f,
			-0.55f, 1.5f, 0.4f, -0.05f, -0.3f,  0.0f,  1.0f,  0.0f,
			-0.55f, 1.5f, 0.4f, -0.05f, -0.3f,  0.0f,  1.0f,  0.0f,
			-0.55f, 1.5f, 0.5f, -0.05f, -0.3f,  0.0f,  0.0f,  0.0f,
			-0.6f,  1.8f, 0.5f, -0.05f, -0.3f,  0.0f,  0.0f,  1.0f,

								 // Pos. X and Neg. Y
			 0.6f,  1.8f, 0.5f,  0.05f,  -0.3f,  0.0f,  1.0f,  1.0f,
			 0.6f,  1.8f, 0.4f,  0.05f,  -0.3f,  0.0f,  0.0f,  1.0f,
			 0.55f, 1.5f, 0.4f,  0.05f,  -0.3f,  0.0f,  0.0f,  0.0f,
			 0.55f, 1.5f, 0.4f,  0.05f,  -0.3f,  0.0f,  0.0f,  0.0f,
			 0.55f, 1.5f, 0.5f,  0.05f,  -0.3f,  0.0f,  1.0f,  0.0f,
			 0.6f,  1.8f, 0.5f,  0.05f,  -0.3f,  0.0f,  1.0f,  1.0f,

			 	 	 	 	 	// Pos. Y
			-0.6f, 1.8f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			 0.6f, 1.8f, 0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.6f, 1.8f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.6f, 1.8f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			-0.6f, 1.8f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			-0.6f, 1.8f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,

								 // Neg. Y
			-0.55f, 1.5f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.55f, 1.5f, 0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.55f, 1.5f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.55f, 1.5f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.55f, 1.5f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.55f, 1.5f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

			// Chair Middle Back-Support
								// Neg. Z
			-0.5f, 0.4f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.5f, 0.4f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.5f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.5f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.5f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.5f, 0.4f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

								// Pos. Z
			-0.5f, 0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.5f, 0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.5f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.5f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.5f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.5f, 0.4f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,

								// Pos. Y
			-0.5f, 0.5f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
			 0.5f, 0.5f, 0.4f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, 0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, 0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
			-0.5f, 0.5f, 0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
			-0.5f, 0.5f, 0.4f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,

								// Neg. Y
			-0.5f, 0.4f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
			 0.5f, 0.4f, 0.4f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
			 0.5f, 0.4f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			 0.5f, 0.4f, 0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
			-0.5f, 0.4f, 0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
			-0.5f, 0.4f, 0.4f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,


			// Chair Vertical Back-Support 1
								// Neg. Z
			 0.2f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			 0.3f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 	 0.3f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.3f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.2f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			 0.2f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

			 	 	 	 	 	// Pos. Z
			 0.2f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.3f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.3f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.3f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.2f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			 0.2f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,

			 	 	 	 	 	// Neg. X
			 0.2f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.2f, 1.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.2f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.2f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.2f, 0.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.2f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

			 	 	 	 	 	// Pos. X
			 0.3f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.3f, 1.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.3f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.3f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.3f, 0.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.3f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

			// Chair Vertical Back-Support 2
			 	 	 	 	 	 // Neg. Z
			-0.05f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			 0.05f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			 0.05f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			 0.05f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			-0.05f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			-0.05f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

								 // Pos. Z
			-0.05f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			 0.05f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			 0.05f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			 0.05f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			-0.05f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.05f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,

								 // Neg. X
			-0.05f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.05f, 1.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.05f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.05f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.05f, 0.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.05f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

								 // Pos. X
			 0.05f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			 0.05f, 1.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			 0.05f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.05f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			 0.05f, 0.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			 0.05f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

			// Chair Vertical Back-Support 3
			 	 	 	 	 	// Neg. Z
			-0.3f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
			-0.2f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
			-0.2f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			-0.2f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
			-0.3f, 0.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
			-0.3f, 1.5f, 0.4f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

								// Pos. Z
			-0.2f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
			-0.3f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
			-0.3f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.3f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
			-0.2f, 0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
			-0.2f, 1.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,

								// Neg. X
			-0.3f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.3f, 1.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.3f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.3f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.3f, 0.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.3f, 1.5f, 0.4f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,

								// Pos. X
			-0.2f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
			-0.2f, 1.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
			-0.2f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.2f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
			-0.2f, 0.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
			-0.2f, 1.5f, 0.4f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,

			// TABLE
			// Back of table top
			-1.0f, 0.65f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			-1.0f, 0.5f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			 1.0f, 0.5f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			 1.0f, 0.65f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			-1.0f, 0.65f, -1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

			// Pos. X facing side of table top
			 1.0f, 0.65f, -3.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, 0.65f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			 1.0f, 0.5f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, -3.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			 1.0f, 0.65f, -3.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Front of table top
			-1.0f, 0.65f, -3.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
			-1.0f, 0.5f, -3.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			 1.0f, 0.5f, -3.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, -3.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, 0.65f, -3.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			-1.0f, 0.65f, -3.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

			// Neg. X facing side of table top
			-1.0f, 0.65f, -3.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, 0.65f, -1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-1.0f, 0.5f, -3.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, 0.5f, -3.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, 0.5f, -1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-1.0f, 0.65f, -1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Top of table top
			-1.0f, 0.65f, -3.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, 0.65f, -1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, 0.65f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
			 1.0f, 0.65f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
			 1.0f, 0.65f, -3.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
			-1.0f, 0.65f, -3.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

			// Bottom of table top
			-1.0f, 0.5f, -3.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, 0.5f, -1.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			 1.0f, 0.5f, -1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, 0.5f, -1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, 0.5f, -3.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			-1.0f, 0.5f, -3.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,

			// Table leg 1 back
			-0.95f,  0.5f,  -1.05f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			-0.95f, -1.08f, -1.05f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -1.05f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			-0.75f, -1.08f, -1.05f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			-0.75f,  0.5f,  -1.05f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			-0.95f,  0.5f,  -1.05f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

			// Table leg 1 Neg. X facing side
			-0.95f,  0.5f,  -1.25f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-0.95f, -1.08f, -1.25f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-0.95f, -1.08f, -1.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-0.95f, -1.08f, -1.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-0.95f,  0.5f,  -1.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-0.95f,  0.5f,  -1.25f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

			// Table leg 1 front
			-0.95f,  0.5f,  -1.25f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
			-0.75f,  0.5f,  -1.25f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			-0.75f, -1.08f, -1.25f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -1.25f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			-0.95f, -1.08f, -1.25f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			-0.95f,  0.5f,  -1.25f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

			// Table leg 1 Pos. X facing side
			-0.75f,  0.5f,  -1.05f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-0.75f, -1.08f, -1.05f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-0.75f,  0.5f,  -1.25f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-0.75f,  0.5f,  -1.25f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-0.75f, -1.08f, -1.25f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-0.75f, -1.08f, -1.05f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

			// Table leg 1 bottom
			-0.95f, -1.08f, -1.05f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			-0.95f, -1.08f, -1.25f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -1.05f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			-0.75f, -1.08f, -1.05f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			-0.75f, -1.08f, -1.25f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			-0.95f, -1.08f, -1.25f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,

			// Table leg 2 back
			-0.95f,  0.5f,  -2.75f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			-0.95f, -1.08f, -2.75f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -2.75f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			-0.75f, -1.08f, -2.75f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			-0.75f,  0.5f,  -2.75f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			-0.95f,  0.5f,  -2.75f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

			// Table leg 2 Neg. X facing side
			-0.95f,  0.5f,  -2.75f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-0.95f, -1.08f, -2.75f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-0.95f, -1.08f, -2.95f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-0.95f, -1.08f, -2.95f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-0.95f,  0.5f,  -2.95f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-0.95f,  0.5f,  -2.75f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Table leg 2 front
			-0.95f,  0.5f,  -2.95f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
			-0.95f, -1.08f, -2.95f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			-0.75f, -1.08f, -2.95f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -2.95f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			-0.75f,  0.5f,  -2.95f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			-0.95f,  0.5f,  -2.95f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

			// Table leg 2 Pos. X facing side
			-0.75f,  0.5f,  -2.75f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-0.75f,  0.5f,  -2.95f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-0.75f, -1.08f, -2.95f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-0.75f, -1.08f, -2.95f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-0.75f, -1.08f, -2.75f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-0.75f,  0.5f,  -2.75f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

			// Table leg 2 bottom
			-0.95f, -1.08f, -2.75f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			-0.95f, -1.08f, -2.95f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -2.75f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			-0.75f, -1.08f, -2.75f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			-0.95f, -1.08f, -2.95f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			-0.75f, -1.08f, -2.95f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,

			// Table leg 3 back
			0.75f,  0.5f,  -2.75f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			0.75f, -1.08f, -2.75f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -2.75f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			0.95f, -1.08f, -2.75f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			0.95f,  0.5f,  -2.75f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			0.75f,  0.5f,  -2.75f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

			// Table leg 3 Neg. X facing side
			0.75f,  0.5f,  -2.95f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			0.75f,  0.5f,  -2.75f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			0.75f, -1.08f, -2.95f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.75f, -1.08f, -2.95f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.75f, -1.08f, -2.75f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			0.75f,  0.5f,  -2.75f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Table leg 3 front
			0.75f,  0.5f,  -2.95f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
			0.95f,  0.5f,  -2.95f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			0.95f, -1.08f, -2.95f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -2.95f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			0.75f, -1.08f, -2.95f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			0.75f,  0.5f,  -2.95f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

			// Table leg 3 Pos. X facing side
			0.95f,  0.5f,  -2.75f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			0.95f,  0.5f,  -2.95f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			0.95f, -1.08f, -2.75f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -2.75f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -2.95f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			0.95f,  0.5f,  -2.95f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Table leg 3 bottom
			0.75f, -1.08f, -2.75f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			0.75f, -1.08f, -2.95f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -2.75f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			0.95f, -1.08f, -2.75f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			0.95f, -1.08f, -2.95f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			0.75f, -1.08f, -2.95f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,

			// Table leg 4 back
			0.75f,  0.5f,  -1.05f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			0.75f, -1.08f, -1.05f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			0.95f,  0.5f,  -1.05f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			0.95f,  0.5f,  -1.05f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			0.95f, -1.08f, -1.05f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			0.75f, -1.08f, -1.05f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,

			// Table leg 4 Neg. X facing side
			0.75f,  0.5f,  -1.25f,  -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			0.75f,  0.5f,  -1.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			0.75f, -1.08f, -1.25f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.75f, -1.08f, -1.25f,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.75f, -1.08f, -1.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			0.75f,  0.5f,  -1.05f,  -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Table leg 4 front
			0.75f,  0.5f,  -1.25f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
			0.75f, -1.08f, -1.25f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			0.95f, -1.08f, -1.25f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -1.25f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			0.95f,  0.5f,  -1.25f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			0.75f,  0.5f,  -1.25f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

			// Table leg 4 Pos. X facing side
			0.95f,  0.5f,  -1.05f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			0.95f,  0.5f,  -1.25f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			0.95f, -1.08f, -1.05f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -1.05f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -1.25f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			0.95f,  0.5f,  -1.25f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Table leg 4 bottom
			0.75f, -1.08f, -1.05f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			0.75f, -1.08f, -1.25f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			0.95f, -1.08f, -1.25f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			0.95f, -1.08f, -1.25f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			0.95f, -1.08f, -1.05f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			0.75f, -1.08f, -1.05f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f

	};

	// Generate buffer id for chair
	glGenVertexArrays(1, &ChairVAO); // Vertex Array Object for Chair vertices
	glGenBuffers(1, &ChairVBO);

	// Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers.
	glBindVertexArray(ChairVAO);

	// Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, ChairVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO

	// Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) 0);
	glEnableVertexAttribArray(0); // Enables vertex attribute

	// Set attribute pointer 1 to hold Normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Set attribute pointer 2 to hold Texture data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Deactivates the VAO which is good practice


	GLfloat lightVertices[] = {
			// Positions          // Normals          // Textures
			// Front of cube
			-1.0f, 1.5f, 2.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
			-1.0f, 0.5f, 2.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, 2.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			 1.0f, 0.5f, 2.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
			 1.0f, 1.5f, 2.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
			-1.0f, 1.5f, 2.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,

			// Pos. X facing side of cube
			 1.0f, 1.5f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, 1.5f, 2.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			 1.0f, 0.5f, 2.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, 2.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			 1.0f, 1.5f, 1.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Back of cube
			-1.0f, 1.5f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
			-1.0f, 0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
			 1.0f, 0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, 0.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, 1.5f, 1.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
			-1.0f, 1.5f, 1.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,

			// Neg. X facing side of cube
			-1.0f, 1.5f, 1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, 1.5f, 2.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
			-1.0f, 0.5f, 1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, 0.5f, 1.0f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, 0.5f, 2.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
			-1.0f, 1.5f, 2.0f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,

			// Top of cube
			-1.0f, 1.5f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, 1.5f, 2.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
			 1.0f, 1.5f, 2.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
			 1.0f, 1.5f, 2.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
			 1.0f, 1.5f, 1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
			-1.0f, 1.5f, 1.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,

			// Bottom of cube
			-1.0f, 0.5f, 1.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			-1.0f, 0.5f, 2.0f,  0.0f, -1.0f, 0.0f,  0.0f, 1.0f,
			 1.0f, 0.5f, 2.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, 0.5f, 2.0f,  0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
			 1.0f, 0.5f, 1.0f,  0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
			-1.0f, 0.5f, 1.0f,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f
	};

	// Generate buffer id for key light
	glGenVertexArrays(1, &KeyLightVAO); // Vertex Array Object for vertex copies to serve as key light source
	glGenBuffers(1, &LightVBO);

	// Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers.
	glBindVertexArray(KeyLightVAO);

	// Referencing the same VBO for its vertices
	glBindBuffer(GL_ARRAY_BUFFER, LightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW); // Copy vertices to VBO

	// Set attribute pointer 0 to hold Position data (used for the key light)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Deactivates the VAO which is good practice

	// Generate buffer id for fill light
	glGenVertexArrays(1, &FillLightVAO); // Vertex Array Object for vertex copies to serve as fill light source
	glGenBuffers(1, &LightVBO);

	// Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers.
	glBindVertexArray(FillLightVAO);

	// Referencing the same VBO for its vertices
	glBindBuffer(GL_ARRAY_BUFFER, LightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices, GL_STATIC_DRAW); // Copy vertices to VBO

	// Set attribute pointer 0 to hold Position data (used for the fill light)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Deactivates the VAO which is good practice
}

/* Implements the UKeyboard function */
void UKeyboard(unsigned char key, GLint x, GLint y)
{

	// Assigns certain characters which are used for keyboard navigation
	switch (key) {
		case 'w':
			currentKey = key;
			break;

		case 's':
			currentKey = key;
			break;

		case 'a':
			currentKey = key;
			break;

		case 'd':
			currentKey = key;
			break;

		// Added this key for changing views (Perspective/Orthographic)
		case 'p':
			currentKey = key;
			if(perspectiveViewOn == true){
				perspectiveViewOn = false;
			cout<<"You are now in Orthographic view"<<endl;
			}

			else {
				perspectiveViewOn = true;
			cout<<"You are now in Perspective view"<<endl;
			}
			break;

		// No action is performed if a key press is not recognized
		default:
			cout<<"Press another key"<<endl;
	}

}

/*Implements the UKeyReleased Function*/
void UKeyReleased(unsigned char key, GLint x, GLint y)
{
			currentKey = '0';
}

// Implements the UMousePR function to be able to use mouse button clicks
void UMousePR(int button, int state, int x, int y){

	// Left Mouse Button is used for orbiting
	if (button == GLUT_LEFT_BUTTON) {
		orbit = (state == GLUT_DOWN);
	}

	// Right Mouse Button is used for zooming
	else if (button == GLUT_RIGHT_BUTTON) {
		zoom = (state == GLUT_DOWN);
	}

}

/* Implements the UMouseMove function */
void UMouseMove(int x, int y)
{
	// Only works if user presses down 'Alt' AND either the left mouse button or the right mouse button, but not both mouse buttons
	if (glutGetModifiers() == GLUT_ACTIVE_ALT && orbit != zoom) {

		// Immediately replaces center locked coordinates with new mouse coordinates
		if(mouseDetected)
		{
			lastX = x;
			lastY = y;
			mouseDetected = false;
		}

		// Gets the direction the mouse was moved in x and y
		xOffset = x - lastX;
		yOffset = lastY - y; //Inverted Y

		// Updates with new mouse coordinates
		lastX = x;
		lastY = y;

		// Applies sensitivity to mouse direction
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		// Calls for an orbit
		if(orbit) {
			// Accumulates the yaw and pitch variables
			yaw += xOffset;
			pitch += yOffset;
		}

		// Calls for a zoom which will use CameraForwardZ movement either - or + in the z-space
		else if (zoom) {
			camDistance += mouseCameraSpeed * yOffset;
		}

		// Maintains a 90 degree pitch for gimbal lock
		if(pitch && yaw > 89.0f)
			pitch = 89.0f;

		if(pitch && yaw < -89.0f)
			pitch = -89.0f;

		// Orbits around the center
		front.x = camDistance * cos(yaw);
		front.y = camDistance * sin(pitch);
		front.z = sin(yaw) * cos(pitch) * camDistance;

	}
}

/* Generate and load the texture */
void UGenerateTexture() {

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height;

	unsigned char* image = SOIL_load_image("Wood.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads Wood texture file

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbinds the texture
}
