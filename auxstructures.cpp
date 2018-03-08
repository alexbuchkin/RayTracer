#include "auxstructures.hpp"

Triangle::Triangle(){}

Triangle::Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
{
	v[0] = p0; v[1] = p1; v[2] = p2;
}

Triangle&
Triangle::operator= (const Triangle& tr)
{
	v[0] = tr.v[0];
	v[1] = tr.v[1];
	v[2] = tr.v[2];
	return *this;
}
