///------------------------------------------------------------------------------------------------
///  MeshResource.h
///  StarBird
///
///  Created by Alex Koukoulas on 20/11/2019.
///------------------------------------------------------------------------------------------------

#ifndef MeshResource_h
#define MeshResource_h

///------------------------------------------------------------------------------------------------

#include "IResource.h"
#include "../utils/MathUtils.h"

///------------------------------------------------------------------------------------------------

namespace resources
{

///------------------------------------------------------------------------------------------------

using GLuint = unsigned int;

class MeshResource final: public IResource
{
    friend class OBJMeshLoader;
    
public:
    GLuint GetVertexArrayObject() const;
    GLuint GetElementCount() const;
    const glm::vec3& GetDimensions() const;

private:
    MeshResource(const GLuint vertexArrayObject, const GLuint elementCount, const glm::vec3& meshDimensions);
    
private:
    const GLuint mVertexArrayObject;
    const GLuint mElementCount;
    const glm::vec3 mDimensions;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* MeshResource_h */
