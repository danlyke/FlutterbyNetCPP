#include "StdAfx.h"

#include <math.h>
#include <float.h>
#include <limits.h>
#include "fby_core_unix.h"

using namespace std;

namespace boost
{

	void intrusive_ptr_add_ref(FbyHelpers::BaseObj * p)
	{
		// increment reference count of object *p

		++(p->baseObjReferences);
	}



	void intrusive_ptr_release(FbyHelpers::BaseObj * p)
	{
		// decrement reference count, and delete object when reference count reaches 0

		if (--(p->baseObjReferences) == 0)
			delete p;
			//p->Wipe();
	} 


} // namespace boost



FbyHelpers::BaseObj::BaseObj(const char *name, int size)
	: baseObjReferences(0), baseObjName(name), baseObjSize(size)
{
}

void FbyHelpers::BaseObj::Wipe()
{
	const char *name = baseObjName;
	int size = baseObjSize;

	memset(this, 0x00, size);
	baseObjSize = size;
	baseObjName = name;
}

FbyHelpers::BaseObj::~BaseObj()
{
	Wipe();
}


