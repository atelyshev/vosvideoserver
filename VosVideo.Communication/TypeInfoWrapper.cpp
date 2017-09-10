#include "stdafx.h"
#include <cassert>
#include "TypeInfoWrapper.h"

using vosvideo::communication::TypeInfoWrapper;

TypeInfoWrapper::TypeInfoWrapper()
{
	class Nil {};
	tInfo_ = &typeid(Nil);	
	assert(tInfo_);
}

TypeInfoWrapper::TypeInfoWrapper(const std::type_info& tInfo) : tInfo_(&tInfo)
{
	assert(tInfo_);
}


TypeInfoWrapper::~TypeInfoWrapper()
{
}

const std::type_info& TypeInfoWrapper::Get() const
{
	assert(tInfo_);
	return *tInfo_;
}

bool TypeInfoWrapper::before( const TypeInfoWrapper& typeInfoWrapper ) const
{
	assert(tInfo_);
	// type_info::before return type is int in some VC libraries 
	return tInfo_->before(*typeInfoWrapper.tInfo_) != 0;
}

const char* TypeInfoWrapper::name() const
{
	assert(tInfo_);
	return tInfo_->name();
}

bool vosvideo::communication::operator==(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs)
// type_info::operator== return type is int in some VC libraries
{ 
	return (lhs.Get() == rhs.Get()) != 0; 
}

bool vosvideo::communication::operator<(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs)
{ 
	return lhs.before(rhs); 
}

bool vosvideo::communication::operator!=(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs)
{ 
	return !(lhs == rhs); 
}    

bool vosvideo::communication::operator>(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs)
{ 
	return rhs < lhs; 
}

bool vosvideo::communication::operator<=(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs)
{ 
	return !(lhs > rhs); 
}

bool vosvideo::communication::operator>=(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs)
{ 
	return !(lhs < rhs); 
}
