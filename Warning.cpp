#include "Warning.h"

Warning::Warning()
{
}

Warning::Warning(string NameAlert)
{
	this->Name = NameAlert;
	this->TimeStart= std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point Warning::GetTimeStart()
{
	return std::chrono::steady_clock::time_point();
}

string Warning::GetName()
{
	return this->Name;
}
