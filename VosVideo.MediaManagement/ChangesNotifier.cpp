#include "stdafx.h"
#include "ChangesNotifier.h"

using namespace std;
using namespace boost::signals2;
using namespace vosvideo::archive;

ChangesNotifier::ChangesNotifier()
{
}

ChangesNotifier::~ChangesNotifier()
{
}

connection ChangesNotifier::ConnectToChangesSignal(signal<void (const wstring&)>::slot_function_type subscriber)
{
	return notifierSignal_.connect(subscriber);
}
