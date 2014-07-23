#define NEEDS_TO_BE_PORTED
#include <string>

#define FBYENUM(c) public enum class c
#define FBYCLASS(c) public ref class c
#define FBYABSTRACTCLASS(c) public ref class c abstract
#define FBYCLASSPTR(c) ref class c; typedef c^ c##Ptr
#define FBYCLASSLITEPTR(c) ref class c; typedef c^ c##Ptr

#define FBYCLASSUNMANAGED(c) FBYCLASS(c)
#define FBYCLASSUNMANAGEDPTR(c) ref class c; typedef c^ c##Ptr

namespace Fby
{
#define BASEOBJINIT(t) "", 0
	FBYCLASS(BaseObj) {
	private:
	public:
		BaseObj(const char *name, int size) {}
		virtual ~BaseObj() {}
	};
}


#define ARRAY(t) array<t>
#define NARRAY(t,n) array<t,n>
#define DYNARRAY(t) System::Collections::ArrayList
#define ARRAYSIZE(a) (a)->Length
#define ARRAYELEM(t,a,n) dynamic_cast<t>(a[(n)])
#define DYNARRAYSIZE(a) (a)->Count
#define DYNARRAYELEM(t,a,n) dynamic_cast<t>(a[(n)])

#define STATICARRAY(type, name, count) array<type> ^name = FBYNEW array<type>(count)

#define STRING System::String ^
#define NEWSTRING(s) FBYNEW System::String(s)
#define STRINGSIZE(s) s->Length
#define FBYNEW gcnew
#define FBYNEWARRAY(t,n) FBYNEW ARRAY(t)(n)
#define FBYNEWDYNARRAY(t,n) FBYNEW DYNARRAY(t)(n)
#define FBYNEWNARRAY(t,n,s) FBYNEW NARRAY(t,n)(s)
#define FBYNULL nullptr
#define FBYINITNULL nullptr
#define FBYPTR(t) t ^
#define RAWPTR(t) FBYPTR(t)
#define FBYPARENTPTR(t) t ^
#define DYNAMIC_CAST(t,o) dynamic_cast<FBYPTR(t)>(o)
#define FBYARRAYPTR(t) FBYPTR(ARRAY(t))
#define FBYDYNARRAYPTR(t) FBYPTR(DYNARRAY(t))
#define FBYPTRNARRAY(t,n) FBYPTR(NARRAY(t,n))
#define THROWEXCEPTION(s) throw FBYNEW System::Exception(s)
typedef System::Exception FbyBaseException;
typedef FbyBaseException^ FbyBaseExceptionPtr;

#define CLEARDYNARRAY(a) a->Clear()
#define ADDDYNARRAY(a,o) a->Add(o)
#define ADDDYNARRAYRANGE(a,o) a->AddRange(o)
#define REMOVEDYNARRAYIDX(a, i) a->RemoveAt(i)
#define REMOVEDYNARRAYRANGE(a, i1, i2) a->RemoveRange(i1, i2 - i1)
#define REMOVEDYNARRAYELEM(a, e) a->Remove(e)

#define POINTF System::Drawing::PointF
#define POINT System::Drawing::Point
#define RECTANGLEF System::Drawing::RectangleF
#define RECTANGLE System::Drawing::Rectangle
#define MATRIX System::Drawing::Drawing2D::Matrix
#define IMAGE System::Drawing::Image
#define BITMAP System::Drawing::Bitmap

namespace FbyHelpers
{
	inline 	System::Drawing::Drawing2D::MatrixOrder MatrixOrder_Append()
	{
		return System::Drawing::Drawing2D::MatrixOrder::Append;
	}
	inline 	System::Drawing::Drawing2D::MatrixOrder MatrixOrder_Prepend()
	{
		return System::Drawing::Drawing2D::MatrixOrder::Prepend;
	}
}

#define FBYTYPEDNULL(t) FBYNULL

typedef FBYPTR(System::Drawing::Font) FbyFontPtr;
typedef FBYPTR(System::Drawing::StringFormat) FbyStringFormatPtr;
typedef POINTF ^PointFPtr;
typedef RECTANGLEF ^RectangleFPtr;
typedef RECTANGLE ^RectanglePtr;
typedef MATRIX ^MatrixPtr;
typedef IMAGE ^ImagePtr;
typedef BITMAP ^BitmapPtr;
#define LOGERROR(s) System::Console::WriteLine(s)


inline BitmapPtr NewBitmap(int width, int height)
{
	BitmapPtr bmp = FBYNEW System::Drawing::Bitmap(width, height, System::Drawing::Imaging::PixelFormat::Format32bppArgb);
	return bmp;
}

inline BitmapPtr NewBitmap(int width, int height, bool opaque)
{
	BitmapPtr bmp = FBYNEW System::Drawing::Bitmap(width, height,
												  opaque ? System::Drawing::Imaging::PixelFormat::Format24bppRgb
												  : System::Drawing::Imaging::PixelFormat::Format32bppArgb);
	return bmp;
}

namespace FbyHelpers
{
	FBYCLASSPTR(FbyDrawingContext);
	FbyDrawingContextPtr DrawingContextFromBitmap(BitmapPtr);
};


#define FbyASSERT(s) if (!(s)) { LOGERROR(#s); THROWEXCEPTION(#s); }
#define FBYMANAGED public ref

namespace FbyHelpers {
	inline int Compare(STRING a, STRING b)
	{
		return a->CompareTo(b);
	}
	inline int Compare(STRING a, const char *v)
	{
		return a->CompareTo(NEWSTRING(v));
	}


#define IsNull(o) ((o) == FBYNULL)
#define ArrayIsNull(o) ((o) == FBYNULL)

typedef System::Byte byte;
typedef System::UInt16 ushort;
}


typedef FBYPTR(System::IO::TextWriter) TextWriterPtr;
typedef System::IO::BinaryWriter ^BinaryWriterPtr;
typedef System::IO::BinaryReader ^BinaryReaderPtr;

#define OVERRIDE override


inline std::string STRING2string(STRING s)
{
	std::string rs;
	for (int i = 0; i < s->Length; ++i)
	{
		rs += (char)(s[i]);
	}
	return rs;
}

inline std::string NumToString(float value)
{
	char ach[16];
	sprintf_s(ach, sizeof(ach), "%f", value);
	return std::string(ach);
}

inline std::string NumToString(bool value)
{
	if (value)
		return std::string("True");
	else
		return std::string("False");
}
inline std::string NumToString(int value)
{
	char ach[16];
	sprintf_s(ach, sizeof(ach),"%d", value);
	return std::string(ach);
}

inline std::string NumToString(int value, int places)
{
	char ach[16];
	sprintf_s(ach, sizeof(ach),"%*.*d", places, places, value);
	return std::string(ach);
}

inline std::string NumToString(int value, int places, int places2)
{
	char ach[16];
	sprintf_s(ach, sizeof(ach),"%*.*d", places, places, value);
	return std::string(ach);
}

inline int Math_Min(int a, int b)
{
	return (a < b) ? a : b;
}

inline int Math_Max(int a, int b)
{
	return (a > b) ? a : b;
}

inline float Math_Min(float a, float b)
{
	return (a < b) ? a : b;
}

inline float Math_Max(float a, float b)
{
	return (a > b) ? a : b;
}

inline float Math_Round(float a)
{
	return (float)(int)(a + .5);
}

inline float Math_Round(float a, int nplaces)
{
	float places = 1;
	
	for (int i = 0; i < nplaces; ++i)
		places *= 10;
	return ((float)(int)(a * places + .5)) / places;
}


int StringToInt(STRING s);
bool StringToInt(STRING s, int *i);
float StringToFloat(STRING s);

#define STATICARRAYSIZE(a) (sizeof(a)/sizeof(*a))

#define PI 3.1415926538

#define STATIC_H_INIT(x) = x
#define STATIC_C_INIT(t,c,v,x)
#define STATIC_C_DECL(t,c,v)
#define FBYNULLINIT (FBYNULL)
#define FBYNULLCONSTRUCTOR (FBYNULL)

typedef System::IO::FileStream ^ FileStreamPtr;
