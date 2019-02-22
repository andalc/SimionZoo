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

#include "matrix44.h"


Matrix44::Matrix44()
{
}


Matrix44::~Matrix44()
{
}

void Matrix44::setIdentity()
{
	set(0, 0, 1.0); set(0, 1, 0.0); set(0, 2, 0.0); set(0, 3, 0.0);
	set(1, 0, 0.0); set(1, 1, 1.0); set(1, 2, 0.0); set(1, 3, 0.0);
	set(2, 0, 0.0); set(2, 1, 0.0); set(2, 2, 1.0); set(2, 3, 0.0);
	set(3, 0, 0.0); set(3, 1, 0.0); set(3, 2, 0.0); set(3, 3, 1.0);
}

//http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
void Matrix44::setRotation(const Quaternion& originalQuat)
{
	Quaternion quat = originalQuat;
	if (originalQuat.bUseOrientations())
		quat.fromOrientations();

	double sqw = quat.w()*quat.w();
	double sqx = quat.x()*quat.x();
	double sqy = quat.y()*quat.y();
	double sqz = quat.z()*quat.z();

	// invs (inverse square length) is only required if quaternion is not already normalised
	double invs = 1 / (sqx + sqy + sqz + sqw);
	set(0, 0, (sqx - sqy - sqz + sqw)*invs); // since sqw + sqx + sqy + sqz =1/invs*invs
	set(1, 1, (-sqx + sqy - sqz + sqw)*invs);
	set(2, 2, (-sqx - sqy + sqz + sqw)*invs);
	set(3, 0, 0.0);
	set(3, 1, 0.0);
	set(3, 2, 0.0);
	set(0, 3, 0.0);
	set(1, 3, 0.0);
	set(2, 3, 0.0);
	set(3, 3, 1.0);

	double tmp1 = quat.x()*quat.y();
	double tmp2 = quat.z()*quat.w();
	set(0, 1, 2.0 * (tmp1 + tmp2)*invs);
	set(1, 0, 2.0 * (tmp1 - tmp2)*invs);

	tmp1 = quat.x()*quat.z();
	tmp2 = quat.y()*quat.w();
	set(0, 2, 2.0 * (tmp1 - tmp2)*invs);
	set(2, 0, 2.0 * (tmp1 + tmp2)*invs);
	tmp1 = quat.y()*quat.z();
	tmp2 = quat.x()*quat.w();
	set(1, 2, 2.0 * (tmp1 + tmp2)*invs);
	set(2, 1, 2.0 * (tmp1 - tmp2)*invs);
}

void Matrix44::setTranslation(const Vector3D& translation)
{
	set(0, 0, 1.0);
	set(0, 1, 0.0);
	set(0, 2, 0.0);
	set(0, 3, 0.0);
	set(1, 0, 0.0);
	set(1, 1, 1.0);
	set(1, 2, 0.0);
	set(1, 3, 0.0);
	set(2, 0, 0.0);
	set(2, 1, 0.0);
	set(2, 2, 1.0);
	set(2, 3, 0.0);
	set(3, 0, translation.x());
	set(3, 1, translation.y());
	set(3, 2, translation.z());
	set(3, 3, 1.0);
}
void Matrix44::setScale(const Vector3D& scale)
{
	set(0, 0, scale.x());
	set(0, 1, 0.0);
	set(0, 2, 0.0);
	set(0, 3, 0.0);
	set(1, 0, 0.0);
	set(1, 1, scale.y());
	set(1, 2, 0.0);
	set(1, 3, 0.0);
	set(2, 0, 0.0);
	set(2, 1, 0.0);
	set(2, 2, scale.z());
	set(2, 3, 0.0);
	set(3, 0, 0.0);
	set(3, 1, 0.0);
	set(3, 2, 0.0);
	set(3, 3, 1.0);
}

void Matrix44::setPerspective(double halfWidth, double halfHeight, double nearPlaneDist, double farPlaneDist)
{
	//http://www.songho.ca/opengl/gl_projectionmatrix.html
	setIdentity();
	
	if (farPlaneDist - nearPlaneDist == 0.0) return;

	//[0,0]= n/r
	set(0, 0, nearPlaneDist / halfWidth);
	//[1,1]= n/t
	set(1, 1, nearPlaneDist / halfHeight);
	//[2,2]= -(f+n)/(f-n)
	set(2, 2, -(farPlaneDist + nearPlaneDist) / (farPlaneDist - nearPlaneDist));
	//[3,2]= -2fn/(f-n)
	set(3, 2, -2 * farPlaneDist*nearPlaneDist / (farPlaneDist - nearPlaneDist));
	//[2,3]= -1
	set(2, 3, -1.0);
	//[3,3]= 0
	set(3, 3, 0.0);
}

BoundingBox3D Matrix44::operator*(const BoundingBox3D& box) const
{
	BoundingBox3D result;
	result.setMin((*this)*box.min());
	result.setMax((*this)*box.max());
	return result;
}

Matrix44 Matrix44::operator*(const Matrix44& mat) const
{
	Matrix44 result;
	result.set(0, 0, get(0, 0)*mat.get(0, 0) + get(1, 0)*mat.get(0, 1) + get(2, 0)*mat.get(0, 2) + get(3, 0)*mat.get(0, 3));
	result.set(1, 0, get(0, 0)*mat.get(1, 0) + get(1, 0)*mat.get(1, 1) + get(2, 0)*mat.get(1, 2) + get(3, 0)*mat.get(1, 3));
	result.set(2, 0, get(0, 0)*mat.get(2, 0) + get(1, 0)*mat.get(2, 1) + get(2, 0)*mat.get(2, 2) + get(3, 0)*mat.get(2, 3));
	result.set(3, 0, get(0, 0)*mat.get(3, 0) + get(1, 0)*mat.get(3, 1) + get(2, 0)*mat.get(3, 2) + get(3, 0)*mat.get(3, 3));

	result.set(0, 1, get(0, 1)*mat.get(0, 0) + get(1, 1)*mat.get(0, 1) + get(2, 1)*mat.get(0, 2) + get(3, 1)*mat.get(0, 3));
	result.set(1, 1, get(0, 1)*mat.get(1, 0) + get(1, 1)*mat.get(1, 1) + get(2, 1)*mat.get(1, 2) + get(3, 1)*mat.get(1, 3));
	result.set(2, 1, get(0, 1)*mat.get(2, 0) + get(1, 1)*mat.get(2, 1) + get(2, 1)*mat.get(2, 2) + get(3, 1)*mat.get(2, 3));
	result.set(3, 1, get(0, 1)*mat.get(3, 0) + get(1, 1)*mat.get(3, 1) + get(2, 1)*mat.get(3, 2) + get(3, 1)*mat.get(3, 3));

	result.set(0, 2, get(0, 2)*mat.get(0, 0) + get(1, 2)*mat.get(0, 1) + get(2, 2)*mat.get(0, 2) + get(3, 2)*mat.get(0, 3));
	result.set(1, 2, get(0, 2)*mat.get(1, 0) + get(1, 2)*mat.get(1, 1) + get(2, 2)*mat.get(1, 2) + get(3, 2)*mat.get(1, 3));
	result.set(2, 2, get(0, 2)*mat.get(2, 0) + get(1, 2)*mat.get(2, 1) + get(2, 2)*mat.get(2, 2) + get(3, 2)*mat.get(2, 3));
	result.set(3, 2, get(0, 2)*mat.get(3, 0) + get(1, 2)*mat.get(3, 1) + get(2, 2)*mat.get(3, 2) + get(3, 2)*mat.get(3, 3));

	result.set(0, 3, get(0, 3)*mat.get(0, 0) + get(1, 3)*mat.get(0, 1) + get(2, 3)*mat.get(0, 2) + get(3, 3)*mat.get(0, 3));
	result.set(1, 3, get(0, 3)*mat.get(1, 0) + get(1, 3)*mat.get(1, 1) + get(2, 3)*mat.get(1, 2) + get(3, 3)*mat.get(1, 3));
	result.set(2, 3, get(0, 3)*mat.get(2, 0) + get(1, 3)*mat.get(2, 1) + get(2, 3)*mat.get(2, 2) + get(3, 3)*mat.get(2, 3));
	result.set(3, 3, get(0, 3)*mat.get(3, 0) + get(1, 3)*mat.get(3, 1) + get(2, 3)*mat.get(3, 2) + get(3, 3)*mat.get(3, 3));

	return result;
}



Point3D Matrix44::operator*(const Point3D& v) const
{
	Point3D result;
	result.setX(get(0, 0)*v.x() + get(1, 0)*v.y() + get(2, 0)*v.z() + get(3, 0));
	result.setY(get(0, 1)*v.x() + get(1, 1)*v.y() + get(2, 1)*v.z() + get(3, 1));
	result.setZ(get(0, 2)*v.x() + get(1, 2)*v.y() + get(2, 2)*v.z() + get(3, 2));
	return result;
}

Vector3D Matrix44::operator*(const Vector3D& v) const
{
	Vector3D result;
	result.setX(get(0, 0)*v.x() + get(1, 0)*v.y() + get(2, 0)*v.z());
	result.setY(get(0, 1)*v.x() + get(1, 1)*v.y() + get(2, 1)*v.z());
	result.setZ(get(0, 2)*v.x() + get(1, 2)*v.y() + get(2, 2)*v.z());
	return result;
}

Point2D Matrix44::operator*(const Point2D& v) const
{
	Point2D result;
	result.setX(get(0, 0)*v.x() + get(1, 0)*v.y() + get(3, 0));
	result.setY(get(0, 1)*v.x() + get(1, 1)*v.y() + get(3, 1));
	return result;
}

Vector2D Matrix44::operator*(const Vector2D& v) const
{
	Vector2D result;
	result.setX(get(0, 0)*v.x() + get(1, 0)*v.y() );
	result.setY(get(0, 1)*v.x() + get(1, 1)*v.y() );
	return result;
}

double* Matrix44::asArray()
{
	return _values;
}

double Matrix44::get(int col, int row) const
{
	if (col >= 0 && col < 4 && row>=0 && row < 4)
	{
		return _values[col*4 + row];
	}
	return 0.0;
}

void Matrix44::set(int col, int row, double value)
{
	if (col >= 0 && col < 4 && row>=0 && row < 4)
	{
		_values[col*4 + row]= value;
	}
}

void Matrix44::flipYZAxis()
{
	//https://www.gamedev.net/topic/537664-handedness/#entry4480987
	double tmp;
	//swap 2nd and 3rd row vectors
	for (int i = 0; i < 4; ++i)
	{
		tmp = get(i, 1);
		set(i, 1, get(i, 2));
		set(i, 2, tmp);
	}
	//swap 2nd and 3rd column vectors
	for (int i = 0; i < 4; ++i)
	{
		tmp = get(1, i);
		set(1, i, get(2, i));
		set(2, i, tmp);
	}
}