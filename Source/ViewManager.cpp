///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;
	bool gCursorEscaped = false; // Tracks when cursor is captured by window or free

	// time between current frame and last frame
	float gDeltaTime = 0.0f; 
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager *pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(6.7f, 5.1f, 4.0f);
	g_pCamera->Front = glm::vec3(-0.6f, -0.7f, -0.4f);
	g_pCamera->Up = glm::vec3(-0.56f, 0.72f, -0.39f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// callback to receive scroll wheel inputs
	glfwSetScrollCallback(window, &ViewManager::Scroll_Callback);

	// Capturing and hiding cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Scroll_Callback()
 *
 *  This method defines the callback for the scroll wheel
 *	to modify the camera speed
 *
 * KG - 5/29/'26
 ***********************************************************/
void ViewManager::Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset) {
	g_pCamera->ProcessMouseScroll((float)yoffset * -0.3);
	g_pCamera->MouseSensitivity += yoffset * 0.001;
	if (g_pCamera->MouseSensitivity < 0.001) g_pCamera->MouseSensitivity = 0.001;
	if (g_pCamera->MouseSensitivity > 0.1) g_pCamera->MouseSensitivity = 0.1;
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	/*******************************************************
	*	Implemented functionality for window to capture
	*	and hide the cursor when active to prevent
	*	the cursor from espacping under typical use.
	*
	*	Also implemented recapture when window is hovered
	*	over and clicked with LMB.
	*	When RMB is clicked, cursor escapes window and
	*	returns to normal mode.
	*
	*	KG - 5/29/'26
	********************************************************/

	// when the first mouse move event is received, this needs to be recorded so that
	// all subsequent mouse moves can correctly calculate the X position offset and Y
	// position offset for proper operation
	if (gFirstMouse)
	{
		gLastX = xMousePos;
		gLastY = yMousePos;
		gFirstMouse = false;
	}

	// calculate the X offset and Y offset values for moving the 3D camera accordingly
	float xOffset = xMousePos - gLastX;
	float yOffset = gLastY - yMousePos; // reversed since y-coordinates go from bottom to top

	// set the current positions into the last position variables
	gLastX = xMousePos;
	gLastY = yMousePos;

	// move the 3D camera according to the calculated offsets
	// Skip callback if cursor escaped
	if (!gCursorEscaped) g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}

/***********************************************************
 *  ProcessMouseKeyEvent()
 *
 *  This method is called to process any mouse keypresses
 *	for the purpose to provide the user to escape
 *	the application window without having to exit the process
 ***********************************************************/
void ViewManager::ProcessMouseKeyEvent()
{

	// Recapture cursor if window is in focus and clicked
	if (glfwGetWindowAttrib(m_pWindow, GLFW_HOVERED) && 
		(glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		) {
		gCursorEscaped = false;
		// Recapturing cursor
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	// Skip callback if cursor escaped
	if (gCursorEscaped) return;

	// Release cursor if right-mouse-button is clicked
	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		gCursorEscaped = true;
		return;
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{

	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// if the camera object is null, then exit this method
	if (NULL == g_pCamera)
	{
		return;
	}

	/*
	* Added Camera Speed modifier to DeltaTime
	*
	* KG - 5/29/'26
	*/
	// process camera zooming in and out
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(m_pWindow, GLFW_KEY_UP) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(m_pWindow, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
	}

	// process camera panning left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(m_pWindow, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(m_pWindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
	}

	/***********************************************
	*	Implemented upward & downward camera travel
	*	via Q and E keys.
	*
	*	KG - 5/29/'26
	************************************************/
	// process camera panning up and down
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(m_pWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(UP, gDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(m_pWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
	}

	/***********************************************
	*	Implemented functionality to toggle between
	*	perspective and orthographic views.
	*	P - perspective view
	*	O - orthographic view
	*
	*	KG - 5/29/'26
	************************************************/
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS && bOrthographicProjection == false) {
		bOrthographicProjection = true;
		
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS && bOrthographicProjection == true) {
		bOrthographicProjection = false;
	}

}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// process any mouse key events in the event queue
	ProcessMouseKeyEvent();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// if statement to switch between orthographic and perspective view
	if (bOrthographicProjection) {
		// define the orthographic projection matrix
		projection = glm::ortho(
			(GLfloat)WINDOW_WIDTH * -0.01f, (GLfloat)WINDOW_WIDTH * 0.01f, 
			(GLfloat)WINDOW_HEIGHT * -0.01f, (GLfloat)WINDOW_HEIGHT * 0.01f, 
			0.0f, 20.0f
		);
	}
	else {
		// define the current projection matrix
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 
			0.1f, 100.0f
		);
	}
	
	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}