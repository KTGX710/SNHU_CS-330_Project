///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

#include <vector>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	/*** STUDENTS - add the code BELOW for defining object materials. ***/
	/*** There is no limit to the number of object materials that can ***/
	/*** be defined. Refer to the code in the OpenGL Sample for help  ***/

	OBJECT_MATERIAL goldMaterial;
	goldMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.1f);
	goldMaterial.ambientStrength = 0.4f;
	goldMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	goldMaterial.specularColor = glm::vec3(0.6f, 0.5f, 0.4f);
	goldMaterial.shininess = 22.0f;
	goldMaterial.tag = "gold";

	OBJECT_MATERIAL metalMaterial;
	metalMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	metalMaterial.ambientStrength = 0.4f;
	metalMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	metalMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	metalMaterial.shininess = 19.0f;
	metalMaterial.tag = "metal";

	OBJECT_MATERIAL cementMaterial;
	cementMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
	cementMaterial.ambientStrength = 0.2f;
	cementMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	cementMaterial.specularColor = glm::vec3(0.4f, 0.4f, 0.4f);
	cementMaterial.shininess = 0.5f;
	cementMaterial.tag = "cement";

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.ambientColor = glm::vec3(0.4f, 0.3f, 0.1f);
	woodMaterial.ambientStrength = 0.2f;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	woodMaterial.shininess = 0.3f;
	woodMaterial.tag = "wood";

	OBJECT_MATERIAL tileMaterial;
	tileMaterial.ambientColor = glm::vec3(0.2f, 0.3f, 0.4f);
	tileMaterial.ambientStrength = 0.3f;
	tileMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	tileMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	tileMaterial.shininess = 25.0f;
	tileMaterial.tag = "tile";

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	glassMaterial.ambientStrength = 0.3f;
	glassMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.3f);
	glassMaterial.specularColor = glm::vec3(0.6f, 0.6f, 0.6f);
	glassMaterial.shininess = 85.0f;
	glassMaterial.tag = "glass";

	OBJECT_MATERIAL clayMaterial;
	clayMaterial.ambientColor = glm::vec3(0.2f, 0.2f, 0.3f);
	clayMaterial.ambientStrength = 0.3f;
	clayMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.5f);
	clayMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.4f);
	clayMaterial.shininess = 0.5f;
	clayMaterial.tag = "clay";

	m_objectMaterials.push_back(goldMaterial);
	m_objectMaterials.push_back(metalMaterial);
	m_objectMaterials.push_back(cementMaterial);
	m_objectMaterials.push_back(woodMaterial);
	m_objectMaterials.push_back(tileMaterial);
	m_objectMaterials.push_back(glassMaterial);
	m_objectMaterials.push_back(clayMaterial);
}

/***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	// loading instances of texture file from Utilities folder
	//*** Cannot exceed 16 files ***
	//*** Files must be .jpg ***
	std::vector<std::string> jpg_texture_files = {
		"abstract",					// Slot 1
		"backdrop",					// Slot 2
		"breadcrust",				// Slot 3
		"cheddar",					// Slot 4
		"cheese_top",				// Slot 5
		"cheese_wheel",				// Slot 6
		"circular_brushed_gold",	// Slot 7
		"drywall",					// Slot 8
		"gold_seamless",			// Slot 9
		"knife_handle",				// Slot 10
		"pavers",					// Slot 11
		"rustic_wood",				// Slot 12
		//"stained_glass",			// Slot 13
		"stainless",				// Slot 14
		"stainless_end",			// Slot 15
		"tiles"						// Slot 16
	};

	// File directory prefix
	std::string path_prefix = "Utilities/textures/";
	std::string file_suffix_jpg = ".jpg";
	std::string file_suffix_png = ".png";

	// Iterates through texture_files array and loads all textures
	// Aborts if size of array exceeds 16 texture slots

	// Import png files first
	std::string png_tex = path_prefix + "herb" + file_suffix_png;
	CreateGLTexture(png_tex.c_str(), "herb");

	if (jpg_texture_files.size() <= 16) {
		for (std::string tex : jpg_texture_files) {
			std::string file_path = path_prefix + tex + file_suffix_jpg;
			CreateGLTexture(file_path.c_str(), tex);
		}
	}
	else {
		std::cerr << "Aborted texture loading due to size contrainsts\n" <<
			"Size of texture file array exceeded 16. Size of array: " << jpg_texture_files.size() <<
			std::endl;
	}

	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots
	BindGLTextures();
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line
	// m_pShaderManager->setBoolValue(g_UseLightingName, true);

	m_pShaderManager->setVec3Value("lightSources[0].position", 10.0f, 10.0f, -10.0f);
	m_pShaderManager->setVec3Value("lightSource[0].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSource[0].diffuseColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSource[0].specularColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setFloatValue("lightSource[0].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSource[0].specularIntensity", 0.5f);

	m_pShaderManager->setVec3Value("lightSources[1].position", -10.0f, 10.0f, -10.0f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.05f);

	m_pShaderManager->setVec3Value("lightSource[2].position", 0.0f, 10.0f, -10.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.01f, 0.01f, 0.01f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 32.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.05f);

	m_pShaderManager->setBoolValue("bUseLighting", true);

}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// define the materials for objects in the scene
	DefineObjectMaterials();
	// add and define the light sources for the scene
	SetupSceneLights();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadTorusMesh();

	LoadSceneTextures();
}

/***********************************************************
 *  getRGB()
 *
 * This method is a helper function to derive the vec4
 * color value of a hexedecimal color code
 ***********************************************************/
std::vector<float> SceneManager::getRGB(int u_HexValue, float u_alpha)
{
	// std::cout << "Calculating RGB value for " << std::to_string(u_HexValue) << std::endl;

	float r = float((u_HexValue >> 16) & 0xFF) / 255.0;
	float g = float((u_HexValue >> 8) & 0xFF) / 255.0;
	float b = float(u_HexValue & 0xFF) / 255.0;

	// std::cout << "RGB values are: (" << r << ", " << g << ", " << b << ")" << std::endl;

	return { r, g, b, u_alpha };
}

/***********************************************************
 *  drawShape()
 *
 * This method ecapsulates the function of drawing
 * 3D shapes to the screen, allowing all transformations 
 * and attributes to be passed as function argument,
 * minimizing code redundancy.
 ***********************************************************/
void SceneManager::drawShape(
	float xRot,
	float yRot,
	float zRot,
	float xScale,
	float yScale,
	float zScale,
	float xPos,
	float yPos,
	float zPos,
	int colorHex,
	float colorAlpha,
	std::string tex,
	std::string material,
	std::string shape
)
{
	// declare the variables for the transformations
	// set the XYZ scale for the mesh
	glm::vec3 scaleXYZ = glm::vec3(xScale, yScale, zScale); // 20, 1, 10
	float XrotationDegrees = (xRot);
	float YrotationDegrees = (yRot);
	float ZrotationDegrees = (zRot);
	// set the XYZ position for the mesh
	glm::vec3 positionXYZ = glm::vec3(xPos, yPos, zPos);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Assign the texture file, if no file name is provided, default to shader color
	if (!tex.empty()) {
		SetShaderTexture(tex);
	}
	else {
		// Set the object color and alpha value
		std::vector<float> rgb = getRGB(colorHex, colorAlpha);
		// set the color values into the shader
		SetShaderColor(rgb[0], rgb[1], rgb[2], rgb[3]);
	}

	SetShaderMaterial(material);

	// draw the shape mesh with transformation values
	if (shape == "plane") {
		m_basicMeshes->DrawPlaneMesh();
	}
	else if (shape == "box") {
		m_basicMeshes->DrawBoxMesh();
	}
	else if (shape == "cylinder") {
		m_basicMeshes->DrawCylinderMesh();
	}
	else if (shape == "tcylinder") {
		m_basicMeshes->DrawTaperedCylinderMesh();
	}
	else if (shape == "cone") {
		m_basicMeshes->DrawConeMesh();
	}
	else if (shape == "sphere") {
		m_basicMeshes->DrawSphereMesh();
	}
	else if (shape == "hsphere") {
		m_basicMeshes->DrawHalfSphereMesh();
	}
	else if (shape == "prism") {
		m_basicMeshes->DrawPrismMesh();
	}
	else if (shape == "pyramid3") {
		m_basicMeshes->DrawPyramid3Mesh();
	}
	else if (shape == "pyramid4") {
		m_basicMeshes->DrawPyramid4Mesh();
	}
	else if (shape == "torus") {
		m_basicMeshes->DrawTorusMesh();
	}
	else if (shape == "htorus") {
		m_basicMeshes->DrawHalfTorusMesh();
	}
	else {
		m_basicMeshes->DrawBoxMesh();
	}
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Backdrop
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		100.0f, 30.0f, 70.0f,
		// Position
		0.0f, 14.99f, 0.0f,
		// Color
		0x8E8E8E, 1,
		// Texture
		"abstract",
		// Material
		"clay",
		// Shape
		"box"
	);
	
	// Floor
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		50.0f, 1.0f, 35.0f,
		// Position
		0.0f, 0.0f, 0.0f,
		// Color
		0x8E8E8E, 1,
		// Texture
		"pavers",
		// Material
		"cement",
		// Shape
		"plane"
	);

	// Sky
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		50.0f, 1.0f, 35.0f,
		// Position
		0.0f, 29.98f, 0.0f,
		// Color
		0x8E8E8E, 1,
		// Texture
		"tiles",
		// Material
		"clay",
		// Shape
		"plane"
	);

	// Board body
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		5.0f, 0.5f, 5.0f,
		// Position
		0.0f, 0.05f, 0.0f,
		// Color
		0xD5B07C, 1,
		// Texture
		"rustic_wood",
		// Material
		"wood",
		// Shape
		"cylinder"
	);

	// Board handle
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		3.0f, 0.49f, 1.0f,
		// Position
		6.0f, 0.3f, 0.0f,
		// Color
		0xD5B07C, 1,
		// Texture
		"knife_handle",
		// Material
		"wood",
		// Shape
		"box"
	);

	// Board handle end
	drawShape(
		// Rotations (Degrees)
		0.0f, 90.0f, 0.0f,
		// Scale
		0.5f, 0.50f, 0.5f,
		// Position
		7.5f, 0.05f, 0.0f,
		// Color
		0xD5B07C, 1,
		// Texture
		"knife_handle",
		// Material
		"wood",
		// Shape
		"cylinder"
	);
	drawShape(
		// Rotations (Degrees)
		0.0f, 90.0f, 0.0f,
		// Scale
		0.35f, 0.502f, 0.35f,
		// Position
		7.5f, 0.05f, 0.0f,
		// Color
		0xD5B07C, 1,
		// Texture
		"stainless_end",
		// Material
		"metal",
		// Shape
		"cylinder"
	);

	// Cheese wheel
	drawShape(
		// Rotations (Degrees)
		0.0f, 90.0f, 0.0f,
		// Scale
		1.4f, 0.6f, 1.4f,
		// Position
		3.3f, 0.5f, 0.8f,
		// Color
		0xD5B07C, 1,
		// Texture
		"cheese_wheel",
		// Material
		"clay",
		// Shape
		"cylinder"
	);

	// Cheese block
	drawShape(
		// Rotations (Degrees)
		0.0f, -30.0f, 0.0f,
		// Scale
		3.0f, 1.0f, 1.5f,
		// Position
		3.0f, 1.0f, -2.0f,
		// Color
		0xD5B07C, 1,
		// Texture
		"cheddar",
		// Material
		"clay",
		// Shape
		"box"
	);

	// Grapes

	// Caviar
	for (unsigned int i = 7; i > 0; --i) {
		unsigned int k = 4 * i;
		for (unsigned int j = 0; j < k; ++j) {
			float posAngle = (360.0f / k) * j;
			float posZ = sin(glm::radians(posAngle));
			float posX = cos(glm::radians(posAngle));

			drawShape(
				// Rotations (Degrees)
				0.0f, 0.0f, 0.0f,
				// Scale
				0.1f, 0.1f, 0.1f,
				// Position
				0.4f + (0.11f * posX * i), 1.1f - (0.05f * i), 0.7f + (0.11f * posZ * i),
				// Color
				0x111111, 1,
				// Texture
				"",
				// Material
				"glass",
				// Shape
				"sphere"
			);
		}
	}
	// Caviar Bowl
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 180.0f,
		// Scale
		1.0f, 0.5f, 1.0f,
		// Position
		0.4f, 0.9f, 0.7f,
		// Color
		0xFFFFFF, 1,
		// Texture
		"drywall",
		// Material
		"clay",
		// Shape
		"hsphere"
	);
	// Caviar Bowl Base
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		0.8f, 0.05f, 0.8f,
		// Position
		0.4f, 0.55f, 0.7f,
		// Color
		0xFFFFFF, 1,
		// Texture
		"drywall",
		// Material
		"clay",
		// Shape
		"cylinder"
	);

	// Crackers
	for (unsigned int i = 0; i < 12; ++i) {
		float posAngle = 183.0f + (i * 9);
		float posZ = sin(glm::radians(posAngle));
		float posX = cos(glm::radians(posAngle));

		drawShape(
			// Rotations (Degrees)
			-10.0f * posX, 0.0f, -10.0f * posZ,
			// Scale
			1.0f, 0.1f, 1.0f,
			// Position
			3.6f * posX, 0.8f, 3.6f * posZ,
			// Color
			0xD5B07C, 1,
			// Texture
			"gold_seamless",
			// Material
			"wood",
			// Shape
			"cylinder"
		);
	}

	// Salami
	for (unsigned int i = 0; i < 8; ++i) {
		float posAngle = 180.0f + (i * 360 / 20);
		float posZ = sin(glm::radians(posAngle));
		float posX = cos(glm::radians(posAngle));

		drawShape(
			// Rotations (Degrees)
			10.0f * posX, 0.0f, 10.0f * posZ,
			// Scale
			1.0f, 0.1f, 1.0f,
			// Position
			1.5f * posX, 0.7f, 1.5f * posZ,
			// Color
			0xD5B07C, 1,
			// Texture
			"abstract",
			// Material
			"clay",
			// Shape
			"cylinder"
		);
	}

	// Olives
	for (unsigned int i = 0; i < 6; ++i) {
		float posAngle = (i * 360.0f / 6.0f);
		float posZ = sin(glm::radians(posAngle));
		float posX = cos(glm::radians(posAngle));

		drawShape(
			// Rotations (Degrees)
			0.0f, -1.0f * posAngle, 50.0f,
			// Scale
			0.4f, 0.2f, 0.2f,
			// Position
			(0.4f * posX) + 1.55f, 0.9f, (0.4f * posZ) + 2.0f,
			// Color
			0x808000, 1,
			// Texture
			"",
			// Material
			"metal",
			// Shape
			"sphere"
		);
	}
	// Cherry Tomato
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		0.2f, 0.2f, 0.2f,
		// Position
		1.55f, 0.9f, 2.0f,
		// Color
		0x952E25, 1,
		// Texture
		"",
		// Material
		"metal",
		// Shape
		"sphere"
	);
	// Tomato Leaves
	for (unsigned int j = 0; j < 5; ++j) {
		GLfloat posAngle = 360.0f / 5 * j;
		GLfloat posX = cos(glm::radians(posAngle)) / 8.0f;
		GLfloat posZ = sin(glm::radians(posAngle)) / 8.0f;
		drawShape(
			// Rotations (Degrees)
			0.0f, posAngle, -15.0f,
			// Scale
			0.1f, 0.0005f, 0.05f,
			// Position
			1.55f + posX, 1.05f, 2.0f - posZ,
			// Color
			0x009900, 1,
			// Texture
			"",
			// Material
			"wood",
			// Shape
			"sphere"
		);
	}
	// Olive Bowl
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 180.0f,
		// Scale
		0.7f, 0.5f, 0.7f,
		// Position
		1.55f, 0.95f, 2.0f,
		// Color
		0xFFFFFF, 1,
		// Texture
		"drywall",
		// Material
		"clay",
		// Shape
		"hsphere"
	);
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		0.42f, 0.05f, 0.42f,
		// Position
		1.55f, 0.51f, 2.0f,
		// Color
		0xFFFFFF, 1,
		// Texture
		"drywall",
		// Material
		"clay",
		// Shape
		"cylinder"
	);

	// Herb
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		0.02f, 0.2f, 0.02f,
		// Position
		3.0f, 0.5f, 2.5f,
		// Color
		0xA52A2A, 1,
		// Texture
		"",
		// Material
		"wood",
		// Shape
		"cylinder"
	);
	for (unsigned int i = 0; i < 1; ++i) {
		for (unsigned int j = 0; j < 6; ++j) {
			GLfloat posAngle = 360.0f / 6 * j;
			GLfloat posX = cos(glm::radians(posAngle)) / 4.0f;
			GLfloat posZ = sin(glm::radians(posAngle)) / 4.0f;
			drawShape(
				// Rotations (Degrees)
				0.0f, posAngle, 45.0f,
				// Scale
				0.4f, 0.2f, 0.001f,
				// Position
				2.8f + posX, 1.f, 2.6f - posZ,
				// Color
				0x009900, 1,
				// Texture
				"",
				// Material
				"wood",
				// Shape
				"sphere"
			);
		}
	}

	// Bread
	drawShape(
		// Rotations (Degrees)
		0.0f, 70.0f, 0.0f,
		// Scale
		1.8f, 0.9f, 1.8f,
		// Position
		-2.4f, 0.9f, 2.5f,
		// Color
		0x0000FF, 1,
		// Texture
		"breadcrust",
		// Material
		"wood",
		// Shape
		"sphere"
	);

	// Orange
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		0.7f, 0.7f, 0.7f,
		// Position
		0.1f, 1.2f, 2.5f,
		// Color
		0xFFA500, 1,
		// Texture
		"",
		// Material
		"clay",
		// Shape
		"sphere"
	);

	// Tomatoes
	for (unsigned int i = 1; i < 4; ++i) {
		for (unsigned int j = 3; j > 0; --j) {
			if (!(j == 2 && i == 2)) {
				drawShape(
					// Rotations (Degrees)
					0.0f, 0.0f, 0.0f,
					// Scale
					0.2f, 0.2f, 0.2f,
					// Position
					1.3f + (i / 2.5f), 0.75f, 2.7f + (j / 2.5f),
					// Color
					0x952E25, 1,
					// Texture
					"",
					// Material
					"metal",
					// Shape
					"sphere"
				);
			}
			if (!(i == 2 || j == 2)) {
				drawShape(
					// Rotations (Degrees)
					0.0f, 0.0f, 0.0f,
					// Scale
					0.2f, 0.2f, 0.2f,
					// Position
					1.7f + (i / 5.0f), 1.0f, 3.1f + (j / 5.0f),
					// Color
					0x952E25, 1,
					// Texture
					"",
					// Material
					"metal",
					// Shape
					"sphere"
				);
			}
			if (i == 2 && j == 2) {
				drawShape(
					// Rotations (Degrees)
					0.0f, 0.0f, 0.0f,
					// Scale
					0.2f, 0.2f, 0.2f,
					// Position
					2.1f, 1.25f, 3.5f,
					// Color
					0x952E25, 1,
					// Texture
					"",
					// Material
					"metal",
					// Shape
					"sphere"
				);
			}
		}
	}

	// Dummy shape for debugging
	/*
	drawShape(
		// Rotations (Degrees)
		0.0f, 0.0f, 0.0f,
		// Scale
		1.0f, 1.0f, 1.0f,
		// Position
		0.0f, 0.0f, 0.0f,
		// Color
		0x0000FF, 1,
		// Shape
		"sphere"
	);
	*/

	/****************************************************************/
}
