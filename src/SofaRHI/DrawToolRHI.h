#pragma once

#include <sofa/helper/visual/DrawTool.h>

#include <sofa/core/visual/DisplayFlags.h>
#include <sofa/type/Mat.h>

#include <array>
#include <stack>

#include <QtGui/private/qrhi_p.h>

namespace sofa::core::visual
{
    class VisualParams;
}

namespace sofa::rhi
{

class DrawToolRHI : public sofa::helper::visual::DrawTool
{
    using Inherited = sofa::helper::visual::DrawTool;
    using DisplayFlags = sofa::core::visual::DisplayFlags;

    using Vec4i = sofa::type::Vec4i;
    using Vec2f = sofa::type::Vec2f;
    using Matrix4f = sofa::type::Mat4x4f;
    using Matrix4d = sofa::type::Mat4x4d;

    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;

    struct VertexInputData {
        enum class Attributes : int
        {
            POSITION = 0,
            NORMAL = 1,
            COLOR = 2,
        };
        enum class PrimitiveType : int
        {
            POINT = 0,
            LINE = 1,
            TRIANGLE = 2, 
            INSTANCE_TRIANGLE = 3
        };
        struct MemoryInfo {
            QRhiBuffer* buffer;
            int offset;
            int size;
        };

        std::vector<MemoryInfo> attributesInfo;
        std::vector<MemoryInfo> instanceAttributesInfo;
        MemoryInfo indexInfo;
        PrimitiveType primitiveType;
        int nbPrimitive;
        int nbInstance = 1;
    };
    
public:
    DrawToolRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc);
    virtual ~DrawToolRHI() override {}

    /// @name Primitive rendering methods
    /// @{
    virtual void drawPoints(const std::vector<Vector3> &points, float size, const  RGBAColor& color) override;
    virtual void drawPoints(const std::vector<Vector3> &points, float size, const std::vector<RGBAColor>& colors) override;

    virtual void drawLine(const type::Vector3 &p1, const type::Vector3 &p2, const RGBAColor& color) override;
    virtual void drawInfiniteLine(const type::Vector3 &point, const type::Vector3 &direction, const RGBAColor& color) override;
    virtual void drawLines(const std::vector<Vector3> &points, float size, const RGBAColor& color) override;
    virtual void drawLines(const std::vector<Vector3> &points, float size, const std::vector<RGBAColor>& colorss) override;
    virtual void drawLines(const std::vector<Vector3> &points, const std::vector< Vec2i > &index, float size, const RGBAColor& color) override;

    virtual void drawLineStrip(const std::vector<Vector3> &points, float size, const RGBAColor& color) override;
    virtual void drawLineLoop(const std::vector<Vector3> &points, float size, const RGBAColor& color) override;

    virtual void drawDisk(float radius, double from, double to, int resolution, const RGBAColor& color) override;
    virtual void drawCircle(float radius, float lineThickness, int resolution, const RGBAColor& color) override;

    virtual void drawTriangles(const std::vector<Vector3> &points, const RGBAColor& color) override;
    virtual void drawTriangles(const std::vector<Vector3> &points, const type::Vector3& normal, const RGBAColor& color) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector< Vec3i > &index,
        const std::vector<Vector3>  &normal,
        const RGBAColor& color) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector< Vec3i > &index,
        const std::vector<Vector3>  &normal,
        const std::vector<RGBAColor>& colors) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector< RGBAColor > &colors) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector<Vector3>  &normal,
        const std::vector< RGBAColor > &colors) override;
    virtual void drawTriangleStrip(const std::vector<Vector3> &points,
        const std::vector<Vector3>  &normal,
        const RGBAColor& color) override;
    virtual void drawTriangleFan(const std::vector<Vector3> &points,
        const std::vector<Vector3>  &normal,
        const RGBAColor& color) override;

    virtual void drawFrame(const type::Vector3& position, const Quaternion &orientation, const Vec3f &size) override;
    virtual void drawFrame(const type::Vector3& position, const Quaternion &orientation, const Vec3f &size, const RGBAColor &colors) override;

    virtual void drawSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const RGBAColor& color) override;
    virtual void drawSpheres(const std::vector<Vector3> &points, float radius, const RGBAColor& color) override;
    virtual void drawFakeSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const RGBAColor& color) override;
    virtual void drawFakeSpheres(const std::vector<Vector3> &points, float radius, const RGBAColor& color) override;

    virtual void drawCone(const type::Vector3& p1, const type::Vector3 &p2, float radius1, float radius2, const RGBAColor& color, int subd = 16) override;

    /// Draw a cube of size one centered on the current point.
    virtual void drawCube(const float& radius, const RGBAColor& color, const int& subd = 16) override;

    virtual void drawCylinder(const type::Vector3& p1, const type::Vector3 &p2, float radius, const RGBAColor& color, int subd = 16) override;

    virtual void drawCapsule(const type::Vector3& p1, const type::Vector3 &p2, float radius, const RGBAColor& color, int subd = 16) override;

    virtual void drawArrow(const type::Vector3& p1, const type::Vector3 &p2, float radius, const RGBAColor& color, int subd = 16) override;
    virtual void drawArrow(const type::Vector3& p1, const type::Vector3 &p2, float radius, float coneLength, const RGBAColor& color, int subd = 16) override;
    virtual void drawArrow(const type::Vector3& p1, const type::Vector3& p2, float radius, float coneLength, float coneRadius, const RGBAColor& color, int subd = 16) override;

    /// Draw a cross (3 lines) centered on p
    virtual void drawCross(const type::Vector3&p, float length, const RGBAColor& color) override;

    /// Draw a plus sign of size one centered on the current point.
    virtual void drawPlus(const float& radius, const RGBAColor& color, const int& subd = 16) override;

    virtual void drawPoint(const type::Vector3 &p, const RGBAColor &c) override;
    virtual void drawPoint(const type::Vector3 &p, const type::Vector3 &n, const RGBAColor &c) override;

    virtual void drawTriangle(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3,
        const type::Vector3 &normal) override;
    virtual void drawTriangle(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3,
        const type::Vector3 &normal, const RGBAColor &c) override;
    virtual void drawTriangle(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3,
        const type::Vector3 &normal,
        const RGBAColor &c1, const RGBAColor &c2, const RGBAColor &c3) override;
    virtual void drawTriangle(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3,
        const type::Vector3 &normal1, const type::Vector3 &normal2, const type::Vector3 &normal3,
        const RGBAColor &c1, const RGBAColor &c2, const RGBAColor &c3) override;

    virtual void drawQuad(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3, const type::Vector3 &p4,
        const type::Vector3 &normal) override;
    virtual void drawQuad(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3, const type::Vector3 &p4,
        const type::Vector3 &normal, const RGBAColor &c) override;
    virtual void drawQuad(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3, const type::Vector3 &p4,
        const type::Vector3 &normal,
        const RGBAColor &c1, const RGBAColor &c2, const RGBAColor &c3, const RGBAColor &c4) override;
    virtual void drawQuad(const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3, const type::Vector3 &p4,
        const type::Vector3 &normal1, const type::Vector3 &normal2, const type::Vector3 &normal3, const type::Vector3 &normal4,
        const RGBAColor &c1, const RGBAColor &c2, const RGBAColor &c3, const RGBAColor &c4) override;
    virtual void drawQuads(const std::vector<Vector3> &points, const RGBAColor& color) override;
    virtual void drawQuads(const std::vector<Vector3> &points, const std::vector<RGBAColor>& colorss) override;

    virtual void drawTetrahedron(const type::Vector3 &p0, const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3, const RGBAColor &colors) override;
    virtual void drawTetrahedra(const std::vector<Vector3> &points, const RGBAColor& color) override;
    //Scale each tetrahedron
    virtual void drawScaledTetrahedra(const std::vector<Vector3> &points, const RGBAColor& color, const float scale) override;

    virtual void drawHexahedron(const type::Vector3 &p0, const type::Vector3 &p1, const type::Vector3 &p2, const type::Vector3 &p3,
        const type::Vector3 &p4, const type::Vector3 &p5, const type::Vector3 &p6, const type::Vector3 &p7, const RGBAColor &colors) override;
    virtual void drawHexahedra(const std::vector<Vector3> &points, const RGBAColor& color) override;
    //Scale each hexahedron
    virtual void drawScaledHexahedra(const std::vector<Vector3> &points, const RGBAColor& color, const float scale) override;

    virtual void drawSphere(const type::Vector3 &p, float radius) override;
    virtual void drawSphere(const type::Vector3 &p, float radius, const RGBAColor& colour) override;

    virtual void drawEllipsoid(const type::Vector3 &p, const type::Vector3 &radii) override;

    virtual void drawBoundingBox(const type::Vector3 &min, const type::Vector3 &max, float size = 1.0) override;

    virtual void draw3DText(const type::Vector3 &p, float scale, const RGBAColor &color, const char* text) override;
    virtual void draw3DText_Indices(const std::vector<Vector3> &positions, float scale, const RGBAColor &color) override;
    /// @}

    /// @name Transformation methods.
    /// @{
    virtual void pushMatrix() override;

    virtual void popMatrix() override;

    virtual void multMatrix(float*) override;

    virtual void scale(float) override;
    virtual void translate(float x, float y, float z) override;
    /// @}

    /// @name Drawing style methods.
    virtual void setMaterial(const RGBAColor &colors) override;

    virtual void resetMaterial(const RGBAColor &colors) override;
    virtual void resetMaterial() override;

    virtual void setPolygonMode(int _mode, bool _wireframe) override;

    virtual void setLightingEnabled(bool _isAnabled) override;
    /// @}

    void enablePolygonOffset(float factor, float units) override;
    void disablePolygonOffset() override;

    virtual void enableBlending() override;
    virtual void disableBlending() override;

    virtual void enableLighting() override;
    virtual void disableLighting() override;

    virtual void enableDepthTest() override;
    virtual void disableDepthTest() override;

    /// @name States (save/restore)
    virtual void saveLastState() override;
    virtual void restoreLastState() override;

    /// @name Overlay methods

    /// draw 2D text at position (x,y) from top-left corner
    virtual void writeOverlayText(int x, int y, unsigned fontSize, const RGBAColor &color, const char* text) override;

    // @name Color Buffer method
    virtual void readPixels(int x, int y, int w, int h, float* rgb, float* z = nullptr) override;
    /// @}

    virtual void init() override;
    virtual void clear() override;

    // RHI specific
    void initRHI();
    QRhiPtr getRHI() 
    { 
        //would be better to restrict access to rhi and provide wrapper functions for buffer, etc
        //but lazy
        return m_rhi; 
    }; 
    QRhiRenderPassDescriptorPtr getRenderPassDescriptor() 
    {
        return m_rpDesc;
    }
    QRhiResourceUpdateBatch* getResourceUpdateBatch()
    {
        return m_currentRUB;
    }
    QRhiCommandBuffer* getCommandBuffer()
    {
        return m_currentCB;
    }
    const QRhiViewport getViewport()
    {
        return m_currentViewport;
    }

    void beginFrame(core::visual::VisualParams*  vparams, QRhiResourceUpdateBatch* rub, QRhiCommandBuffer* cb,  const QRhiViewport& viewport);
    void endFrame();
    void executeCommands();

private:

    //static type::Vector3 computeNormal(const type::Vector3& a, const type::Vector3& b, const type::Vector3& c);

    /// Hidden general drawing methods
    using Vector3f = std::array<float, 3>;
    template<typename A, typename B>
    static void convertVecAToVecB(const A& vecA, B& vecB);

    void internalDrawPoints(const std::vector<Vector3> &points, float size, const std::vector<RGBAColor>& colors);
    void internalDrawLines(const std::vector<Vector3> &points, const std::vector< Vec2i > &index, float size, const std::vector<RGBAColor>& colors);
    void internalDrawTriangles(const std::vector<Vector3> &points, const std::vector< Vec3i > &index, const std::vector<Vector3>  &normal, const std::vector<RGBAColor>& colors);
    void internalDrawInstancedTriangles(const std::vector<Vector3>& points, const std::vector< Vec3i >& index, const std::vector<Vector3>& normal, const std::vector<RGBAColor>& colors, const std::vector<Vector3f>& transforms);

    QRhiPtr m_rhi; //needed to create Buffers
    QRhiRenderPassDescriptorPtr m_rpDesc;
    bool m_bHasInit = false;

    QRhiGraphicsPipeline* m_trianglePipeline;
    QRhiGraphicsPipeline* m_linePipeline;
    QRhiGraphicsPipeline* m_pointPipeline;
    QRhiGraphicsPipeline* m_instancedTrianglePipeline;
    QRhiShaderResourceBindings* m_triangleSrb;
    QRhiShaderResourceBindings* m_lineSrb;
    QRhiShaderResourceBindings* m_pointSrb;
    QRhiShaderResourceBindings* m_instancedTriangleSrb;
    
    QRhiBuffer* m_cameraUniformBuffer;
    QRhiBuffer* m_materialUniformBuffer;
    QRhiBuffer* m_vertexBuffer;
    QRhiBuffer* m_indexBuffer;
    QRhiBuffer* m_instanceBuffer;

    QMatrix4x4 m_correctionMatrix;
    QRhiCommandBuffer* m_currentCB = nullptr;
    QRhiViewport m_currentViewport;
    QRhiResourceUpdateBatch* m_currentRUB = nullptr;

    int m_currentVertexBufferByteSize = 0;
    int m_currentIndexBufferByteSize = 0;
    int m_currentInstanceBufferByteSize = 0;
    std::map<VertexInputData::PrimitiveType, std::vector<VertexInputData> > m_vertexInputData;

    static constexpr int INITIAL_VERTEX_BUFFER_SIZE{ 1000000 * 10 * sizeof(float) }; //large enough for 1M vertices (position + normal + color)
    static constexpr int INITIAL_INDEX_BUFFER_SIZE{ 1000000 * 3 * sizeof(unsigned int) }; //large enough for 1M triangles
    static constexpr int INITIAL_INSTANCE_BUFFER_SIZE{ 1000000 * 3 * sizeof(float) }; //1M instance of vec3 (translation...)

};

} // namespace sofa::rhi

