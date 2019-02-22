/*
	SimionZoo: A framework for online model-free Reinforcement Learning on continuous
	control problems

	Copyright (c) 2016 SimionSoft. https://github.com/simionsoft

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "stdafx.h"
#include "camera.h"
#include "xml-load.h"
#include "renderer.h"
#include "../GeometryLib/vector3d.h"
#include "../GeometryLib/transform3d.h"

Camera::Camera(): SceneActor3D()
{
}

Camera::~Camera()
{
}

Matrix44 Camera::getModelviewMatrix() const
{
	Matrix44 mat, rot, trans;
	Quaternion rotation = m_transform.rotation();
	rotation = rotation.inverse();
	rot.setRotation(rotation);
	Vector3D translation = m_transform.translation();
	translation = translation.inverse();
	trans.setTranslation(translation);
	mat = rot*trans;

	return mat;
}

Camera* Camera::getInstance(tinyxml2::XMLElement* pNode)
{
	tinyxml2::XMLElement* pChild = pNode->FirstChildElement();
	if (!strcmp(pChild->Name(), XML_TAG_SIMPLE_CAMERA))
		return new SimpleCamera(pChild);
	return nullptr;
}

SimpleCamera::SimpleCamera(tinyxml2::XMLElement* pNode)
{
	tinyxml2::XMLElement* pChild = pNode->FirstChildElement(XML_TAG_TRANSFORM);
	if (pChild)
		XML::load(pChild,m_transform);
}

SimpleCamera::SimpleCamera()
{
}

void SimpleCamera::set()
{
	//set projection matrix
	glMatrixMode(GL_PROJECTION);
	Matrix44 perspectiveMatrix;
	//the near plane has width 1 and the height is adjusted according to the aspect ratio
	int screenHeight, screenWidth;
	Renderer::get()->getWindowsSize(screenWidth, screenHeight);
	double aspectRatio = (double)screenWidth / (double)screenHeight;
	perspectiveMatrix.setPerspective(0.5, 0.5/aspectRatio, nearPlane, farPlane);
	glLoadMatrixd(perspectiveMatrix.asArray());

	//set modelview matrix
	glMatrixMode(GL_MODELVIEW);
	Matrix44 modelviewMatrix = getModelviewMatrix();
	glLoadMatrixd(modelviewMatrix.asArray());

	//The order of the matrix multiplication seems to be wrong in the source paper:
	//http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	Matrix44 cameraMatrix = perspectiveMatrix * modelviewMatrix;
	m_frustum.fromCameraMatrix(cameraMatrix);
}

//This method sets an orthogonal view that maps the screen to coordinates [0.0,0.0] - [1.0,1.0]
void Camera::set2DView()
{
	//set projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);

	//set modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}