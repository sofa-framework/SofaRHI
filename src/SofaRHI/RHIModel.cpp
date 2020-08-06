#include <SofaRHI/RHIModel.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/defaulttype/RGBAColor.h>
#include <sofa/helper/types/Material.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/helper/system/FileRepository.h>
#include <SofaRHI/DrawToolRHI.h>

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
    , m_uniformBuffer(nullptr)
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

void RHIModel::updateBuffers()
{
    if(!ready())
        return;
    // ?
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

void RHIModel::updateVertexBuffer(QRhiResourceUpdateBatch* batch)
{
    const VecCoord& vertices = this->getVertices();
    const VecDeriv& vnormals = this->getVnormals();
    const VecTexCoord& vtexcoords = this->getVtexcoords();

    size_t positionsBufferSize, normalsBufferSize;
    size_t textureCoordsBufferSize = 0;

    positionsBufferSize = (vertices.size() * sizeof(vertices[0]));
    normalsBufferSize = (vnormals.size() * sizeof(vnormals[0]));
    textureCoordsBufferSize = (vtexcoords.size() * sizeof(vtexcoords[0]));

    //TODO: Check finally if double or float has an impact on rendering
    //convert vertices to float if needed
    const void* ptrVertices = reinterpret_cast<const void*>(vertices.data());
    helper::vector<sofa::defaulttype::Vec3f> fVertices;
    if (std::is_same<DataTypes::Real, float>::value == false)
    {
        for (const auto& v : vertices)
        {
            fVertices.push_back(sofa::defaulttype::Vec3f(v[0], v[1], v[2]));
        }
        ptrVertices = reinterpret_cast<const void*>(fVertices.data());
        positionsBufferSize = (vertices.size() * sizeof(fVertices[0]));
    }

    //convert normals to float if needed
    const void* ptrNormals = reinterpret_cast<const void*>(vnormals.data());
    helper::vector<sofa::defaulttype::Vec3f> fNormals;
    if (std::is_same<DataTypes::Real, float>::value == false)
    {
        for (const auto& n : vnormals)
        {
            fNormals.push_back(sofa::defaulttype::Vec3f(n[0], n[1], n[2]));
        }
        ptrNormals = reinterpret_cast<const void*>(fNormals.data());
        normalsBufferSize = (vnormals.size() * sizeof(fNormals[0]));
    }

    m_vertexPositionBuffer->setSize(positionsBufferSize + normalsBufferSize + textureCoordsBufferSize );
    batch->updateDynamicBuffer(m_vertexPositionBuffer, 0, positionsBufferSize, ptrVertices);
    batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize, normalsBufferSize, ptrNormals);
    batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize + normalsBufferSize, textureCoordsBufferSize, vtexcoords.data());

    if (!m_vertexPositionBuffer->build())
    {
        msg_error() << "Problem while building vertex buffer";
    }
}

void RHIModel::updateIndexBuffer(QRhiResourceUpdateBatch* batch)
{
    const VecTriangle& triangles = this->getTriangles();
    //const VecQuad& quads = this->getQuads();
    ////convert to triangles
    //VecTriangle quadTriangles;
    //for (const Quad& q : quads)
    //{
    //    quadTriangles.push_back(Triangle(q[0], q[1], q[2]));
    //    quadTriangles.push_back(Triangle(q[2], q[3], q[0]));
    //}

    m_triangleNumber = triangles.size();

    size_t triangleSize = m_triangleNumber * sizeof(triangles[0]);
    m_indexTriangleBuffer->setSize(triangleSize);
    batch->updateDynamicBuffer(m_indexTriangleBuffer, 0, triangleSize, triangles.data());

    if (!m_indexTriangleBuffer->build())
    {
        msg_error() << "Problem while building index buffer";
    }
}

void RHIModel::updateUniformBuffer(QRhiResourceUpdateBatch* batch)
{
    const int UNIFORM_BLOCK_SIZE = 64; // matrix 
    const auto vparams = sofa::core::visual::VisualParams::defaultInstance(); // get from parameters

    QMatrix4x4 qProjectionMatrix, qModelViewMatrix;
    double projectionMatrix[16];
    double modelviewMatrix[16];

    vparams->getProjectionMatrix(projectionMatrix);
    vparams->getModelViewMatrix(modelviewMatrix);
    for (auto i = 0; i < 16; i++)
    {
        qProjectionMatrix.data()[i] = float(projectionMatrix[i]);
        qModelViewMatrix.data()[i] = float(modelviewMatrix[i]);
    }
    QMatrix4x4 projmodelviewMatrix = qProjectionMatrix.transposed() * qModelViewMatrix.transposed();

    batch->updateDynamicBuffer(m_uniformBuffer, 0, UNIFORM_BLOCK_SIZE, projmodelviewMatrix.constData());

    if (!m_uniformBuffer->build())
    {
        msg_error() << "Problem while building uniform buffer";
    }
}

void RHIModel::initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc)
{
    const int UNIFORM_BLOCK_SIZE = 64; // matrix 
    const VecCoord& vertices = this->getVertices();
    const VecDeriv& vnormals = this->getVnormals();
    const VecTexCoord& vtexcoords = this->getVtexcoords();

    // Create Buffers
    m_vertexPositionBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, 0); // set size later (when we know it)
    m_indexTriangleBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::IndexBuffer, 0); // set size later (when we know it)
    m_uniformBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, UNIFORM_BLOCK_SIZE);
    
    // Create Pipeline
    m_srb = rhi->newShaderResourceBindings();
    const QRhiShaderResourceBinding::StageFlags commonVisibility = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
    m_srb->setBindings({
                         QRhiShaderResourceBinding::uniformBuffer(0, commonVisibility, m_uniformBuffer, 0, UNIFORM_BLOCK_SIZE),
        });
    if (!m_srb->build())
    {
        msg_error() << "Problem while building srb";
    }

    m_pipeline = rhi->newGraphicsPipeline();
    QShader vs = loadShader(":/shaders/gl/phong.vert.qsb");
    QShader fs = loadShader(":/shaders/gl/phong.frag.qsb");
    if (!vs.isValid())
    {
        msg_error() << "Problem while vs shader";
    }
    if (!fs.isValid())
    {
        msg_error() << "Problem while fs shader";
    }

    m_pipeline->setShaderStages({ { QRhiShaderStage::Vertex, vs }, { QRhiShaderStage::Fragment, fs } });
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ 
        { 0 } 
    }); // 3 floats vertex + 3 floats normal + 2 floats uv
    inputLayout.setAttributes({ 
        { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
        { 0, 1, QRhiVertexInputAttribute::Float3, quint32(sizeof(float) * 3 * vertices.size()) },
        { 0, 2, QRhiVertexInputAttribute::Float2, quint32(sizeof(float) * 3 * vertices.size() + sizeof(float) * 3 * vnormals.size()) }
    });
    m_pipeline->setVertexInputLayout(inputLayout);
    m_pipeline->setShaderResourceBindings(m_srb);
    m_pipeline->setRenderPassDescriptor(rpDesc.get());
    m_pipeline->setTopology(QRhiGraphicsPipeline::Topology::Triangles);
    if (!m_pipeline->build())
    {
        msg_error() << "Problem while building pipeline";
    }
}

void RHIModel::addResourceUpdate(QRhiResourceUpdateBatch* batch)
{
    if (m_vertexPositionBuffer == nullptr)
    {
        //initRHI not called
        return;
    }

    //Update Buffers
    updateVertexBuffer(batch);
    updateIndexBuffer(batch);
    updateUniformBuffer(batch);
}

void RHIModel::updateRHI(QRhiCommandBuffer* cb, const QRhiViewport& viewport)
{
    if (m_vertexPositionBuffer == nullptr)
    {
        //initRHI not called
        return;
    }

    //Create commands
    cb->setGraphicsPipeline(m_pipeline);
    cb->setShaderResources();
    cb->setViewport(viewport);
    QRhiCommandBuffer::VertexInput vbindings(m_vertexPositionBuffer, 0);
    cb->setVertexInput(0, 1, &vbindings, m_indexTriangleBuffer,0, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(m_triangleNumber * 3);
}

SOFA_DECL_CLASS(RHIModel)

int RHIModelClass = core::RegisterObject("RHIModel")
.add< RHIModel>()
;

} // namespace sofa::rhi
