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
		private:
			typedef boost::function<ReceivedData*()> factory;
			std::unordered_map<MsgType, factory> factories_;

		public:
			DtoFactory();
			~DtoFactory();

			std::shared_ptr<ReceivedData> Create(MsgType);
		};
	}
}
