#include "wikiobjects.h"

using namespace std;


bool Image::HasInstances(FbyDBPtr db)
{
    return LoadInstances(db);
}

bool Image::LoadInstances(FbyDBPtr db)
{
    if (0 == instances.size())
    {
        string sql("SELECT * FROM ImageInstance WHERE imagename=" + db->Quote(imagename)
            + " ORDER BY width");
//        cout << "Loading instance " << sql << endl;
        db->Load(instances, sql.c_str());
        for (auto inst = instances.begin() ; inst != instances.end(); ++inst)
        {
//            cout << "   " << (*inst)->filename << endl;
        }
    }
    return 0 != instances.size();
}

ImageInstancePtr Image::Fullsize(FbyDBPtr db)
{
    if (LoadInstances(db))
    {
        size_t i;
        for (i = instances.size() - 1; i > 0; --i)
        {
            if (instances[i]->width <= 1024)
                break;
        }
        return instances[i];
    }
    return ImageInstancePtr();
}

ImageInstancePtr Image::Original(FbyDBPtr db)
{
    if (LoadInstances(db))
    {
        return instances.back();
    }
    return ImageInstancePtr();
}

const std::vector<ImageInstancePtr> &Image::Instances(FbyDBPtr db)
{
    LoadInstances(db);
    return instances;
}


ImageInstancePtr Image::Thumb(FbyDBPtr db)
{
    if (LoadInstances(db))
        return instances[0];
    return ImageInstancePtr();
}

ImageInstancePtr Image::Medium(FbyDBPtr db)
{
    if (LoadInstances(db))
    {
        if (instances.size() > 1)
            return instances[1];
        else
            return instances[0];
    }
    return ImageInstancePtr();
}

ImageInstancePtr Image::LightboxZoom(FbyDBPtr db)
{
    return Fullsize(db);
}

bool ImageInstance::NeedsDimensionsLoaded()
{
    return false;
}

bool ImageInstance::LoadDimensions()
{
    return true;
}

