/*

SDG                                                                                               JJ

                                               Orc Horde

										Math Functions and Types
*/

#pragma once

#include <glm/glm.hpp> // TODO(caleb): get rid of GLM
#include <cmath>
#include <numbers>

typedef glm::vec2 skyVec2; //  FIXME(caleb): you're a trained mathematician. WRITE YOUR OWN MATH
typedef glm::vec3 skyVec3; // TODO(caleb): seriously, write your own math library
typedef glm::vec4 skyVec4; // TODO(caleb): seriously, write your own math library

struct skyQuat {
  float w;
  float x;
  float y;
  float z;

  const skyQuat operator+(const skyQuat q) {
	return skyQuat { .w = w + q.w,
					 .x = x + q.x,
					 .y = y + q.y,
					 .z = z + q.z, };
  }
	  

  const skyQuat operator*(const skyQuat q) const {
 	auto V = skyVec3( x, y, z );
	auto Q = skyVec3( q.x, q.y, q.z );

	skyVec3 Vprime = q.w * V + w * Q + glm::cross(V, Q);
	
	return skyQuat { .w = w * q.w - glm::dot(V, Q),
					 .x = Vprime.x,
					 .y = Vprime.y,
					 .z = Vprime.z, };
  }

  const skyQuat operator*(const float f) const {
	return skyQuat { .w = w * f,
					 .x = x * f,
					 .y = y * f,
					 .z = z * f, };
  }

  const skyQuat operator+=(const skyQuat q) {
	*this = *this + q;
	return *this;
  }

  // multiply on the right (this means rotate in object space)
  const skyQuat operator*=(const skyQuat q) {
	*this = *this * q;
	return *this;
  }

  const skyQuat operator*=(const float f) {
	*this = *this * f;
	return *this;
  }

  const skyQuat normalize() {
	float norm = std::sqrt( w * w +
							x * x +
							y * y +
							z * z   );
	assert(norm > 0.0);

	*this *= (1.0 / norm);
	return *this;	
  }

  const skyVec3 pureQuat() {
	return skyVec3(x, y, z);
  }

  static const skyQuat unitVec() {
	skyQuat q;
	q.x = q.y = q.z = 0;
	q.w = 1;
	return q;
  }

  static const skyQuat fromAngle(skyVec3 v, float theta) {
	auto s = skyQuat { .w = std::cos(theta / 2),
					   .x = std::sin(theta / 2),
					   .y = std::sin(theta / 2),
					   .z = std::sin(theta / 2), };
	s.normalize();
	return s;
  }

  explicit operator skyVec4() const { return skyVec4(w, x, y, z); }
};
