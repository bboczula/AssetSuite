#pragma once

#include <cmath>
#include <iostream>

namespace MiniMath
{
	class Vector3D
	{
	public:
		Vector3D(float x, float y, float z);
		double Length();
		void Normalize();
		friend std::ostream& operator<<(std::ostream& os, const Vector3D& vector)
		{
			os << vector.x << " " << vector.y << " " << vector.z;
			return os;
		}
		float x;
		float y;
		float z;
	};

	class Point3D
	{
	public:
		Point3D(float x, float y, float z);
		Point3D operator-(Point3D a);
		operator Vector3D() const;
		friend std::ostream& operator<<(std::ostream& os, const Point3D& point)
		{
			os << point.x << " " << point.y << " " << point.z;
			return os;
		}
		float x;
		float y;
		float z;
	};

	class Point2D
	{
	public:
		Point2D(float x, float y);
		Point2D operator-(Point2D a);
		friend std::ostream& operator<<(std::ostream& os, const Point2D& point)
		{
			os << point.x << " " << point.y;
			return os;
		}
		float x;
		float y;
	};

	Vector3D CrossProduct(Vector3D a, Vector3D b);
}