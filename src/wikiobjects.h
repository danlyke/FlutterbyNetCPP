#ifndef WIKIOBJECTS_H_INCLUDED
#define WIKIOBJECTS_H_INCLUDED

#include "fbydb.h"
#include <string>
#include <float.h>

FBYCLASSPTR(StatusUpdate);
FBYCLASS(StatusUpdate) : public FbyORM /* nosql */ {
    FBYORM_SQLOBJECT;
public:
StatusUpdate() : FbyORM(BASEOBJINIT(StatusUpdate)),
        id(),
        status(),
        entered(),
        locationset(),
        latitude(),
        longitude(),
        twitter_updated(),
        facebook_updated(),
        flutterbynetlog_updated(),
        posaccuracy(),
        twitter_update(),
        facebook_update(),
        person_id(),
        person(),
        flutterby_update(),
        flutterby_updated(),
        identica_update(),
        identica_updated(),
        imagename(),
        thumbnailpath(),
        thumbnailwidth(),
        thumbnailheight(),
        uniquifer(),
        xid(),
        publishdelay(),
        twitterid()
        {}
    int id; // SQL INTEGER PRIMARY KEY,
    std::string status; // SQL TEXT
    time_t entered; // SQL TIMESTAMP,
    bool locationset; // SQL BOOLEAN
    double latitude; // SQL double precision             
    double longitude; // SQL double precision             
    bool twitter_updated; // SQL boolean                      default false
    bool facebook_updated; // SQL boolean                      default false
    bool flutterbynetlog_updated; // SQL boolean                      default false
    double posaccuracy; // SQL double precision             default 0
    bool twitter_update; // SQL boolean                      default false
    bool facebook_update; // SQL boolean                      default false
    int person_id; // SQL integer                      
    std::string person; // SQL TEXT
    bool flutterby_update; // SQL boolean                      default false
    bool flutterby_updated; // SQL boolean                      default false
    bool identica_update; // SQL boolean                      default false
    bool identica_updated; // SQL boolean                      default false
    std::string imagename; // SQL text                         
    std::string thumbnailpath; // SQL text                         
    int thumbnailwidth; // SQL integer                      
    int thumbnailheight; // SQL integer                      
    int uniquifer; // SQL integer                      default (-1)
    std::string xid; // SQL text                         
    double publishdelay; // SQL double precision             default 0
    std::string twitterid;
};

FBYCLASSPTR(Camera);

FBYCLASS(Camera)  : public FbyORM {
    FBYORM_SQLOBJECT;
public:
Camera() : FbyORM(BASEOBJINIT(Camera)),
        name()
        {};
    std::string name; // SQL TEXT PRIMARY KEY,
    // SQL unique index name
};

FBYCLASSPTR(ImageInstance);
FBYCLASSPTR(Image);

FBYCLASS(Image) : public FbyORM
{
   FBYORM_SQLOBJECT;
public:
Image() : FbyORM(BASEOBJINIT(Image)),
        imagename(),
//        longitude(-FLT_MAX),
        longitude(-0),
//        exposuretime(-FLT_MAX),
        exposuretime(-0),
        camera_name(),
//        aperture(-FLT_MAX),
        aperture(-0),
//        set_flags(0),
        set_flags(0),
//        focallength(-FLT_MAX),
        focallength(-0),
        exiftime(0),
//        focusdist(-FLT_MAX),
        focusdist(-0),
//        latitude(-FLT_MAX),
        latitude(-0),
        calctime(0),
//       altitude(-FLT_MAX),
       altitude(-0),
       instances()
    {}
public :
   ImageInstancePtr Fullsize(FbyDBPtr);
   ImageInstancePtr Thumb(FbyDBPtr);
   ImageInstancePtr LightboxZoom(FbyDBPtr);
   bool HasInstances(FbyDBPtr);
   ImageInstancePtr Original(FbyDBPtr);
   const std::vector<ImageInstancePtr> &Instances(FbyDBPtr);

protected:
   std::string imagename; // SQL TEXT PRIMARY KEY,
   double longitude; // SQL FLOAT,
   double exposuretime; // SQL FLOAT,
   std::string camera_name; // SQL TEXT REFERENCES camera(name),
   double aperture; // SQL FLOAT,
   int set_flags; // SQL INTEGER,
   double focallength; // SQL FLOAT,
   time_t exiftime; // SQL TIMESTAMP,
   double focusdist; // SQL FLOAT,
   double latitude; // SQL FLOAT,
   time_t calctime; // SQL TIMESTAMP,
   double altitude; // SQL FLOAT

   bool LoadInstances(FbyDBPtr);
   std::vector<ImageInstancePtr> instances;
};


FBYCLASSPTR(ImageInstance);

FBYCLASS(ImageInstance) : public FbyORM 
{
    FBYORM_SQLOBJECT;
public:
ImageInstance() : FbyORM(BASEOBJINIT(ImageInstance)),
        filename(),
        width(-1),
        height(-1),
        mtime(0),
        imagename()
    {}
public:
    std::string filename; // SQL TEXT PRIMARY KEY,
    int width; // SQL INTEGER,
    int height; // SQL INTEGER
    time_t mtime; // SQL TIMESTAMP,
    std::string imagename; // SQL TEXT REFERENCES Image(imagename)

    bool NeedsDimensionsLoaded();
    bool LoadDimensions();
};


FBYCLASSPTR(WikiEntry);

FBYCLASS(WikiEntry) : public FbyORM
{
    FBYORM_SQLOBJECT;
public:
WikiEntry() : FbyORM(BASEOBJINIT(WikiEntry)),
        wikiname(),
        inputname(),
        needsContentRebuild(0),
        needsExternalRebuild(0)
        {};
public:
    std::string wikiname; // SQL TEXT PRIMARY KEY,
    std::string inputname; // SQL TEXT,
    int needsContentRebuild; // SQL INTEGER,
    int needsExternalRebuild; // SQL INTEGER,
};

FBYCLASSPTR(WikiEntryReference);

FBYCLASS(WikiEntryReference) : public FbyORM
{
    FBYORM_SQLOBJECT;
public:
WikiEntryReference() :
    FbyORM(BASEOBJINIT(WikiEntryReference)),
        from_wikiname(),
        to_wikiname(),
        needsdeletion(false)
        {};
    std::string from_wikiname; // SQL TEXT REFERENCES WikiEntry(wikiname), -- key
    std::string to_wikiname; // SQL TEXT REFERENCES WikiEntry(wikiname), -- key
    bool needsdeletion; // SQL BOOLEAN,
};
// SQL: CREATE UNIQUE INDEX WikiEntryReference_From_To ON WikiEntryReference(from_wikiname, to_wikiname);


FBYCLASSPTR(CameraTime);

FBYCLASS(CameraTime) : public FbyORM {
    FBYORM_SQLOBJECT;
CameraTime() : FbyORM(BASEOBJINIT(CameraTime)),
        camera_name(),
        insertiontime(0),
        cameratime(0),
        gpstime(0)
    {};
    std::string camera_name; // SQL TEXT REFERENCES Camera(name)
    time_t insertiontime; // SQL TIMESTAMP
    time_t cameratime; // SQL TIMESTAMP
    time_t gpstime; // SQL TIMESTAMP
};


#endif /* #ifndef WIKIOBJECTS_H_INCLUDED */
