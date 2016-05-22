#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <math/Vector3f.h>
#include <math/Matrix4f.h>
#include <math/Quaternion.h>


class ICamera
{
public:
	virtual Quaternion GetOrientation() const = 0;
	virtual Vector3f GetViewPosition() const = 0;
	virtual Matrix4f GetViewProjection() const = 0;
	virtual Matrix4f GetProjection() const = 0;
	virtual void SetProjection(const Matrix4f& projection) = 0;
};


class ArcBallCamera : public ICamera
{
public:
	Matrix4f	projection;
	Vector3f	position;
	Quaternion	rotation;
	float		distance;

	Vector3f GetViewPosition() const override
	{
		return (position - (rotation.GetForward() * distance));
	}
	
	Quaternion GetOrientation() const override
	{
		return rotation;
	}
	
	Matrix4f GetProjection() const override
	{
		return projection;
	}
	
	void SetProjection(const Matrix4f& projection) override
	{
		this->projection = projection;
	}
	

	Matrix4f GetViewProjection() const override
	{
		Vector3f pos = position - (rotation.GetForward() * distance);
		Matrix4f viewMatrix = Matrix4f::CreateRotation(rotation.GetConjugate()) *
			Matrix4f::CreateTranslation(-pos);
		return (projection * viewMatrix);
	}
};


class Camera : public ICamera
{
public:
	Matrix4f	projection;
	Vector3f	position;
	Quaternion	rotation;

	Vector3f GetViewPosition() const  override
	{
		return position;
	}
	
	Quaternion GetOrientation() const override
	{
		return rotation;
	}
	
	void SetProjection(const Matrix4f& projection) override
	{
		this->projection = projection;
	}
	
	Matrix4f GetProjection() const override
	{
		return projection;
	}

	Matrix4f GetViewProjection() const override
	{
		Matrix4f viewMatrix = Matrix4f::CreateRotation(rotation.GetConjugate()) *
			Matrix4f::CreateTranslation(-position);
		return (projection * viewMatrix);
	}
};


#endif // _CAMERA_H_