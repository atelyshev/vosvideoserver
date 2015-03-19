#pragma once
#include <boost/signals2.hpp>

namespace vosvideo
{
	namespace archive
	{
		class ChangesNotifier
		{
		public:
			ChangesNotifier();
			virtual ~ChangesNotifier() = 0;

			boost::signals2::connection ConnectToChangesSignal(boost::signals2::signal<void (const std::wstring&)>::slot_function_type subscriber);
			boost::signals2::signal<void (const std::wstring&)> notifierSignal_;
		};
	}
}
