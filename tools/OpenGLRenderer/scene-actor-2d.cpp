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
#include "scene-actor-2d.h"
#include "xml-load.h"

SceneActor2D::SceneActor2D(tinyxml2::XMLElement* pNode)
{
	XMLElement* pChild;
	pChild = pNode->FirstChildElement(XML_TAG_TRANSFORM);
	if (pChild)
		XML::load(pChild, m_transform);
}

SceneActor2D::SceneActor2D(Vector2D origin, Vector2D size, double rotation, double depth)
{
	m_transform.setTranslation(origin);
	m_transform.setScale(size);
	m_transform.setRotation(rotation);
	m_transform.setDepth(depth);
}

SceneActor2D::~SceneActor2D()
{
}


void SceneActor2D::setTransform()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	double *pMatrix = getTransformMatrix().asArray();
	if (pMatrix)
		glMultMatrixd(pMatrix);
}

void SceneActor2D::restoreTransform()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

Matrix44 SceneActor2D::getTransformMatrix()
{
	Quaternion quat;
	Matrix44 mat, rot, scale, trans;
	quat.fromOrientations(0.0, 0.0, m_transform.rotation());
	rot.setRotation(quat);
	Vector2D translation= m_transform.translation();
	Vector3D translation3D = Vector3D(translation.x(), translation.y(), m_transform.depth());
	trans.setTranslation(translation3D);
	Vector2D s = m_transform.scale();
	Vector3D s3D = Vector3D(s.x(), s.y(), 1.0);
	scale.setScale(s3D);
	mat = trans * rot * scale;
	return mat;
}

void SceneActor2D::addLocalOffset(Vector2D& offset)
{
	Matrix44 mat = getTransformMatrix();
	Vector3D offset3D = Vector3D(offset.x(), offset.y(), 0.0);
	Vector3D worldOffset = mat * offset3D;
	m_transform.setTranslation(m_transform.translation() + Vector2D(worldOffset.x(),worldOffset.y()));
}

void SceneActor2D::addWorldOffset(Vector2D& offset)
{
	m_transform.setTranslation(m_transform.translation() + offset);
}

void SceneActor2D::addRotation(double angle)
{
	m_transform.setRotation(m_transform.rotation() + angle);
}