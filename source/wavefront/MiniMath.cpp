#include "pch.h"
#include "MiniMath.h"

MiniMath::Point2D::Point2D(float x, float y) : x(x), y(y)
{
}

MiniMath::Point2D MiniMath::Point2D::operator-(Point2D a)
{
	Point2D temp = { 0.0f, 0.0f };
	temp.x = x - a.x;
	temp.y = y - a.y;
	return temp;
}

MiniMath::Point3D::Point3D(float x, float y, float z) : x(x), y(y), z(z)
{
}

MiniMath::Point3D MiniMath::Point3D::operator-(Point3D a)
{
	Point3D temp = { 0.0f, 0.0f, 0.0f };
	temp.x = x - a.x;
	temp.y = y - a.y;
	temp.z = z - a.z;
	return temp;
}

MiniMath::Point3D::operator MiniMath::Vector3D() const
{
	return Vector3D(x, y, z);
}

MiniMath::Vector3D MiniMath::CrossProduct(Vector3D a, Vector3D b)
{
	Vector3D result = { 0.0f, 0.0f, 0.0f };
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

MiniMath::Vector3D::Vector3D(float x, float y, float z) : x(x), y(y), z(z)
{
}

double MiniMath::Vector3D::Length()
{
	return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));;
}

void MiniMath::Vector3D::Normalize()
{
	auto length = Length();
	x /= length;
	y /= length;
	z /= length;
}
