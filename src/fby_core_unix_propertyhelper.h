#ifndef FBY_CORE_UNIX_PROPERTYHELPER_H_INCLUDED
#define FBY_CORE_UNIX_PROPERTYHELPER_H_INCLUDED
#include <stdio.h>
#include <string.h>
#include <assert.h>

	template <typename Container, typename ValueType> class PropertyHelper
	{
	public:
		PropertyHelper(Container * const containerObject = NULL,
					   ValueType (Container::*Get)() const = NULL,
					   void (Container::*Set)(ValueType value) = NULL) :
			containerObject(const_cast<Container*>(containerObject)), Get(Get), Set(Set)
		{
		}

		ValueType operator =(const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Set != NULL);
			(containerObject->*Set)(value);
			return value;
		}
#if 0
		ValueType operator =(PropertyHelper<Container,ValueType> &rhs)
		{
			assert(containerObject != NULL);
			assert(Set != NULL);
			ValueType value = (rhs.*Get)();
			(containerObject->*Set)(value);
			return value;
		}
#endif
		ValueType operator +=(const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Set != NULL);
			assert(Get != NULL);
			(containerObject->*Set)((containerObject->*Get)() + value);
			return value;
		}


		bool operator < (const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Get != NULL);
			return (containerObject->*Get)() < value;
		}

		bool operator > (const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Get != NULL);
			return (containerObject->*Get)() > value;
		}
		bool operator <= (const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Get != NULL);
			return (containerObject->*Get)() <= value;
		}

		bool operator >= (const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Get != NULL);
			return (containerObject->*Get)() >= value;
		}

		ValueType operator -=(const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Set != NULL);
			assert(Get != NULL);
			(containerObject->*Set)((containerObject->*Get)() - value);
			return value;
		}
		ValueType operator /=(const ValueType& value)
		{
			assert(containerObject != NULL);
			assert(Set != NULL);
			assert(Get != NULL);
			(containerObject->*Set)((containerObject->*Get)() / value);
			return value;
		}

		operator ValueType() const
		{
			assert(containerObject != NULL);
			assert(Get != NULL);
			return (containerObject->*Get)();
		}
	private:
		Container* containerObject;  //-- Pointer to the module that
		ValueType (Container::*Get)() const;
		void (Container::*Set)(ValueType value);
	};

#endif // ifndef FBY_CORE_UNIX_PROPERTYHELPER_H_INCLUDED
