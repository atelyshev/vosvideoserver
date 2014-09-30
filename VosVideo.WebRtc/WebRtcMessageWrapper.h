#pragma once
#include "VosVideo.Data/ReceivedData.h"

namespace vosvideo
{
	namespace vvwebrtc
	{
		template<class T>
		class WebRtcMessageWrapper
		{
		public:
			WebRtcMessageWrapper(std::shared_ptr<T> receivedData) : 
				receivedData_(receivedData){}

			~WebRtcMessageWrapper(){}

			std::shared_ptr<T> GetData()
			{
				return receivedData_;
			}

		private:
			std::shared_ptr<T> receivedData_;
		};
	}
}
