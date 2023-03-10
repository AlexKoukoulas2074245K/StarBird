///------------------------------------------------------------------------------------------------
///  IResource.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef IResource_h
#define IResource_h

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

class IResource
{
public:
    virtual ~IResource() = default;
    IResource(const IResource&) = delete;
    const IResource& operator = (const IResource&) = delete;
    
protected:
    IResource() = default;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* IResource_h */
