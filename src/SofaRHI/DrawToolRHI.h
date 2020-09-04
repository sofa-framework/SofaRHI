#pragma once

#include <sofa/core/visual/DrawTool.h>

#include <sofa/core/visual/DisplayFlags.h>

#include <array>
#include <stack>

#include <QtGui/private/qrhi_p.h>

namespace sofa::core::visual
{
    class VisualParams;
}

namespace sofa::rhi
{

class DrawToolRHI : public sofa::core::visual::DrawTool
{
    using Inherited = sofa::core::visual::DrawTool;
    using DisplayFlags = sofa::core::visual::DisplayFlags;

    using Vec4i = sofa::defaulttype::Vec4i;

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
        };
        struct MemoryInfo {
            QRhiBuffer* buffer;
            int offset;
            int size;
        };

        std::array<MemoryInfo, 3> attributesInfo;
        MemoryInfo indexInfo;
        PrimitiveType primitiveType;
        int nbPrimitive;
    };
    
public:
    DrawToolRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc);
    virtual ~DrawToolRHI() override {}

    /// @name Primitive rendering methods
    /// @{
    virtual void drawPoints(const std::vector<Vector3> &points, float size, const  Vec4f& color) override;
    virtual void drawPoints(const std::vector<Vector3> &points, float size, const std::vector<Vec4f>& colors) override;

    virtual void drawLine(const Vector3 &p1, const Vector3 &p2, const Vec4f& color) override;
    virtual void drawInfiniteLine(const Vector3 &point, const Vector3 &direction, const Vec4f& color) override;
    virtual void drawLines(const std::vector<Vector3> &points, float size, const Vec4f& color) override;
    virtual void drawLines(const std::vector<Vector3> &points, float size, const std::vector<Vec4f>& colorss) override;
    virtual void drawLines(const std::vector<Vector3> &points, const std::vector< Vec2i > &index, float size, const Vec4f& color) override;

    virtual void drawLineStrip(const std::vector<Vector3> &points, float size, const Vec4f& color) override;
    virtual void drawLineLoop(const std::vector<Vector3> &points, float size, const Vec4f& color) override;

    virtual void drawDisk(float radius, double from, double to, int resolution, const Vec4f& color) override;
    virtual void drawCircle(float radius, float lineThickness, int resolution, const Vec4f& color) override;

    virtual void drawTriangles(const std::vector<Vector3> &points, const Vec4f& color) override;
    virtual void drawTriangles(const std::vector<Vector3> &points, const Vector3& normal, const Vec4f& color) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector< Vec3i > &index,
        const std::vector<Vector3>  &normal,
        const Vec4f& color) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector< Vec3i > &index,
        const std::vector<Vector3>  &normal,
        const std::vector<Vec4f>& colors) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector< Vec4f > &colors) override;
    virtual void drawTriangles(const std::vector<Vector3> &points,
        const std::vector<Vector3>  &normal,
        const std::vector< Vec4f > &colors) override;
    virtual void drawTriangleStrip(const std::vector<Vector3> &points,
        const std::vector<Vector3>  &normal,
        const Vec4f& color) override;
    virtual void drawTriangleFan(const std::vector<Vector3> &points,
        const std::vector<Vector3>  &normal,
        const Vec4f& color) override;

    virtual void drawFrame(const Vector3& position, const Quaternion &orientation, const Vec3f &size) override;
    virtual void drawFrame(const Vector3& position, const Quaternion &orientation, const Vec3f &size, const Vec4f &colors) override;

    virtual void drawSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const Vec4f& color) override;
    virtual void drawSpheres(const std::vector<Vector3> &points, float radius, const Vec4f& color) override;
    virtual void drawFakeSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const Vec4f& color) override;
    virtual void drawFakeSpheres(const std::vector<Vector3> &points, float radius, const Vec4f& color) override;

    virtual void drawCone(const Vector3& p1, const Vector3 &p2, float radius1, float radius2, const Vec4f& color, int subd = 16) override;

    /// Draw a cube of size one centered on the current point.
    virtual void drawCube(const float& radius, const Vec4f& color, const int& subd = 16) override;

    virtual void drawCylinder(const Vector3& p1, const Vector3 &p2, float radius, const Vec4f& color, int subd = 16) override;

    virtual void drawCapsule(const Vector3& p1, const Vector3 &p2, float radius, const Vec4f& color, int subd = 16) override;

    virtual void drawArrow(const Vector3& p1, const Vector3 &p2, float radius, const Vec4f& color, int subd = 16) override;
    virtual void drawArrow(const Vector3& p1, const Vector3 &p2, float radius, float coneLength, const Vec4f& color, int subd = 16) override;
    virtual void drawArrow(const Vector3& p1, const Vector3& p2, float radius, float coneLength, float coneRadius, const Vec4f& color, int subd = 16) override;

    /// Draw a cross (3 lines) centered on p
    virtual void drawCross(const Vector3&p, float length, const Vec4f& color) override;

    /// Draw a plus sign of size one centered on the current point.
    virtual void drawPlus(const float& radius, const Vec4f& color, const int& subd = 16) override;

    virtual void drawPoint(const Vector3 &p, const Vec4f &c) override;
    virtual void drawPoint(const Vector3 &p, const Vector3 &n, const Vec4f &c) override;

    virtual void drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
        const Vector3 &normal) override;
    virtual void drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
        const Vector3 &normal, const Vec4f &c) override;
    virtual void drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
        const Vector3 &normal,
        const Vec4f &c1, const Vec4f &c2, const Vec4f &c3) override;
    virtual void drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
        const Vector3 &normal1, const Vector3 &normal2, const Vector3 &normal3,
        const Vec4f &c1, const Vec4f &c2, const Vec4f &c3) override;

    virtual void drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
        const Vector3 &normal) override;
    virtual void drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
        const Vector3 &normal, const Vec4f &c) override;
    virtual void drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
        const Vector3 &normal,
        const Vec4f &c1, const Vec4f &c2, const Vec4f &c3, const Vec4f &c4) override;
    virtual void drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
        const Vector3 &normal1, const Vector3 &normal2, const Vector3 &normal3, const Vector3 &normal4,
        const Vec4f &c1, const Vec4f &c2, const Vec4f &c3, const Vec4f &c4) override;
    virtual void drawQuads(const std::vector<Vector3> &points, const Vec4f& color) override;
    virtual void drawQuads(const std::vector<Vector3> &points, const std::vector<Vec4f>& colorss) override;

    virtual void drawTetrahedron(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vec4f &colors) override;
    virtual void drawTetrahedra(const std::vector<Vector3> &points, const Vec4f& color) override;
    //Scale each tetrahedron
    virtual void drawScaledTetrahedra(const std::vector<Vector3> &points, const Vec4f& color, const float scale) override;

    virtual void drawHexahedron(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
        const Vector3 &p4, const Vector3 &p5, const Vector3 &p6, const Vector3 &p7, const Vec4f &colors) override;
    virtual void drawHexahedra(const std::vector<Vector3> &points, const Vec4f& color) override;
    //Scale each hexahedron
    virtual void drawScaledHexahedra(const std::vector<Vector3> &points, const Vec4f& color, const float scale) override;

    virtual void drawSphere(const Vector3 &p, float radius) override;
    virtual void drawSphere(const Vector3 &p, float radius, const Vec4f& colour) override;

    virtual void drawEllipsoid(const Vector3 &p, const Vector3 &radii) override;

    virtual void drawBoundingBox(const Vector3 &min, const Vector3 &max, float size = 1.0) override;

    virtual void draw3DText(const Vector3 &p, float scale, const Vec4f &color, const char* text) override;
    virtual void draw3DText_Indices(const std::vector<Vector3> &positions, float scale, const Vec4f &color) override;
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
    virtual void setMaterial(const Vec4f &colors) override;

    virtual void resetMaterial(const Vec4f &colors) override;
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
    virtual void writeOverlayText(int x, int y, unsigned fontSize, const Vec4f &color, const char* text) override;

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

    static Vector3 computeNormal(const Vector3& a, const Vector3& b, const Vector3& c);

    /// Hidden general drawing methods
    using Vector3f = std::array<float, 3>;
    template<typename A, typename B>
    static void convertVecAToVecB(const A& vecA, B& vecB);

    void internalDrawPoints(const std::vector<Vector3> &points, float size, const std::vector<Vec4f>& colors);
    void internalDrawLines(const std::vector<Vector3> &points, const std::vector< Vec2i > &index, float size, const std::vector<Vec4f>& colors);
    void internalDrawTriangles(const std::vector<Vector3> &points, const std::vector< Vec3i > &index, const std::vector<Vector3>  &normal, const std::vector<Vec4f>& colors);
    void internalDrawQuads(const std::vector<Vector3> &points, const std::vector< Vec4i > &index, const std::vector<Vector3>  &normal, const std::vector<Vec4f>& colors);
    void internalDrawSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const Vec4f &color);
    void internalDrawTetrahedra(const std::vector<Vector3> &points, const std::vector<Vec4f>& colors, const float scale);
    void internalDrawHexahedra(const std::vector<Vector3> &points, const std::vector<Vec4f>& colors, const float scale);

    QRhiPtr m_rhi; //needed to create Buffers
    QRhiRenderPassDescriptorPtr m_rpDesc;
    bool m_bHasInit = false;

    QRhiGraphicsPipeline* m_trianglePipeline;
    QRhiGraphicsPipeline* m_edgePipeline;
    QRhiGraphicsPipeline* m_pointPipeline;
    QRhiShaderResourceBindings* m_srb;
    QRhiBuffer* m_cameraUniformBuffer;
    QRhiBuffer* m_materialUniformBuffer;
    QRhiBuffer* m_vertexPositionBuffer;
    QRhiBuffer* m_indexTriangleBuffer;
    QMatrix4x4 m_correctionMatrix;
    QRhiCommandBuffer* m_currentCB{nullptr};
    QRhiViewport m_currentViewport;
    QRhiResourceUpdateBatch* m_currentRUB{nullptr};

    int m_currentVertexPositionBufferSize{0};
    int m_currentIndexTriangleBufferSize{0};
    std::vector<VertexInputData> m_vertexInputData;

    static constexpr int INITIAL_VERTEX_BUFFER_SIZE{1000000 * 9 * sizeof(float)}; //large enough for 1M vertices (position + normal + color)
    static constexpr int INITIAL_INDEX_BUFFER_SIZE{ 1000000 * 3 * sizeof(unsigned int)}; //large enough for 1M triangles
};

} // namespace sofa::rhi

