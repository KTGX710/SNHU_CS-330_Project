///////////////////////////////////////////////////////////////////////////////
// shadermanager.h
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"

#include <string>
#include <vector>

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
	// constructor
	SceneManager(ShaderManager *pShaderManager);
	// destructor
	~SceneManager();

	struct TEXTURE_INFO
	{
		std::string tag;
		uint32_t ID;
	};

	struct OBJECT_MATERIAL
	{
		float ambientStrength;
		glm::vec3 ambientColor;
		glm::vec3 diffuseColor;
		glm::vec3 specularColor;
		float shininess;
		std::string tag;
	};

private:
	// pointer to shader manager object
	ShaderManager* m_pShaderManager;
	// pointer to basic shapes object
	ShapeMeshes* m_basicMeshes;
	// total number of loaded textures
	int m_loadedTextures;
	// loaded textures info
	TEXTURE_INFO m_textureIDs[16];
	// defined object materials
	std::vector<OBJECT_MATERIAL> m_objectMaterials;

	// load texture images and convert to OpenGL texture data
	bool CreateGLTexture(const char* filename, std::string tag);
	// bind loaded OpenGL textures to slots in memory
	void BindGLTextures();
	// free the loaded OpenGL textures
	void DestroyGLTextures();
	// find a loaded texture by tag
	int FindTextureID(std::string tag);
	int FindTextureSlot(std::string tag);
	// find a defined material by tag
	bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);
	// define the materials for objects in the scene
	void DefineObjectMaterials();
	// add and define the light sources for the scene
	void SetupSceneLights();

	/***********************
	* My Helper Functions
	************************/

	// color hexadecimal value converter
	std::vector<GLfloat> getRGB(int u_HexColor, GLfloat u_alpha);

	// get hexadecimal value from RGB values
	int getHex(int r, int g, int b);

	// Encapsulate object drawing
	void drawShape(
		GLfloat xRot,
		GLfloat yRot,
		GLfloat zRot,
		GLfloat xScale,
		GLfloat yScale,
		GLfloat zScale,
		GLfloat xPos,
		GLfloat yPos,
		GLfloat zPos,
		int colorHex,
		GLfloat colorAlpha,
		std::string texture,
		std::string material,
		std::string shape
	);

	// set the transformation values 
	// into the transform buffer
	void SetTransformations(
		glm::vec3 scaleXYZ,
		GLfloat XrotationDegrees,
		GLfloat YrotationDegrees,
		GLfloat ZrotationDegrees,
		glm::vec3 positionXYZ);

	// set the color values into the shader
	void SetShaderColor(
		GLfloat redColorValue,
		GLfloat greenColorValue,
		GLfloat blueColorValue,
		GLfloat alphaValue);

	// set the texture data into the shader
	void SetShaderTexture(
		std::string textureTag);

	// set the UV scale for the texture mapping
	void SetTextureUVScale(
		float u, float v);

	// set the object material into the shader
	void SetShaderMaterial(
		std::string materialTag);

public:

	// The following methods are for the students to 
	// customize for their own 3D scene
	void LoadSceneTextures();
	void PrepareScene();
	void RenderScene();

};