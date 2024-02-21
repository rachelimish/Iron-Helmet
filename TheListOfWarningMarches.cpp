#include "TheListOfWarningMarches.h"

void TheListOfWarningMarches::addAlert(const thread NameAlert)
{
	this->alertList.emplace_back(NameAlert);
}

