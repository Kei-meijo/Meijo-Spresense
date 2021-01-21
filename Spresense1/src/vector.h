#ifndef _VEC_H_
#define _VEC_H_
#include "Arduino.h"


class Vector {
	
	
public:
	float val[3];
	Vector(float acc[3]);
	Vector();
	Vector(float x, float y, float z);
	~Vector();

	const Vector add(Vector& v);
	const Vector sub(Vector& v);
	const float mul(Vector& v);
	const Vector div(float f);

	

	bool equals(Vector& v);
	

};

#endif
