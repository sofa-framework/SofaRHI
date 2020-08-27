#include <SofaRHI/RHIModel.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/defaulttype/RGBAColor.h>
#include <sofa/helper/types/Material.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/helper/system/FileRepository.h>
#include <SofaRHI/DrawToolRHI.h>
#include <SofaRHI/RHIUtils.h>

namespace sofa::rhi
{

using sofa::defaulttype::RGBAColor;
using namespace sofa;
using namespace sofa::component::visualmodel;

RHIModel::RHIModel()
    : InheritedVisual()
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


void RHIModel::initVisual()
{ 
    InheritedVisual::initVisual(); 

    sofa::core::objectmodel::BaseObject::d_componentState.setValue(sofa::core::objectmodel::ComponentState::Invalid);

    // I suppose it would be better to get the visualParams given as params but it is only in update/draw steps
    rhi::DrawToolRHI* rhiDrawTool = dynamic_cast<rhi::DrawToolRHI*>(sofa::core::visual::VisualParams::defaultInstance()->drawTool());

    if (rhiDrawTool == nullptr)
    {
        msg_error("RHIModel") << "Can only works with RHIViewer as gui; DrawToolRHI not detected.";
        return;
    }

    QRhiPtr rhi = rhiDrawTool->getRHI();
    QRhiRenderPassDescriptorPtr rpDesc = rhiDrawTool->getRenderPassDescriptor();
    if (!initRHI(rhi, rpDesc))
    {
        msg_error() << "Error while initializing RHI features";
        return;
    }

    sofa::core::objectmodel::BaseObject::d_componentState.setValue(sofa::core::objectmodel::ComponentState::Valid);
}

void RHIModel::updateVisual()
{
    if (d_componentState.getValue() != sofa::core::objectmodel::ComponentState::Valid)
        return;

    // we bypass the updateBuffers part because bugus and we want to custom anyway
    //update geometry
    InheritedVisual::updateVisual();

    //a bit of inconsistency with the vparams thing
    rhi::DrawToolRHI* rhiDrawTool = dynamic_cast<rhi::DrawToolRHI*>(sofa::core::visual::VisualParams::defaultInstance()->drawTool());
    QRhiResourceUpdateBatch* batch = rhiDrawTool->getResourceUpdateBatch();

    if (batch != nullptr)
        updateRHIResources(batch);
}

void RHIModel::updateBuffers()
{
    if (d_componentState.getValue() != sofa::core::objectmodel::ComponentState::Valid)
        return;

    m_updateGeometry = true; //modified is ...modified by InheritedVisual::updateVisual

}


void RHIModel::internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent)
{
    if (d_componentState.getValue() != sofa::core::objectmodel::ComponentState::Valid)
    {
        return;
    }
    if(!vparams->displayFlags().getShowVisual())
    {
        return;
    }
    //if (!m_bDidUpdateBufferWorkAround)
    //{
    //    return;
    //}

    rhi::DrawToolRHI* rhiDrawTool = dynamic_cast<rhi::DrawToolRHI*>(vparams->drawTool());
    QRhiCommandBuffer* cb = rhiDrawTool->getCommandBuffer();
    const QRhiViewport& viewport = rhiDrawTool->getViewport();

    updateRHICommands(cb,viewport);

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
    auto itBegin=m_topology->beginChange();
    auto itEnd=m_topology->endChange();

    m_bTopologyHasChanged = (itBegin != itEnd);

    InheritedVisual::handleTopologyChange();

    updateBuffers();

}

void RHIModel::updateVertexBuffer(QRhiResourceUpdateBatch* batch)
{
    const auto& vertices = this->getVertices();
    const auto& vnormals = this->getVnormals();
    const auto& vtexcoords = this->getVtexcoords();

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

    m_positionsBufferSize = positionsBufferSize;
    m_normalsBufferSize = normalsBufferSize;
    m_textureCoordsBufferSize = textureCoordsBufferSize;
}

void RHIModel::updateIndexBuffer(QRhiResourceUpdateBatch* batch)
{
    const auto& triangles = this->getTriangles();
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
    const auto inverseModelViewMatrix = qModelViewMatrix.inverted();

    const defaulttype::Vec3f cameraPosition{ inverseModelViewMatrix.data()[3], inverseModelViewMatrix.data()[7], inverseModelViewMatrix.data()[11] }; // or 12 13 14 if transposed
    const QMatrix4x4 mvpMatrix = m_correctionMatrix.transposed() * qProjectionMatrix.transposed() * qModelViewMatrix.transposed();
    batch->updateDynamicBuffer(m_uniformBuffer, 0, utils::MATRIX4_SIZE, mvpMatrix.constData());
    batch->updateDynamicBuffer(m_uniformBuffer, utils::MATRIX4_SIZE, utils::VEC3_SIZE, cameraPosition.data());

    if (!m_uniformBuffer->build())
    {
        msg_error() << "Problem while building uniform buffer";
    }
}

bool RHIModel::initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc)
{
    const VecCoord& vertices = this->getVertices();
    const VecDeriv& vnormals = this->getVnormals();
    const VecTexCoord& vtexcoords = this->getVtexcoords();

    // Create Buffers
    m_vertexPositionBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, 0); // set size later (when we know it)
    m_indexTriangleBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::IndexBuffer, 0); // set size later (when we know it)
    m_uniformBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, utils::MATRIX4_SIZE + utils::VEC3_SIZE);
    
    // Create Pipeline
    m_srb = rhi->newShaderResourceBindings();
    const QRhiShaderResourceBinding::StageFlags commonVisibility = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
    m_srb->setBindings({
                         QRhiShaderResourceBinding::uniformBuffer(0, commonVisibility, m_uniformBuffer, 0, utils::MATRIX4_SIZE + utils::VEC3_SIZE)
        });
    if (!m_srb->build())
    {
        msg_error() << "Problem while building srb";
        return false;
    }

    m_pipeline = rhi->newGraphicsPipeline();
    QShader vs = utils::loadShader(":/shaders/gl/phong.vert.qsb");
    QShader fs = utils::loadShader(":/shaders/gl/phong.frag.qsb");
    if (!vs.isValid())
    {
        msg_error() << "Problem while vs shader";
        return false;
    }
    if (!fs.isValid())
    {
        msg_error() << "Problem while fs shader";
        return false;
    }

    m_pipeline->setShaderStages({ { QRhiShaderStage::Vertex, vs }, { QRhiShaderStage::Fragment, fs } });
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ 
        { 3 * sizeof(float) } , 
        { 3 * sizeof(float) } , 
        { 2 * sizeof(float) }
    }); // 3 floats vertex + 3 floats normal + 2 floats uv
    inputLayout.setAttributes({ 
        { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
        { 1, 1, QRhiVertexInputAttribute::Float3, 0 },
        { 2, 2, QRhiVertexInputAttribute::Float2, 0 }
    });
    m_pipeline->setVertexInputLayout(inputLayout);
    m_pipeline->setShaderResourceBindings(m_srb);
    m_pipeline->setRenderPassDescriptor(rpDesc.get());
    m_pipeline->setTopology(QRhiGraphicsPipeline::Topology::Triangles);
    m_pipeline->setDepthTest(true);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthOp(QRhiGraphicsPipeline::Less);
    m_pipeline->setStencilTest(false);
    //m_pipeline->setCullMode(QRhiGraphicsPipeline::None);
    
    if (!m_pipeline->build())
    {
        msg_error() << "Problem while building pipeline";
        return false;
    }

    // SOFA gives a projection matrix for OpenGL system
    // but other graphics API compute differently their clip space
    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // clipSpaceCorrMatrix() return a matrix to convert for other systems and identity for OpenGL
    m_correctionMatrix = rhi->clipSpaceCorrMatrix(); 

    return true;
}

void RHIModel::updateRHIResources(QRhiResourceUpdateBatch* batch)
{
    if (m_vertexPositionBuffer == nullptr)
    {
        return;
    }

    //Update Buffers
    if(m_updateGeometry) // we will update only when a step is done
    {
        updateVertexBuffer(batch);
        updateIndexBuffer(batch);
    }
    updateUniformBuffer(batch); //will be updated all the time (camera, light and no step)
}

void RHIModel::updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport)
{
    if (m_vertexPositionBuffer == nullptr)
    {
        return;
    }    

    //Create commands
    cb->setGraphicsPipeline(m_pipeline);
    cb->setShaderResources();
    cb->setViewport(viewport);
    const QRhiCommandBuffer::VertexInput vbindings[] = {
        { m_vertexPositionBuffer, 0 },
        { m_vertexPositionBuffer, m_positionsBufferSize },
        { m_vertexPositionBuffer, m_positionsBufferSize + m_normalsBufferSize }
    };
    //QRhiCommandBuffer::VertexInput vbindings(m_vertexPositionBuffer, 0);
    cb->setVertexInput(0, 3, vbindings, m_indexTriangleBuffer,0, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(m_triangleNumber * 3);
}

SOFA_DECL_CLASS(RHIModel)

int RHIModelClass = core::RegisterObject("RHIModel")
.add< RHIModel>()
;

} // namespace sofa::rhi
