#pragma once
#include <typeinfo>
#include <memory>

namespace vosvideo
{
	namespace communication
	{
		class TypeInfoWrapper
		{
		public:
			TypeInfoWrapper(void);
			TypeInfoWrapper(const std::type_info&);
			~TypeInfoWrapper(void);

			// Access for the wrapped std::type_info
			const std::type_info& Get() const;
			// Compatibility functions
			bool before(const TypeInfoWrapper& typeInfoWrapper) const;
			const char* name() const;

			// Comparison operators		    
		    friend bool operator==(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs);		    
		    friend bool operator<(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs);		    
		    friend bool operator!=(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs);		    		    
		    friend bool operator>(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs);		    		    
		    friend bool operator<=(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs);		    
		    friend bool operator>=(const TypeInfoWrapper& lhs, const TypeInfoWrapper& rhs);		    
		private:
			 const std::type_info* tInfo_;
		};
	}
}

