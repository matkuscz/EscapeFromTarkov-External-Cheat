#pragma once

class FVector;
class FRotator;

float DegToRad(float x);
float RadToDeg(float x);
float DistancePointToLine(FVector Point, FVector LineOrigin, FVector Dir);

class FVector
{
public:
	float x, z, y;

	FVector();
	FVector(float x, float y, float z);
	FVector(const FVector& other);

	FVector operator+ (const FVector& other) const;
	FVector operator- (const FVector& other) const;
	FVector operator* (const float other) const;
	float operator* (const FVector& other) const;

	bool operator == (const FVector &other) const;
	bool operator != (const FVector &other) const;

	FVector& operator= (const FVector& other);
	FVector& operator+= (const FVector& other);
	FVector& operator-= (const FVector& other);
	FVector& operator*= (const float other);

	float& operator[](size_t i);
	const float& operator[](size_t i) const;

	float GetLength() const;
	float GetMagnitudeSqr();

	FRotator VectorAngles() const;
};

class FMatrix
{
public:
	FMatrix() : m{
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f },
		{ 0.f, 0.f, 0.f, 0.f } }
	{
	}

	FMatrix(const FMatrix&) = default;


	float* operator[](size_t i) { return m[i]; }
	const float* operator[](size_t i) const { return m[i]; }


	FVector operator*(const FVector & vec);
	FMatrix operator*(const FMatrix &other);
	float m[4][4];
};

class FRotator
{
public:
	float yaw;
	float pitch;
	float roll;

	FRotator();
	FRotator(float pitch, float yaw, float roll);
	FRotator(const FRotator& other);

	void ToSourceAngles();
	void ToUnityAngles();
	void Normalize();
	FVector AngleVector();
	void AngleVectors(FVector* x, FVector* y, FVector* z);
};

struct FQuat
{
	float x;
	float y;
	float z;
	float w;

	FQuat operator*(const FQuat& other);
};

struct FTransform
{
public:
	FQuat Rotation;
	FVector Translation; 
private:
	float pad0;
public:
	FVector Scale3D;
private:
	float pad1;

public:
	FMatrix ToMatrixWithScale();
	
};

struct FBoxSphereBounds
{
	FVector	Origin;
	FVector BoxExtent;
	float SphereRadius;
};