#include "stdafx.h"
#include "InterprocessCommEngine.h"

using namespace std;
using namespace vosvideo::communication;

InterprocessCommEngine::InterprocessCommEngine(shared_ptr<PubSubService> pubsubService) : 
	pubSubService_(pubsubService)
{
}


InterprocessCommEngine::~InterprocessCommEngine()
{
}
