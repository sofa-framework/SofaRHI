#include <SofaRHI/RHIModel.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/defaulttype/RGBAColor.h>
#include <sofa/helper/types/Material.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/helper/system/FileRepository.h>

namespace sofa::rhi
{

using sofa::defaulttype::RGBAColor;
using namespace sofa;
using namespace sofa::component::visualmodel;

RHIModel::RHIModel()
    : InheritedVisual()
    , m_isReady(false)
    , m_bTopologyHasChanged(false)
    , m_vertexPositionBuffer(nullptr)
    , m_indexTriangleBuffer(nullptr)
    , m_indexQuadBuffer(nullptr)
    , m_indexEdgeBuffer(nullptr)
    , m_indexWireframeBuffer(nullptr)
{
}

void RHIModel::init() 
{
    InheritedVisual::init();
}

void RHIModel::updateVisual()
{
    //update geometry
    InheritedVisual::updateVisual();

}


void RHIModel::createVertexBuffer()
{
    //m_vertexPositionBuffer = new Qt3DRender::QBuffer(getRootEntity());
    //m_vertexNormalBuffer = new Qt3DRender::QBuffer(getRootEntity());
    //m_vertexTexcoordBuffer = new Qt3DRender::QBuffer(getRootEntity());
    initVertexBuffer();

}

void RHIModel::createEdgesIndicesBuffer()
{
    //m_indexEdgeBuffer = new Qt3DRender::QBuffer(getRootEntity());
    updateEdgesIndicesBuffer();
}

void RHIModel::createTrianglesIndicesBuffer()
{
    //m_indexTriangleBuffer = new Qt3DRender::QBuffer(getRootEntity());
    updateTrianglesIndicesBuffer();
}


void RHIModel::createQuadsIndicesBuffer()
{
    //m_indexQuadBuffer = new Qt3DRender::QBuffer(getRootEntity());
    updateQuadsIndicesBuffer();
}

void RHIModel::initVertexBuffer()
{
    const VecCoord& vertices = this->getVertices();
    const VecDeriv& vnormals = this->getVnormals();
    const VecTexCoord& vtexcoords = this->getVtexcoords();
    updateVertexBuffer();

    //m_positionAttribute = new Qt3DRender::QAttribute();
    //m_positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    //m_positionAttribute->setBuffer(m_vertexPositionBuffer);
    //m_positionAttribute->setVertexBaseType(VertexType);
    //m_positionAttribute->setVertexSize(3);
    //m_positionAttribute->setByteOffset(0);
    //m_positionAttribute->setByteStride(0);
    //m_positionAttribute->setCount(vertices.size());
    //m_positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    //m_normalAttribute = new Qt3DRender::QAttribute();
    //m_normalAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    //m_normalAttribute->setBuffer(m_vertexNormalBuffer);
    //m_normalAttribute->setVertexBaseType(NormalType);
    //m_normalAttribute->setVertexSize(3);
    //m_normalAttribute->setByteOffset(0);
    //m_normalAttribute->setByteStride(0);
    //m_normalAttribute->setCount(vnormals.size());
    //m_normalAttribute->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());

    //m_texcoordAttribute = new Qt3DRender::QAttribute();
    //m_texcoordAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    //m_texcoordAttribute->setBuffer(m_vertexTexcoordBuffer);
    //m_texcoordAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    //m_texcoordAttribute->setVertexSize(2);
    //m_texcoordAttribute->setByteOffset(0);
    //m_texcoordAttribute->setByteStride(0);
    //m_texcoordAttribute->setCount(vtexcoords.size());
    //m_texcoordAttribute->setName(Qt3DRender::QAttribute::defaultTextureCoordinateAttributeName());
}

void RHIModel::updateVertexBuffer()
{
    //const VecCoord& vertices = this->getVertices();
    //const VecDeriv& vnormals = this->getVnormals();
    //const VecTexCoord& vtexcoords = this->getVtexcoords();

    //unsigned positionsBufferSize, normalsBufferSize;
    //unsigned textureCoordsBufferSize = 0;

    //positionsBufferSize = (vertices.size() * sizeof(vertices[0]));
    //normalsBufferSize = (vnormals.size() * sizeof(vnormals[0]));
    //textureCoordsBufferSize = (vtexcoords.size() * sizeof(vtexcoords[0]));

    ////convert vertices to float if needed
    //const char* ptrVertices = reinterpret_cast<const char*>(vertices.data());
    //helper::vector<sofa::defaulttype::Vec3f> fVertices;
    //if(std::is_same<DataTypes::Real, float>::value == false)
    //{
    //    for(const auto& v : vertices)
    //    {
    //        fVertices.push_back(sofa::defaulttype::Vec3f(v[0],v[1],v[2]));
    //    }
    //    ptrVertices = reinterpret_cast<const char*>(fVertices.data());
    //    positionsBufferSize = (vertices.size() * sizeof(fVertices[0]));
    //}

    ////convert normals to float if needed
    //const char* ptrNormals = reinterpret_cast<const char*>(vnormals.data());
    //helper::vector<sofa::defaulttype::Vec3f> fNormals;
    //if(std::is_same<DataTypes::Real, float>::value == false)
    //{
    //    for(const auto& n : vnormals)
    //    {
    //        fNormals.push_back(sofa::defaulttype::Vec3f(n[0],n[1],n[2]));
    //    }
    //    ptrNormals = reinterpret_cast<const char*>(fNormals.data());
    //    normalsBufferSize = (vnormals.size() * sizeof(fNormals[0]));
    //}

    //QByteArray qbaPosition(ptrVertices, positionsBufferSize);
    //m_vertexPositionBuffer->setData(qbaPosition);
    //QByteArray qbaNormal(ptrNormals, normalsBufferSize);
    //m_vertexNormalBuffer->setData(qbaNormal);
    //QByteArray qbaTexcoord(reinterpret_cast<const char*>(vtexcoords.data()), textureCoordsBufferSize);
    //m_vertexTexcoordBuffer->setData(qbaTexcoord);

}

void RHIModel::updateEdgesIndicesBuffer()
{
    const VecEdge& edges = this->getEdges();
    //QByteArray qbaEdges(reinterpret_cast<const char*>(edges.data()), edges.size() * 2 * sizeof(unsigned int));
    //m_indexEdgeBuffer->setData(qbaEdges);
}

void RHIModel::updateTrianglesIndicesBuffer()
{
    const VecTriangle& triangles = this->getTriangles();
    //QByteArray qbaTriangles(reinterpret_cast<const char*>(triangles.data()), triangles.size() * 3 * sizeof(unsigned int));
    //m_indexTriangleBuffer->setData(qbaTriangles);
}

void RHIModel::updateQuadsIndicesBuffer()
{
    const VecQuad& quads = this->getQuads();
    //convert to triangles
    VecTriangle quadTriangles;
    for (const Quad& q : quads)
    {
        quadTriangles.push_back(Triangle(q[0], q[1], q[2]));
        quadTriangles.push_back(Triangle(q[2], q[3], q[0]));
    }

    //QByteArray qbaQuads(reinterpret_cast<const char*>(quadTriangles.data()), quadTriangles.size() * 3 * sizeof(unsigned int));
    //m_indexQuadBuffer->setData(qbaQuads);
}

void RHIModel::updateBuffers()
{
    if(!ready())
        return;

    const VecEdge& edges = this->getEdges();
    const VecTriangle& triangles = this->getTriangles();
    const VecQuad& quads = this->getQuads();
    const VecCoord& vertices = this->getVertices();
    const VecDeriv& normals = this->getVnormals();
    const VecTexCoord& texCoords = this->getVtexcoords();
    const VecCoord& tangents = this->getVtangents();
    const VecCoord& bitangents = this->getVbitangents();
    bool topologyHasChanged = false;

    if (!m_vertexPositionBuffer)
    {
        createVertexBuffer();
    }
    else
    {
        updateVertexBuffer();
    }

    //Indices
    //Edges
    if (m_indexEdgeBuffer != nullptr)
    {
        // will just detect if the size change
        if (m_oldEdgeSize != edges.size())
        {
            updateEdgesIndicesBuffer();
        }
    }
    else if (edges.size() > 0)
    {
        createEdgesIndicesBuffer();
    }

    //Triangles
    if (m_indexTriangleBuffer != nullptr)
    {
        if (m_oldTriangleSize != triangles.size())
        {
            updateTrianglesIndicesBuffer();
        }
    }
    else if (triangles.size() > 0)
    {
        createTrianglesIndicesBuffer();
    }

    //Quads
    if (m_indexQuadBuffer != nullptr)
    {
        if (m_oldQuadSize != quads.size())
        {
            updateQuadsIndicesBuffer();
        }
    }
    else if (quads.size() > 0)
    {
        createQuadsIndicesBuffer();
    }

    m_oldPositionSize = vertices.size();
    m_oldNormalSize = normals.size();
    m_oldEdgeSize = edges.size();
    m_oldTriangleSize = triangles.size();
    m_oldQuadSize = quads.size();

    if(m_bTopologyHasChanged)
    {
        //m_positionAttribute->setCount(vertices.size());
        //m_normalAttribute->setCount(normals.size());
        //m_texcoordAttribute->setCount(texCoords.size());


        m_bTopologyHasChanged = false;
    }
}


void RHIModel::internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent)
{
    if(!ready())
        return;

    //todo: cache variables to not set flags every time
    //if(!vparams->displayFlags().getShowVisual())
    //{
    //    //m_wireframeEntity->setEnabled(false);
    //    setGeometriesEnabled(false);
    //}
    //else
    //{
    //    setGeometriesEnabled(true);

    //    if(vparams->displayFlags().getShowWireFrame())
    //    {
    //        setWireframeEnabled(true);
    //    }
    //    else
    //    {
    //        setWireframeEnabled(false);
    //    }
    //}


}


void RHIModel::handleTopologyChange()
{
    std::list<const sofa::core::topology::TopologyChange *>::const_iterator itBegin=m_topology->beginChange();
    std::list<const sofa::core::topology::TopologyChange *>::const_iterator itEnd=m_topology->endChange();

    m_bTopologyHasChanged = (itBegin != itEnd);

    InheritedVisual::handleTopologyChange();

    updateBuffers();

}

SOFA_DECL_CLASS(RHIModel)

int RHIModelClass = core::RegisterObject("RHIModel")
.add< RHIModel>()
;

} // namespace sofa::rhi
