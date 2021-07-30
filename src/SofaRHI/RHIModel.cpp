#include <SofaRHI/RHIModel.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/type/RGBAColor.h>
#include <sofa/type/Material.h>
#include <sofa/core/ObjectFactory.h>

#include <sofa/helper/system/FileRepository.h>
#include <SofaRHI/DrawToolRHI.h>
#include <SofaRHI/RHIUtils.h>

namespace sofa::rhi
{

using sofa::type::RGBAColor;
using namespace sofa;
using namespace sofa::component::visualmodel;

///// RHI Mesh
RHIGroup::RHIGroup(const utils::BufferInfo& bufferInfo, int materialID)
    : m_bufferInfo(bufferInfo)
    , m_materialID(materialID)
{

}

void RHIGroup::addDrawCommand(QRhiCommandBuffer* cb, const QRhiCommandBuffer::VertexInput* vbindings)
{
    cb->setVertexInput(0, 3, vbindings, m_bufferInfo.buffer, m_bufferInfo.offset, QRhiCommandBuffer::IndexUInt32);
    cb->drawIndexed(m_bufferInfo.size * 3);
}

///// RHI Phong Group
bool RHIPhongRendering::initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial)
{
    m_materialBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, int(utils::PHONG_MATERIAL_SIZE));

    m_srb = rhi->newShaderResourceBindings();
    std::vector<QRhiShaderResourceBinding> wholeBindings;
    wholeBindings.resize(globalBindings.size());
    std::copy(globalBindings.begin(), globalBindings.end(), wholeBindings.begin());
    wholeBindings.push_back(QRhiShaderResourceBinding::uniformBuffer(int(globalBindings.size()), QRhiShaderResourceBinding::FragmentStage, m_materialBuffer, 0, int(utils::PHONG_MATERIAL_SIZE)));
    m_srb->setBindings(wholeBindings.begin(), wholeBindings.end());

    if (!m_srb->build())
    {
        msg_error("RHIPhongRendering") << "Problem while building srb";
        return false;
    }

    // Triangle Pipeline 
    //const int secondUbufOffset = rhi->ubufAligned(4 * sizeof(float));
    //std::cout << "4 * sizeof(float) " << 4 * sizeof(float) << std::endl;
    //std::cout << "ubufAlignment " << secondUbufOffset << std::endl;
    m_pipeline = rhi->newGraphicsPipeline();
    QShader vs = utils::loadShader(":/shaders/gl/phong.vert.qsb");
    QShader fs = utils::loadShader(":/shaders/gl/phong.frag.qsb");
    if (!vs.isValid())
    {
        msg_error("RHIPhongRendering") << "Problem while vs shader";
        return false;
    }
    if (!fs.isValid())
    {
        msg_error("RHIPhongRendering") << "Problem while fs shader";
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
    QRhiGraphicsPipeline::TargetBlend premulAlphaBlend;
    premulAlphaBlend.enable = true;
    QVarLengthArray<QRhiGraphicsPipeline::TargetBlend, 4> rtblends;
    int colorAttCount = 1; // for later use
    for (int i = 0; i < colorAttCount; ++i)
        rtblends << premulAlphaBlend;
    m_pipeline->setTargetBlends(rtblends.cbegin(), rtblends.cend());
    //m_pipeline->setCullMode(QRhiGraphicsPipeline::None);

    if (!m_pipeline->build())
    {
        msg_error("RHIPhongRendering") << "Problem while building pipeline";
        return false;
    }

    return true;
}
void RHIPhongRendering::updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial)
{
    utils::PhongMaterial material = {
            { loaderMaterial.ambient.r(), loaderMaterial.ambient.g(),loaderMaterial.ambient.b(), loaderMaterial.ambient.a()},
            { loaderMaterial.diffuse.r(), loaderMaterial.diffuse.g(),loaderMaterial.diffuse.b(), loaderMaterial.diffuse.a()},
            { loaderMaterial.specular.r(), loaderMaterial.specular.g(),loaderMaterial.specular.b(), loaderMaterial.specular.a()},
            { loaderMaterial.shininess, 0.0f , 0.0f, 0.0f}
    };

    batch->updateDynamicBuffer(m_materialBuffer, 0, utils::PHONG_MATERIAL_SIZE, &material);

    if (!m_materialBuffer->build())
    {
        msg_error("RHIPhongRendering") << "Problem while building material uniform buffer";
    }
}

///// RHI Textured Phong Group
bool RHIDiffuseTexturedPhongRendering::initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial)
{
    //load image
    std::string textureFilename(loaderMaterial.textureFilename);

    if (!sofa::helper::system::DataRepository.findFile(textureFilename, "", nullptr))
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Could not find diffuse image " << loaderMaterial.textureFilename;
        return false;
    }

    m_diffuseImage = QImage(QLatin1String(textureFilename.c_str())).convertToFormat(QImage::Format_RGBA8888);
    if (m_diffuseImage.isNull())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while reading diffuse image " << textureFilename;
        return false;
    }
    m_diffuseImage = m_diffuseImage.mirrored(false, true); // seems texcoord are upside down
    // create texture and sampler 
    QRhiTexture::Flags texFlags = 0;
    if (m_bMipMap)
        texFlags |= QRhiTexture::MipMapped;
    if (m_bAutoGenMipMap)
        texFlags |= QRhiTexture::UsedWithGenerateMips;

    m_diffuseTexture = rhi->newTexture(QRhiTexture::RGBA8, QSize(m_diffuseImage.width(), m_diffuseImage.height()), 1, texFlags);
    if (!m_diffuseTexture->build())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while building diffuse texture";
        return false;
    }
    m_diffuseSampler = rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, m_bMipMap ? QRhiSampler::Linear : QRhiSampler::None,
        QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
    if (!m_diffuseSampler->build())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while building diffuse sampler";
        return false;
    }
    
    m_materialBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, int(utils::PHONG_MATERIAL_SIZE));

    m_srb = rhi->newShaderResourceBindings();
    const QRhiShaderResourceBinding::StageFlags commonVisibility = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
    std::vector<QRhiShaderResourceBinding> wholeBindings;
    wholeBindings.resize(globalBindings.size());
    std::copy(globalBindings.begin(), globalBindings.end(), wholeBindings.begin());
    wholeBindings.push_back(QRhiShaderResourceBinding::uniformBuffer(int(globalBindings.size()), QRhiShaderResourceBinding::FragmentStage, m_materialBuffer, 0, int(utils::PHONG_MATERIAL_SIZE)));
    wholeBindings.push_back(QRhiShaderResourceBinding::sampledTexture(int(globalBindings.size()+1), QRhiShaderResourceBinding::FragmentStage, m_diffuseTexture, m_diffuseSampler));
    m_srb->setBindings(wholeBindings.begin(), wholeBindings.end());

    if (!m_srb->build())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while building srb";
        return false;
    }

    // Triangle Pipeline 
    //const int secondUbufOffset = rhi->ubufAligned(4 * sizeof(float));
    //std::cout << "4 * sizeof(float) " << 4 * sizeof(float) << std::endl;
    //std::cout << "ubufAlignment " << secondUbufOffset << std::endl;
    m_pipeline = rhi->newGraphicsPipeline();
    QShader vs = utils::loadShader(":/shaders/gl/phong.vert.qsb");
    QShader fs = utils::loadShader(":/shaders/gl/phong_diffuse_texture.frag.qsb");
    if (!vs.isValid())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while vs shader";
        return false;
    }
    if (!fs.isValid())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while fs shader";
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
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while building pipeline";
        return false;
    }

    return true;
}
void RHIDiffuseTexturedPhongRendering::updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial)
{
    utils::PhongMaterial material = {
            { loaderMaterial.ambient.r(), loaderMaterial.ambient.g(),loaderMaterial.ambient.b(), loaderMaterial.ambient.a()},
            { loaderMaterial.diffuse.r(), loaderMaterial.diffuse.g(),loaderMaterial.diffuse.b(), loaderMaterial.diffuse.a()},
            { loaderMaterial.specular.r(), loaderMaterial.specular.g(),loaderMaterial.specular.b(), loaderMaterial.specular.a()},
            { loaderMaterial.shininess, 0.0f , 0.0f, 0.0f}
    };

    batch->updateDynamicBuffer(m_materialBuffer, 0, utils::PHONG_MATERIAL_SIZE, &material);

    if (!m_materialBuffer->build())
    {
        msg_error("RHIDiffuseTexturedPhongRendering") << "Problem while building material uniform buffer";
    }

    if (!m_diffuseImage.isNull())
    {
        // TODO
        if (m_bMipMap)
        {

        }
        else
        {
            batch->uploadTexture(m_diffuseTexture, m_diffuseImage);
        }
    }
}


///// RHI Wireframe Group
bool RHIWireframeRendering::initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial)
{
    m_materialBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, int(utils::PHONG_MATERIAL_SIZE));

    m_srb = rhi->newShaderResourceBindings();
    std::vector<QRhiShaderResourceBinding> wholeBindings;
    wholeBindings.resize(globalBindings.size());
    std::copy(globalBindings.begin(), globalBindings.end(), wholeBindings.begin());
    wholeBindings.push_back(QRhiShaderResourceBinding::uniformBuffer(int(globalBindings.size()), QRhiShaderResourceBinding::FragmentStage, m_materialBuffer, 0, int(utils::PHONG_MATERIAL_SIZE)));
    m_srb->setBindings(wholeBindings.begin(), wholeBindings.end());

    if (!m_srb->build())
    {
        msg_error("RHIPhongRendering") << "Problem while building srb";
        return false;
    }

    // Line Pipeline 
    m_pipeline = rhi->newGraphicsPipeline();
    QShader vs = utils::loadShader(":/shaders/gl/phong.vert.qsb"); // just use the phong one...
    QShader fs = utils::loadShader(":/shaders/gl/phong.frag.qsb");
    if (!vs.isValid())
    {
        msg_error("RHIPhongRendering") << "Problem while vs shader";
        return false;
    }
    if (!fs.isValid())
    {
        msg_error("RHIPhongRendering") << "Problem while fs shader";
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
    m_pipeline->setTopology(QRhiGraphicsPipeline::Topology::Lines);
    m_pipeline->setDepthTest(true);
    m_pipeline->setDepthWrite(true);
    m_pipeline->setDepthOp(QRhiGraphicsPipeline::Less);
    m_pipeline->setStencilTest(false);
    //m_pipeline->setCullMode(QRhiGraphicsPipeline::None);

    if (!m_pipeline->build())
    {
        msg_error("RHIPhongRendering") << "Problem while building pipeline";
        return false;
    }

    return true;
}
void RHIWireframeRendering::updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial)
{
    utils::PhongMaterial material = {
            { loaderMaterial.ambient.r(), loaderMaterial.ambient.g(),loaderMaterial.ambient.b(), loaderMaterial.ambient.a()},
            { loaderMaterial.diffuse.r(), loaderMaterial.diffuse.g(),loaderMaterial.diffuse.b(), loaderMaterial.diffuse.a()},
            { loaderMaterial.specular.r(), loaderMaterial.specular.g(),loaderMaterial.specular.b(), loaderMaterial.specular.a()},
            { loaderMaterial.shininess, 0.0f , 0.0f, 0.0f}
    };

    batch->updateDynamicBuffer(m_materialBuffer, 0, utils::PHONG_MATERIAL_SIZE, &material);

    if (!m_materialBuffer->build())
    {
        msg_error("RHIWireframeRendering") << "Problem while building material uniform buffer";
    }
}

///// RHI Model
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

    // dont do anything RHI anymore

}

void RHIModel::updateVisual()
{
    if (d_componentState.getValue() != sofa::core::objectmodel::ComponentState::Valid)
        return;

    // we bypass the updateBuffers part because bugus and we want to custom anyway
    //update geometry
    InheritedVisual::updateVisual();

    // dont do anything RHI anymore
}

void RHIModel::updateBuffers()
{
    if (d_componentState.getValue() != sofa::core::objectmodel::ComponentState::Valid)
        return;

    m_needUpdatePositions = true;
}


void RHIModel::internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent)
{
    // dont do anything RHI anymore
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
    type::vector<sofa::type::Vec3f> fVertices;
    if (std::is_same<DataTypes::Real, float>::value == false)
    {
        for (const auto& v : vertices)
        {
            fVertices.push_back(sofa::type::Vec3f(v[0], v[1], v[2]));
        }
        ptrVertices = reinterpret_cast<const void*>(fVertices.data());
        positionsBufferSize = int(vertices.size() * sizeof(fVertices[0]));
    }

    int normalsBufferSize = int(vnormals.size() * sizeof(vnormals[0]));
    //convert normals to float if needed
    const void* ptrNormals = reinterpret_cast<const void*>(vnormals.data());
    type::vector<sofa::type::Vec3f> fNormals;
    if (std::is_same<DataTypes::Real, float>::value == false)
    {
        for (const auto& n : vnormals)
        {
            fNormals.push_back(sofa::type::Vec3f(n[0], n[1], n[2]));
        }
        ptrNormals = reinterpret_cast<const void*>(fNormals.data());
        normalsBufferSize = int(vnormals.size() * sizeof(fNormals[0]));
    }

    int textureCoordsBufferSize = int(vtexcoords.size() * sizeof(vtexcoords[0]));

    //2 cases: or we update everything on the GPU
    // or the mesh just moves and just needs to update position-related data
    if(updateAll || m_vertexPositionBuffer->size() == 0)
    {
        m_vertexPositionBuffer->setSize(positionsBufferSize + normalsBufferSize + textureCoordsBufferSize);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, 0, positionsBufferSize, ptrVertices);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize, normalsBufferSize, ptrNormals);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize + normalsBufferSize, textureCoordsBufferSize, vtexcoords.data());
        
        if (!m_vertexPositionBuffer->build())
        {
            msg_error() << "Problem while building vertex buffer";
        }
    }
    else
    {
        //assert that the size is good, etc
        batch->updateDynamicBuffer(m_vertexPositionBuffer, 0, positionsBufferSize, ptrVertices);
        batch->updateDynamicBuffer(m_vertexPositionBuffer, positionsBufferSize, normalsBufferSize, ptrNormals);
    }

    m_positionsBufferSize = positionsBufferSize;
    m_normalsBufferSize = normalsBufferSize;
    m_textureCoordsBufferSize = textureCoordsBufferSize;
}

void RHIModel::updateIndexBuffer(QRhiResourceUpdateBatch* batch)
{
    const auto& triangles = this->getTriangles();
    const auto& quads = this->getQuads();
    //convert to triangles
    VecVisualTriangle quadTriangles;
    for (const auto& q : quads)
    {
        quadTriangles.push_back({ q[0], q[1], q[2] });
        quadTriangles.push_back({ q[2], q[3], q[0] });
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

    const type::Vec3f cameraPosition{ inverseModelViewMatrix.data()[3], inverseModelViewMatrix.data()[7], inverseModelViewMatrix.data()[11] }; // or 12 13 14 if transposed
    const QMatrix4x4 mvpMatrix = m_correctionMatrix.transposed() * qProjectionMatrix.transposed() * qModelViewMatrix.transposed();
    batch->updateDynamicBuffer(m_cameraUniformBuffer, 0, utils::MATRIX4_SIZE, mvpMatrix.constData());
    batch->updateDynamicBuffer(m_cameraUniformBuffer, utils::MATRIX4_SIZE, utils::VEC3_SIZE, cameraPosition.data());

    if (!m_cameraUniformBuffer->build())
    {
        msg_error() << "Problem while building camera uniform buffer";
    }

}

bool RHIModel::initGraphicResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc)
{
    // I suppose it would be better to get the visualParams given as params but it is only in update/draw steps
    rhi::DrawToolRHI* rhiDrawTool = dynamic_cast<rhi::DrawToolRHI*>(sofa::core::visual::VisualParams::defaultInstance()->drawTool());

    if (rhiDrawTool == nullptr)
    {
        msg_error("RHIModel") << "Can only works with RHIViewer as gui; DrawToolRHI not detected.";
        return false;
    }

    //if (!initRHIResources(rhi, rpDesc))
    //{
    //    msg_error() << "Error while initializing RHI features";
    //    return;
    //}

    const VecCoord& vertices = this->getVertices();
    const VecDeriv& vnormals = this->getVnormals();
    const VecTexCoord& vtexcoords = this->getVtexcoords();

    // Create Buffers
    m_vertexPositionBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, 0); // set size later (when we know it)
    m_indexTriangleBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::IndexBuffer, 0); // set size later (when we know it)
    m_cameraUniformBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, utils::MATRIX4_SIZE + utils::VEC3_SIZE);
    
    std::vector<QRhiShaderResourceBinding> globalBindings;
    const QRhiShaderResourceBinding::StageFlags commonVisibility = QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage;
    globalBindings.push_back({
                         QRhiShaderResourceBinding::uniformBuffer(0, commonVisibility, m_cameraUniformBuffer, 0, utils::MATRIX4_SIZE + utils::VEC3_SIZE)
        }
    );

    // Create groups and their respective renderings
    const auto& groups = this->groups.getValue();
    const auto& triangles = this->getTriangles();
    const auto& quads = this->getQuads();

    //Split group with different primitives (into other groups)
    if (groups.size() == 0)
    {
        FaceGroup defaultGroup;
        bool isTextured = this->texturename.isSet() || !this->texturename.getValue().empty();

        if (triangles.size() > 0)
        {
            utils::BufferInfo bufferInfo;
            bufferInfo.buffer = m_indexTriangleBuffer;
            bufferInfo.offset = 0;
            bufferInfo.size = sofa::Size(triangles.size());

            if(isTextured)
                m_renderGroups.emplace_back(std::make_shared<RHIDiffuseTexturedPhongRendering>(RHIGroup(bufferInfo, defaultGroup.materialId)));
            else
                m_renderGroups.emplace_back(std::make_shared<RHIPhongRendering>(RHIGroup(bufferInfo, defaultGroup.materialId)));

            m_wireframeGroups.emplace_back(std::make_shared<RHIWireframeRendering>(RHIGroup(bufferInfo, defaultGroup.materialId)));

        }
        if (quads.size() > 0)
        {
            utils::BufferInfo bufferInfo;
            bufferInfo.buffer = m_indexTriangleBuffer;
            bufferInfo.offset = sofa::Size(triangles.size() * sizeof(triangles[0]));
            bufferInfo.size = sofa::Size(quads.size() * 2); //2 triangles for each quad

            if (isTextured)
                m_renderGroups.emplace_back(std::make_shared<RHIDiffuseTexturedPhongRendering>(RHIGroup(bufferInfo, defaultGroup.materialId)));
            else
                m_renderGroups.emplace_back(std::make_shared<RHIPhongRendering>(RHIGroup(bufferInfo, defaultGroup.materialId)));

            m_wireframeGroups.emplace_back(std::make_shared<RHIWireframeRendering>(RHIGroup(bufferInfo, defaultGroup.materialId)));
        }
    }
    else
    {
        for (const auto& group : groups)
        {
            const auto& materials = this->materials.getValue();
            auto loaderMaterial = materials[group.materialId];
            bool isTextured = loaderMaterial.useTexture && !loaderMaterial.textureFilename.empty();

            if (group.nbt > 0)
            {
                utils::BufferInfo bufferInfo;
                bufferInfo.buffer = m_indexTriangleBuffer;
                bufferInfo.offset = group.tri0 * sizeof(triangles[0]);
                bufferInfo.size = group.nbt;

                if (isTextured)
                    m_renderGroups.emplace_back(std::make_shared<RHIDiffuseTexturedPhongRendering>(RHIGroup(bufferInfo, group.materialId)));
                else
                    m_renderGroups.emplace_back(std::make_shared<RHIPhongRendering>(RHIGroup(bufferInfo, group.materialId)));

                m_wireframeGroups.emplace_back(std::make_shared<RHIWireframeRendering>(RHIGroup(bufferInfo, group.materialId)));
            }
            if (group.nbq > 0)
            {
                utils::BufferInfo bufferInfo;
                bufferInfo.buffer = m_indexTriangleBuffer;
                bufferInfo.offset = int(triangles.size() * sizeof(triangles[0])) +  (2*group.quad0) * sizeof(triangles[0]); //2 triangles for each quad
                bufferInfo.size = group.nbq * 2;

                if (isTextured)
                    m_renderGroups.emplace_back(std::make_shared<RHIDiffuseTexturedPhongRendering>(RHIGroup(bufferInfo, group.materialId)));
                else
                    m_renderGroups.emplace_back(std::make_shared<RHIPhongRendering>(RHIGroup(bufferInfo, group.materialId)));

                m_wireframeGroups.emplace_back(std::make_shared<RHIWireframeRendering>(RHIGroup(bufferInfo, group.materialId)));
            }
        }
    }

    for (auto& renderGroup : m_renderGroups)
    {
        const int materialID = renderGroup->getMaterialID();
        auto loaderMaterial = this->material.getValue();
        if (materialID >= 0)
        {
            const auto& materials = this->materials.getValue();
            loaderMaterial = materials[materialID];
        }

        renderGroup->initRHIResources(rhi, rpDesc, globalBindings, loaderMaterial);
    }

    for(auto & wireframeGroup : m_wireframeGroups)
    {
        const int materialID = wireframeGroup->getMaterialID();
        auto loaderMaterial = this->material.getValue();
        if (materialID >= 0)
        {
            const auto& materials = this->materials.getValue();
            loaderMaterial = materials[materialID];
        }
        wireframeGroup->initRHIResources(rhi, rpDesc, globalBindings, loaderMaterial);
    }

    // SOFA gives a projection matrix for OpenGL system
    // but other graphics API compute differently their clip space
    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // clipSpaceCorrMatrix() return a matrix to convert for other systems and identity for OpenGL
    m_correctionMatrix = rhi->clipSpaceCorrMatrix(); 


    d_componentState.setValue(sofa::core::objectmodel::ComponentState::Valid);

    return true;
}

void RHIModel::updateGraphicResources(QRhiResourceUpdateBatch* batch)
{
    if (m_vertexPositionBuffer == nullptr)
    {
        return;
    }

    if (batch == nullptr)
        return;

    //will be updated all the time (camera, light and no step)
    updateCameraUniformBuffer(batch);

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
        for (auto& renderGroup : m_renderGroups)
        {
            const int materialID = renderGroup->getMaterialID();
            auto loaderMaterial = this->material.getValue();
            if (materialID >= 0)
            {
                const auto& materials = this->materials.getValue();
                loaderMaterial = materials[materialID];
            }

            renderGroup->updateRHIResources(batch, loaderMaterial);
        }

        for (auto& wireframeGroup : m_wireframeGroups)
        {
            const int materialID = wireframeGroup->getMaterialID();
            auto loaderMaterial = this->material.getValue();
            if (materialID >= 0)
            {
                const auto& materials = this->materials.getValue();
                loaderMaterial = materials[materialID];
            }
            wireframeGroup->updateRHIResources(batch, loaderMaterial);
        }

        m_needUpdateMaterial = false;
    }
}

void RHIModel::updateGraphicCommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport)
{
    auto vparams = sofa::core::visual::VisualParams::defaultInstance();

    if (d_componentState.getValue() != sofa::core::objectmodel::ComponentState::Valid)
    {
        return;
    }
    if (!vparams->displayFlags().getShowVisual())
    {
        return;
    }
    if (m_vertexPositionBuffer == nullptr)
    {
        return;
    }

    const QRhiCommandBuffer::VertexInput vbindings[] = {
        { m_vertexPositionBuffer, quint32(0) },
        { m_vertexPositionBuffer, quint32(m_positionsBufferSize) },
        { m_vertexPositionBuffer, quint32(m_positionsBufferSize + m_normalsBufferSize) }
    };

    if (vparams->displayFlags().getShowWireFrame())
    {
        for (auto& wireGroup : m_wireframeGroups)
        {
            wireGroup->updateRHICommands(cb, viewport, vbindings);
        }
    }
    else
    {
        for (auto& renderGroup : m_renderGroups)
        {
            renderGroup->updateRHICommands(cb, viewport, vbindings);
        }
    }

}

void RHIModel::updateStorageBuffer(QRhiResourceUpdateBatch* batch)
{
    const auto& vertices = this->getVertices();
    const auto& triangles = this->getTriangles();

    std::vector<sofa::type::Vec3f> verticesFromTriangles;
    verticesFromTriangles.reserve(triangles.size() * 3);
    for (const auto& t : triangles)
    {
        for (int i = 0; i < 3; i++)
        {
            verticesFromTriangles.push_back({ float(vertices[t[i]][0]), float(vertices[t[i]][1]), float(vertices[t[i]][2]) });
        }
    }
    const int verticesFromTrianglesSize = int(verticesFromTriangles.size()) * 3 * sizeof(float);
    const int normalsSize = int(vertices.size()) * 3 * sizeof(float);

    if (m_storageBuffer->size() == 0)
    {
        m_storageBuffer->setSize(verticesFromTrianglesSize);
        batch->uploadStaticBuffer(m_storageBuffer, 0, verticesFromTrianglesSize, verticesFromTriangles.data());

        if (!m_storageBuffer->build())
        {
            msg_error() << "Problem while building storage buffer";
        }
    }
    else
    {
        //assert that the size is good, etc
        batch->uploadStaticBuffer(m_storageBuffer, 0, verticesFromTrianglesSize, verticesFromTriangles.data());
    }

    //set size for the resulting normals
    if(m_computeNormalBuffer->size() == 0)
    {
        m_computeNormalBuffer->setSize(normalsSize);
        if (!m_computeNormalBuffer->build())
        {
            msg_error() << "Problem while building compute normal buffer";
        }
    }

}

bool RHIModel::initComputeResources(QRhiPtr rhi)
{
    m_storageBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer, 4); // cannot be empty at creation
    if (!m_storageBuffer->build())
    {
        msg_error() << "Problem while building storage buffer";
    }
    m_computeNormalBuffer = rhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer | QRhiBuffer::VertexBuffer, 4); // cannot be empty at creation
    if (!m_computeNormalBuffer->build())
    {
        msg_error() << "Problem while building compute normal buffer";
    }
    //m_computeUniformBuffer = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, COMPUTE_UBUF_SIZE);
    //m_computeUniformBuffer.computeUniBuf->build();

    //ComputeUBuf ud;
    //ud.step = d.step;
    //ud.count = DATA_COUNT;
    //d.initialUpdates->updateDynamicBuffer(d.computeUniBuf, 0, COMPUTE_UBUF_SIZE, &ud);

    m_computeBindings = rhi->newShaderResourceBindings();
    m_computeBindings->setBindings({
                                      QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, m_storageBuffer),
                                      QRhiShaderResourceBinding::bufferLoadStore(1, QRhiShaderResourceBinding::ComputeStage, m_computeNormalBuffer),
                                       //QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::ComputeStage, m_computeUniformBuffer)
        });
    if (!m_computeBindings->build())
    {
        msg_error() << "Problem while building compute bindings";
        return false;
    }

    QShader cs = utils::loadShader(":/computeshaders/gl/compute_normals.comp.qsb");
    if (!cs.isValid())
    {
        msg_error("RHIModel") << "Problem while cs shader";
        return false;
    }
    m_computePipeline = rhi->newComputePipeline();
    m_computePipeline->setShaderResourceBindings(m_computeBindings);
    m_computePipeline->setShaderStage({ QRhiShaderStage::Compute, cs });
    if (!m_computePipeline->build())
    {
        msg_error() << "Problem while building compute pipeline";
        return false;
    }

    return true;
}
void RHIModel::updateComputeResources(QRhiResourceUpdateBatch* batch)
{
    if (m_storageBuffer == nullptr || m_computeNormalBuffer == nullptr)
        return;

    if (batch == nullptr)
        return;

    //Update Buffers (on demand)
    if (m_needUpdatePositions || m_needUpdateTopology) // true when a new step is done
    {
        //will be updated all the time (camera, light and no step)
        updateStorageBuffer(batch);
    }

}
void RHIModel::updateComputeCommands(QRhiCommandBuffer* cb) 
{
    const int sizeLocalGroup = 256;
    ////Create commands
    cb->setComputePipeline(m_computePipeline);
    cb->setShaderResources();
    cb->dispatch(m_computeNormalBuffer->size() / sizeLocalGroup, 1, 1);
}

SOFA_DECL_CLASS(RHIModel)

int RHIModelClass = core::RegisterObject("RHIModel")
.add< RHIModel>()
;

} // namespace sofa::rhi
