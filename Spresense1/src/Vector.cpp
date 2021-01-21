#include "Arduino.h"
#include "Vector.h"


Vector::Vector() {
	for (int i = 0; i < 3; i++) {
		this->val[i] = 0;
	}
}

Vector::Vector(float acc[3]) {
	for (int i = 0; i < 3; i++) {
		this->val[i] = acc[i];
	}
}

Vector::Vector(float x, float y, float z) {
	this->val[0] = x;
	this->val[1] = y;
	this->val[2] = z;
}

Vector::~Vector() {

}

bool Vector::equals(Vector& v){
	for (int i = 0; i < 3; i++) {
		if (this->val[i] != v.val[i])return false;
	}

	return true;
}

const Vector Vector::add(Vector& v){
	Vector ret;
	for (int i = 0; i < 3; i++) {
		ret.val[i] = this->val[i] + v.val[i];
	}
	return ret;
}

const Vector Vector::sub(Vector& v){
	Vector ret;
	for (int i = 0; i < 3; i++) {
		ret.val[i] = this->val[i] - v.val[i];
	}
	return ret;
}

const float Vector::mul(Vector& v){
	float sum = 0;

	for (int i = 0; i < 3; i++) {
		sum += this->val[i] * v.val[i];
	}

	return sum;
}

const Vector Vector::div(float f){
	Vector ret;
	for (int i = 0; i < 3; i++) {
		ret.val[i] = this->val[i] / f;
	}
	return ret;
}
