#pragma once
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include "ReceivedData.h"
#include "MsgTypes.h"

/*
 * Boost factory implementation
*/
namespace vosvideo
{
	namespace data
	{
		class DtoFactory final
		{
		public:
			DtoFactory();
			virtual ~DtoFactory();

			std::shared_ptr<ReceivedData> Create(MsgType);

		private:
			using factory = boost::function<ReceivedData*()>;
			std::unordered_map<MsgType, factory> factories_;
		};
	}
}
