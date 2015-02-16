#include "stdafx.h"
#include "ConnectionProblemNotifier.h"

using namespace vosvideo::communication;

ConnectionProblemNotifier::ConnectionProblemNotifier()
{
}


ConnectionProblemNotifier::~ConnectionProblemNotifier()
{
}

boost::signals2::connection ConnectionProblemNotifier::ConnectToConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber)
{
	return connectionProblemSignal_.connect(subscriber);
}
