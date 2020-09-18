#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIUtils.h>

namespace sofa::rhi
{
template<typename TVertexType, typename TNormalType, typename TTexcoordType, typename TTriangleType>
struct MeshGenerator
{
    /// http://www.songho.ca/opengl/gl_sphere.html
    static void RoughSphere(std::vector<TVertexType>& vertices, std::vector<TNormalType>& normals, std::vector<TTexcoordType>& texcoords, std::vector<TTriangleType>& indices,
        float radius, int sectors, int stacks);

    static void SmoothSphere(std::vector<TVertexType>& vertices, std::vector<TNormalType>& normals, std::vector<TTexcoordType>& texcoords, std::vector<TTriangleType>& indices,
        float radius, int sectors, int stacks);
};


} // namespace sofa::rhi::utils

