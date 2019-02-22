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
#include "input-handler.h"
#include "renderer.h"
#include "camera.h"
#include <math.h>

#define MOVE_SPEED 10.0
#define ROTATION_SPEED 1.0
#define PI 3.1416f

IInputHandler* IInputHandler::m_pInstance = 0;

FreeCameraInputHandler::FreeCameraInputHandler()
{
	m_pInstance = this;

	for (size_t key = 0; key < NUM_KEYS; ++key)
		m_keyboardState[key] = false;

	for (size_t key = 0; key < NUM_KEYS; ++key)
		m_specialKeysState[key] = false;

	glutSpecialFunc(_onSpecialKeyPressed);
	glutSpecialUpFunc(_onSpecialKeyReleased);
	glutKeyboardFunc(_onKeyPressed);
	glutKeyboardUpFunc(_onKeyReleased);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	m_timer.start();
}

IInputHandler* IInputHandler::get()
{
	return m_pInstance;
}


FreeCameraInputHandler::~FreeCameraInputHandler()
{
}

void FreeCameraInputHandler::_onSpecialKeyPressed(int key, int x, int y)
{
	Renderer::get()->logMessage(string("Special key pressed: ") + to_string(key));
	((FreeCameraInputHandler*)IInputHandler::get())->onSpecialKeyPressed(key, x, y);
}

void FreeCameraInputHandler::_onSpecialKeyReleased(int key, int x, int y)
{
	Renderer::get()->logMessage(string("Special key released: ") + to_string(key));
	((FreeCameraInputHandler*)IInputHandler::get())->onSpecialKeyReleased(key, x, y);
}

void FreeCameraInputHandler::_onKeyPressed(unsigned char key, int x, int y)
{
	Renderer::get()->logMessage(string("Key pressed: ") + to_string(key));
	((FreeCameraInputHandler*)IInputHandler::get())->onKeyPressed(key, x, y);
}

void FreeCameraInputHandler::_onKeyReleased(unsigned char key, int x, int y)
{
	Renderer::get()->logMessage(string("Key released: ") + to_string(key));
	((FreeCameraInputHandler*)IInputHandler::get())->onKeyReleased(key, x, y);
}

bool FreeCameraInputHandler::keyPressed(unsigned char key) const
{
	return m_keyboardState[key];
}

bool FreeCameraInputHandler::specialKeyPressed(unsigned char key) const
{
	return m_specialKeysState[key];
}

bool FreeCameraInputHandler::bMoveForward() const
{
	return keyPressed('w') || specialKeyPressed(GLUT_KEY_UP);
}
bool FreeCameraInputHandler::bMoveBackward() const
{
	return keyPressed('s') || specialKeyPressed(GLUT_KEY_DOWN);
}
bool FreeCameraInputHandler::bYawRight() const
{
	if (bMoveBackward())
		return keyPressed('d') || specialKeyPressed(GLUT_KEY_RIGHT);
	else
		return keyPressed('a') || specialKeyPressed(GLUT_KEY_LEFT);
}

bool FreeCameraInputHandler::bYawLeft() const 
{
	if (bMoveBackward())
		return keyPressed('a') || specialKeyPressed(GLUT_KEY_LEFT);
	else
		return keyPressed('d') || specialKeyPressed(GLUT_KEY_RIGHT);
}

bool FreeCameraInputHandler::bMoveUp() const
{
	return keyPressed('W');
}

bool FreeCameraInputHandler::bMoveDown() const
{
	return keyPressed('S');
}

bool FreeCameraInputHandler::bMoveLeft() const
{
	return keyPressed('A');
}

bool FreeCameraInputHandler::bMoveRight() const 
{
	return keyPressed('D');
}

void FreeCameraInputHandler::onKeyPressed(unsigned char key, int x, int y)
{
	if (key == 27)
	{
		Renderer::get()->logMessage("Exiting program");
		m_bExitRequested = true;
		exit(0); //easier to get out of the main loop
	}
	m_keyboardState[key] = true;
}

void FreeCameraInputHandler::onKeyReleased(unsigned char key, int x, int y)
{
	m_keyboardState[key] = false;
}

void FreeCameraInputHandler::onSpecialKeyPressed(int key, int x, int y)
{
	m_specialKeysState[key] = true;
}

void FreeCameraInputHandler::onSpecialKeyReleased(int key, int x, int y)
{
	m_specialKeysState[key] = false;
}

void FreeCameraInputHandler::handleInput()
{
	double dt = m_timer.getElapsedTime(true);

	glutMainLoopEvent();

	Camera* pCamera = Renderer::get()->getActiveCamera();
	if (!pCamera) return;

	Matrix44 tras = pCamera->getTransformMatrix();
	Quaternion quat;

	double yaw = pCamera->getTransform().rotation().yaw();
	if (bMoveForward())
		pCamera->addWorldOffset(Vector3D(-dt*MOVE_SPEED*sin(yaw), 0.0, -dt*MOVE_SPEED*cos(yaw)));
	if (bMoveBackward())
		pCamera->addWorldOffset(Vector3D(dt*MOVE_SPEED*sin(yaw), 0.0, dt*MOVE_SPEED*cos(yaw)));
	if (bYawRight())
	{
		quat = pCamera->getRotation();
		pCamera->setRotation(Quaternion(quat.yaw() + ROTATION_SPEED * dt, quat.pitch(), 0.0));
	}
	if (bYawLeft())
	{
		quat = pCamera->getRotation();
		pCamera->setRotation(Quaternion(quat.yaw() - ROTATION_SPEED * dt, quat.pitch(), 0.0));
	}
	if (bMoveUp())
		pCamera->addLocalOffset(Vector3D(0.0, dt*MOVE_SPEED, 0.0));
	if (bMoveDown())
		pCamera->addLocalOffset(Vector3D(0.0, -dt*MOVE_SPEED, 0.0));
	if (bMoveLeft())
		pCamera->addLocalOffset(Vector3D(dt*MOVE_SPEED, 0.0, 0.0));
	if (bMoveRight())
		pCamera->addLocalOffset(Vector3D(-dt*MOVE_SPEED, 0.0, 0.0));
}