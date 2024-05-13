#include "Solider.h"

Solider::Solider()
{
}

Solider::Solider(double* location)
{
	this->location[0] = location[0];
	this->location[1] = location[1];
	this->location[2] = location[2];
}
 double* Solider::Get()
{
	return location;
}

void Solider::Update(double* location)
{
	this->location[0] = location[0];
	this->location[1] = location[1];
	this->location[2] = location[2];
}
