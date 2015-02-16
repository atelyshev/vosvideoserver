#pragma once
#include <boost/signals2.hpp>

namespace vosvideo
{
	namespace communication
	{
		class ConnectionProblemNotifier
		{
		public:
			ConnectionProblemNotifier();
			virtual ~ConnectionProblemNotifier();

			// Signal
			virtual boost::signals2::connection ConnectToConnectionProblemSignal(boost::signals2::signal<void()>::slot_function_type subscriber);
		protected:
			// Notify that connection has problem
			boost::signals2::signal<void()> connectionProblemSignal_;

		};
	}
}
