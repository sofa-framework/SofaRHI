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

    m_needUpdatePositions = true;
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

    m_needUpdateTopology = (itBegin != itEnd);

    InheritedVisual::handleTopologyChange();

}

void RHIModel::updateVertexBuffer(QRhiResourceUpdateBatch* batch, bool updateAll)
{
    const auto& vertices = this->getVertices();
    const auto& vnormals = this->getVnormals();
    const auto& vtexcoords = this->getVtexcoords();

    int positionsBufferSize = int(vertices.size() * sizeof(vertices[0]));
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
        positionsBufferSize = int(vertices.size() * sizeof(fVertices[0]));
    }

    int normalsBufferSize = int(vnormals.size() * sizeof(vnormals[0]));
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
        normalsBufferSize = int(vnormals.size() * sizeof(fNormals[0]));
    }

    int textureCoordsBufferSize = int(vtexcoords.size() * sizeof(vtexcoords[0]));

    //TODO: check if it is reasonable to store materialID per vertex
    //first idea was to have an uniform value and update it when drawing each group
    //but dont know if possible/good...
    //for now it seems really not optimized...
    std::vector<uint8_t> materialIDs;
    materialIDs.resize(vertices.size());
    int materialIDBufferSize = int(materialIDs.size() * sizeof(materialIDs[0]));
    if (updateAll)
    {
        const auto& groups = this->groups.getValue();
        if (groups.size() == 0)
        {
            std::fill(materialIDs.begin(), materialIDs.end(), 0);
        }
        else
        {
            const auto& edges = this->getEdges();
            const auto& triangles = this->getTriangles();
            const auto& quads = this->getQuads();
            for (const auto& g : groups)
            {
                //edge
                for (std::size_t i = g.edge0; i < g.nbe; i++)
                {
                    for(std::size_t j = 0; i < 2; j++)
                        materialIDs[edges[i][j]] = g.materialId + 1;
                }
                for (std::size_t i = g.tri0; i < g.nbt; i++)
                {
                    for (std::size_t j = 0; i < 3; j++)
                        materialIDs[triangles[i][j]] = g.materialId + 1;
                }
                for (std::size_t i = g.quad0; i < g.nbq; i++)
                {
                    for (std::size_t j = 0; i < 4; j++)
                        materialIDs[quads[i][j]] = g.materialId + 1;
                }
            }
        }
    }

    //2 cases: or we update everything on the GPU
    // or the mesh just moves and just needs to update position-related data
    if(updateAll || m_vertexPositionBuffer->size() == 0)
    {
        m_vertexPositionBuffer->setSize(positionsBufferSize + normalsBufferSize + textureCoordsBufferSize + materialIDBufferSize);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, 0, positionsBufferSize, ptrVertices);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize, normalsBufferSize, ptrNormals);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize + normalsBufferSize, textureCoordsBufferSize, vtexcoords.data());
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize + normalsBufferSize + textureCoordsBufferSize, materialIDBufferSize, materialIDs.data());
    }
    else
    {
        //assert that the size is good, etc
        batch->updateDynamicBuffer(m_vertexPositionBuffer, 0, positionsBufferSize, ptrVertices);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize, normalsBufferSize, ptrNormals);

    }

    if (!m_vertexPositionBuffer->build())
    {
        msg_error() << "Problem while building vertex buffer";
    }

    m_positionsBufferSize = positionsBufferSize;
    m_normalsBufferSize = normalsBufferSize;
    m_textureCoordsBufferSize = textureCoordsBufferSize;
    m_materialIDBufferSize = materialIDBufferSize;
}

void RHIModel::updateIndexBuffer(QRhiResourceUpdateBatch* batch)
{
    const auto& triangles = this->getTriangles();
    const VecQuad& quads = this->getQuads();
    //convert to triangles
    VecTriangle quadTriangles;
    for (const Quad& q : quads)
    {
        quadTriangles.push_back(Triangle(q[0], q[1], q[2]));
        quadTriangles.push_back(Triangle(q[2], q[3], q[0]));
    }

    int triangleSize = int(triangles.size() * sizeof(triangles[0]));
    int quadTrianglesSize = int(quadTriangles.size() * sizeof(quadTriangles[0]));

    m_indexTriangleBuffer->setSize(triangleSize + quadTrianglesSize);
    batch->updateDynamicBuffer(m_indexTriangleBuffer, 0, triangleSize, triangles.data());
    batch->updateDynamicBuffer(m_indexTriangleBuffer, triangleSize, quadTrianglesSize, quadTriangles.data());

    m_triangleNumber = int(triangles.size());
    m_quadTriangleNumber = int(quadTriangles.size());

    if (!m_indexTriangleBuffer->build())
    {
        msg_error() << "Problem while building index buffer";
    }
}

void RHIModel::updateCameraUniformBuffer(QRhiResourceUpdateBatch* batch)
{
    const auto vparams = sofa::core::visual::VisualParams::defaultInstance(); // TODO:get from parameters?

    // Camera
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
    batch->updateDynamicBuffer(m_cameraUniformBuffer, 0, utils::MATRIX4_SIZE, mvpMatrix.constData());
    batch->updateDynamicBuffer(m_cameraUniformBuffer, utils::MATRIX4_SIZE, utils::VEC3_SIZE, cameraPosition.data());

    if (!m_cameraUniformBuffer->build())
    {
        msg_error() << "Problem while building camera uniform buffer";
    }
}

// Dont know how to push an array of struct into an uniform buffer!
void RHIModel::updateMaterialUniformBuffer(QRhiResourceUpdateBatch* batch)
{

    // Materials
    const auto& materials = this->materials.getValue();
    if (materials.size() > utils::MAXIMUM_MATERIAL_NUMBER)
    {
        msg_warning() << "Too many material defined, will only use " << utils::MAXIMUM_MATERIAL_NUMBER << " instead of " << materials.size();
    }

    utils::MaterialFlags flags = utils::MaterialFlags::NONE;
    std::vector<utils::Material> materialsToPush;
    //first one is the default/pullback material
    const auto& material = this->material.getValue();
    if (material.useTexture)
    {
        flags = utils::MaterialFlags::USE_DIFFUSE_TEXTURE;
    }
    materialsToPush.push_back({
            { material.ambient.r(), material.ambient.g(),material.ambient.b(), material.ambient.a()},
            { material.diffuse.r(), material.diffuse.g(),material.diffuse.b(), material.diffuse.a()},
            { material.specular.r(), material.specular.g(),material.specular.b(), material.specular.a()},
            { material.shininess, 0.0f , 0.0f, 0.0f},
            flags }
    );
     
    //push the other one if it exists
    for(const auto& material: this->materials.getValue())
    {
        utils::MaterialFlags flags = utils::MaterialFlags::NONE;
        if (material.useTexture)
        {
            flags = utils::MaterialFlags::USE_DIFFUSE_TEXTURE;
        }

        materialsToPush.push_back({
            { material.ambient.r(), material.ambient.g(),material.ambient.b(), material.ambient.a()},
            { material.diffuse.r(), material.diffuse.g(),material.diffuse.b(), material.diffuse.a()},
            { material.specular.r(), material.specular.g(),material.specular.b(), material.specular.a()},
            { material.shininess, 0.0f , 0.0f, 0.0f},
            flags }
        );
    }
    materialsToPush.resize(1);


    //std::array<float, 4> diffuse = { 1.0f, 0.0f, 1.0f, 1.0f };
    batch->updateDynamicBuffer(m_materialsUniformBuffer, 0, utils::MAXIMUM_MATERIAL_NUMBER * utils::MATERIAL_SIZE, materialsToPush.data());
    
    if (!m_materialsUniformBuffer->build())
    {
        msg_error() << "Problem while building material uniform buffer";
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
    m_cameraUniformBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, utils::MATRIX4_SIZE + utils::VEC3_SIZE);
    m_materialsUniformBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, utils::MAXIMUM_MATERIAL_NUMBER * utils::MATERIAL_SIZE);
    
    // Create Pipelines
    // Triangle Pipeline 
    //const int secondUbufOffset = rhi->ubufAligned(4 * sizeof(float));
    //std::cout << "4 * sizeof(float) " << 4 * sizeof(float) << std::endl;
    //std::cout << "ubufAlignment " << secondUbufOffset << std::endl;
    m_srb = rhi->newShaderResourceBindings();
    const QRhiShaderResourceBinding::StageFlags commonVisibility = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
    m_srb->setBindings({
                         QRhiShaderResourceBinding::uniformBuffer(0, commonVisibility, m_cameraUniformBuffer, 0, utils::MATRIX4_SIZE + utils::VEC3_SIZE),
                         QRhiShaderResourceBinding::uniformBuffer(1, commonVisibility, m_materialsUniformBuffer, 0,  utils::MAXIMUM_MATERIAL_NUMBER * utils::MATERIAL_SIZE)
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
        { 2 * sizeof(float) } ,
        { 1 * sizeof(uint8_t) }
    }); // 3 floats vertex + 3 floats normal + 2 floats uv
    inputLayout.setAttributes({ 
        { 0, 0, QRhiVertexInputAttribute::Float3, 0 },
        { 1, 1, QRhiVertexInputAttribute::Float3, 0 },
        { 2, 2, QRhiVertexInputAttribute::Float2, 0 },
        { 3, 3, QRhiVertexInputAttribute::UNormByte, 0 }
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

    //Update Buffers (on demand)
    if(m_needUpdatePositions) // true when a new step is done
    {
        updateVertexBuffer(batch, m_needUpdateTopology); // update all if topology changes
        m_needUpdatePositions = false;
    }
    if (m_needUpdateTopology) // true when the topology has changed
    {
        updateIndexBuffer(batch);
        m_needUpdateTopology = false;
    }
    if (m_needUpdateMaterial)
    {
        updateMaterialUniformBuffer(batch);
        m_needUpdateMaterial = false;
    }
    //will be updated all the time (camera, light and no step)
    updateCameraUniformBuffer(batch); 

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
        { m_vertexPositionBuffer, quint32(0) },
        { m_vertexPositionBuffer, quint32(m_positionsBufferSize) },
        { m_vertexPositionBuffer, quint32(m_positionsBufferSize + m_normalsBufferSize) }
    };
    const auto& groups = this->groups.getValue();
    if (groups.size() == 0)
    {
        if (m_triangleNumber > 0)
        {
            cb->setVertexInput(0, 3, vbindings, m_indexTriangleBuffer,0, QRhiCommandBuffer::IndexUInt32);
            cb->drawIndexed(m_triangleNumber * 3);
        }
        if (m_quadTriangleNumber > 0)
        {
            cb->setVertexInput(0, 3, vbindings, m_indexTriangleBuffer, m_triangleNumber, QRhiCommandBuffer::IndexUInt32);
            cb->drawIndexed(m_quadTriangleNumber * 3);
        }
    }
    else
    {
        for (const auto& group : groups)
        {
            if (group.nbt > 0)
            {
                cb->setVertexInput(0, 3, vbindings, m_indexTriangleBuffer, group.tri0, QRhiCommandBuffer::IndexUInt32);
                cb->drawIndexed(group.nbt * 3);
            }
            if (group.nbq > 0)
            {
                //2 triangles for each quad
                cb->setVertexInput(0, 3, vbindings, m_indexTriangleBuffer, m_triangleNumber + group.quad0 * 2, QRhiCommandBuffer::IndexUInt32);
                cb->drawIndexed(group.nbq * 2 * 3);
            }
        }
    }

}

SOFA_DECL_CLASS(RHIModel)

int RHIModelClass = core::RegisterObject("RHIModel")
.add< RHIModel>()
;

} // namespace sofa::rhi
