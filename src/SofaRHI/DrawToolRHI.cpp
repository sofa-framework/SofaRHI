#include <SofaRHI/DrawToolRHI.h>


namespace sofa::core::visual
{

DrawToolRHI::DrawToolRHI()
{

}

DrawToolRHI::Vector3 DrawToolRHI::computeNormal(const Vector3& a, const Vector3& b, const Vector3& c)
{
    Vector3 n = cross((b - a), (c - a));
    n.normalize();

    return n;
}

void DrawToolRHI::internalDrawPoints(const std::vector<Vector3>& points, float size, const std::vector<Vec4f>& colors)
{

}

void DrawToolRHI::internalDrawLines(const std::vector<Vector3>& points, const std::vector< Vec2i >& index, float size, const std::vector<Vec4f>& colors)
{

}
void DrawToolRHI::internalDrawTriangles(const std::vector<Vector3>& points, const std::vector< Vec3i >& index, const std::vector<Vector3>& normal, const std::vector<Vec4f>& colors)
{

}
void DrawToolRHI::internalDrawQuads(const std::vector<Vector3>& points, const std::vector< Vec4i >& index, const std::vector<Vector3>& normal, const std::vector<Vec4f>& colors)
{

}
void DrawToolRHI::internalDrawSpheres(const std::vector<Vector3>& points, const std::vector<float>& radius, const Vec4f& color)
{

}
void DrawToolRHI::internalDrawTetrahedra(const std::vector<Vector3>& points, const std::vector<Vec4f>& colors, const float scale)
{

}
void DrawToolRHI::internalDrawHexahedra(const std::vector<Vector3>& points, const std::vector<Vec4f>& colors, const float scale)
{

}

void DrawToolRHI::drawPoints(const std::vector<Vector3>& points, float size, const  Vec4f& color)
{

}

void DrawToolRHI::drawPoints(const std::vector<Vector3> &points, float size, const std::vector<Vec4f>& colors)
{
    internalDrawPoints(points, size, colors);
}

void DrawToolRHI::drawLine(const Vector3 &p1, const Vector3 &p2, const Vec4f& color)
{
    std::vector<Vector3> positions;
    positions.push_back(p1);
    positions.push_back(p2);

    drawLines(positions, 1, color);
}

void DrawToolRHI::drawLines(const std::vector<Vector3> &points, float size, const Vec4f& color)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    drawLines(points, size, colors);
}

void DrawToolRHI::drawLines(const std::vector<Vector3> &points, float size, const std::vector<Vec4f>& colors)
{
    std::vector<Vec2i> indices;
    unsigned int nbLines = points.size()/2;
    for(unsigned int i=0 ; i < nbLines ; ++i)
    {
        indices.emplace_back(i*2 + 0,i*2 + 1);
    }

    internalDrawLines(points, indices, size, colors);

}

void DrawToolRHI::drawLines(const std::vector<Vector3> &points, const std::vector< Vec2i > &index, float size, const Vec4f& color)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    internalDrawLines(points, index, size, colors);
}
void DrawToolRHI::drawInfiniteLine(const Vector3 &point, const Vector3 &direction, const Vec4f& color){}
void DrawToolRHI::drawLineStrip(const std::vector<Vector3> &points, float size, const Vec4f& color){}
void DrawToolRHI::drawLineLoop(const std::vector<Vector3> &points, float size, const Vec4f& color){}
void DrawToolRHI::drawDisk(float radius, double from, double to, int resolution, const Vec4f& color){}
void DrawToolRHI::drawCircle(float radius, float lineThickness, int resolution, const Vec4f& color){}

/// Triangles
///
void DrawToolRHI::drawTriangles(const std::vector<Vector3> &points,
const std::vector< Vec3i > &index,
const std::vector<Vector3>  &normal,
const std::vector<Vec4f>& colors)
{
    internalDrawTriangles(points, index, normal, colors);
}

void DrawToolRHI::drawTriangles(const std::vector<Vector3> &points, const Vec4f& color)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    drawTriangles(points, colors);
}

void DrawToolRHI::drawTriangles(const std::vector<Vector3> &points, const Vector3& normal, const Vec4f& color)
{
    std::vector<Vec4f> colors;
    std::vector<Vector3> normals;

    colors.resize(points.size());
    normals.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);
    std::fill(normals.begin(), normals.end(), normal);

    drawTriangles(points, normals, colors);
}


void DrawToolRHI::drawTriangles(const std::vector<Vector3> &points,
    const std::vector< Vec4f > &colour)
{
    std::vector<Vec3i> indices;
    std::vector<Vector3> normals;
    unsigned int nbTriangles = points.size()/3;
    normals.resize(points.size());
    indices.reserve(nbTriangles);
    for (unsigned int i=0; i<nbTriangles; ++i)
    {
        const Vector3& a = points[ 3*i+0 ];
        const Vector3& b = points[ 3*i+1 ];
        const Vector3& c = points[ 3*i+2 ];
        Vector3 n = computeNormal(a,b,c);

        normals[ 3*i+0 ]= n;
        normals[ 3*i+1 ]= n;
        normals[ 3*i+2 ]= n;

        indices.emplace_back(i*3 + 0,i*3 + 1,i*3 + 2);
    }

    drawTriangles(points, indices, normals, colour);
}

void DrawToolRHI::drawTriangles(const std::vector<Vector3> &points,
    const std::vector<Vector3>  &normal,
    const std::vector< Vec4f > &colour)
{
    std::vector<Vec3i> indices;
    unsigned int nbTriangles = points.size()/3;
    for(unsigned int i=0 ; i < nbTriangles ; ++i)
    {
        indices.emplace_back(i*3 + 0,i*3 + 1,i*3 + 2);
    }

    drawTriangles(points, indices, normal, colour);
}

void DrawToolRHI::drawTriangles(const std::vector<Vector3> &points,
    const std::vector< Vec3i > &index,
    const std::vector<Vector3>  &normal,
    const Vec4f& color)
{
    setMaterial(color);

    std::vector<Vec4f> colors;
    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    //normal per face
    if(normal.size() * 3 == points.size())
    {
        std::vector<Vector3> normals;
        normals.reserve(points.size());
        for(const Vector3& n : normal)
        {
            for(int j=0 ; j<3 ;j++)
            {
                normals.push_back(n);
            }
        }
        drawTriangles(points, index, normals, colors);
    }
    else if(normal.size()  == points.size()) //assume normal per vertex
    {
        drawTriangles(points, index, normal, colors);
    }
    else
    {
        // fallback: recompute all normals
        std::vector<Vector3> normals;
        normals.resize(points.size());
        unsigned int nbTriangles = points.size()/3;

        for (unsigned int i=0; i<nbTriangles; ++i)
        {
            const Vector3& a = points[ 3*i+0 ];
            const Vector3& b = points[ 3*i+1 ];
            const Vector3& c = points[ 3*i+2 ];
            Vector3 n = computeNormal(a,b,c);

            normals[ 3*i+0 ]= n;
            normals[ 3*i+1 ]= n;
            normals[ 3*i+2 ]= n;
        }
        drawTriangles(points, index, normals, colors);

    }

}
///FIXME: remove single primitive drawing ?
void DrawToolRHI::drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
    const Vector3 &normal)
{
    drawTriangle(p1, p2, p3, normal, Vec4f(1.0f, 1.0f,1.0f,1.0f)); // with a dummy
}

void DrawToolRHI::drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
    const Vector3 &normal, const Vec4f &c)
{
    drawTriangle(p1, p2, p3, normal, normal, normal, c, c, c);
}

void DrawToolRHI::drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
    const Vector3 &normal,
    const Vec4f &c1, const Vec4f &c2, const Vec4f &c3)
{
    drawTriangle(p1, p2, p3, normal, normal, normal, c1, c2, c3);
}

void DrawToolRHI::drawTriangle(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
    const Vector3 &normal1, const Vector3 &normal2, const Vector3 &normal3,
    const Vec4f &c1, const Vec4f &c2, const Vec4f &c3)
{
    std::vector<Vector3> points;
    std::vector<Vector3> normals;
    std::vector<Vec3i> indices;
    std::vector<Vec4f> colors;

    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);

    normals.push_back(normal1);
    normals.push_back(normal2);
    normals.push_back(normal3);

    colors.push_back(c1);
    colors.push_back(c2);
    colors.push_back(c3);

    indices.push_back(Vec3i{0,1,2});

    internalDrawTriangles(points,indices,normals, colors);
}

/// Triangles Strip
void DrawToolRHI::drawTriangleStrip(const std::vector<Vector3> &points,
    const std::vector<Vector3>  &normal,
    const Vec4f& color)
{
}
/// Triangles Fan
void DrawToolRHI::drawTriangleFan(const std::vector<Vector3> &points,
    const std::vector<Vector3>  &normal,
    const Vec4f& color)
{
}

void DrawToolRHI::drawFrame(const Vector3& position, const Quaternion &orientation, const Vec3f &size){}
void DrawToolRHI::drawFrame(const Vector3& position, const Quaternion &orientation, const Vec3f &size, const Vec4f &colour){}

void DrawToolRHI::drawSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const Vec4f& color)
{
    internalDrawSpheres(points, radius, color);
}

void DrawToolRHI::drawSpheres(const std::vector<Vector3> &points, float radius, const Vec4f& color)
{
    std::vector<float> radii;
    radii.resize(points.size());
    std::fill(radii.begin(), radii.end(), radius);

    drawSpheres(points, radii, color);
}

void DrawToolRHI::drawFakeSpheres(const std::vector<Vector3> &points, const std::vector<float>& radius, const Vec4f& color)
{
    drawSpheres(points, radius, color);
}

void DrawToolRHI::drawFakeSpheres(const std::vector<Vector3> &points, float radius, const Vec4f& color)
{
    drawSpheres(points, radius, color);
}

void DrawToolRHI::drawCone(const Vector3& p1, const Vector3 &p2, float radius1, float radius2, const Vec4f& color, int subd){}

/// Draw a cube of size one centered on the current point.
void DrawToolRHI::drawCube(const float& radius, const Vec4f& color, const int& subd){}

void DrawToolRHI::drawCylinder(const Vector3& p1, const Vector3 &p2, float radius, const Vec4f& color, int subd){}

void DrawToolRHI::drawCapsule(const Vector3& p1, const Vector3 &p2, float radius, const Vec4f& color, int subd){}

void DrawToolRHI::drawArrow(const Vector3& p1, const Vector3 &p2, float radius, const Vec4f& color, int subd){}
void DrawToolRHI::drawArrow(const Vector3& p1, const Vector3 &p2, float radius, float coneLength, const Vec4f& color, int subd){}
void DrawToolRHI::drawArrow(const Vector3& p1, const Vector3& p2, float radius, float coneLength, float coneRadius, const Vec4f& color, int subd) {}

/// Draw a cross (3 lines) centered on p
void DrawToolRHI::drawCross(const Vector3&p, float length, const Vec4f& color){}

/// Draw a plus sign of size one centered on the current point.
void DrawToolRHI::drawPlus(const float& radius, const Vec4f& color, const int& subd ){}

void DrawToolRHI::drawPoint(const Vector3 &p, const Vec4f &c){}
void DrawToolRHI::drawPoint(const Vector3 &p, const Vector3 &n, const Vec4f &c){}

/// Quads methods
void DrawToolRHI::drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
    const Vector3 &normal){}
void DrawToolRHI::drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
    const Vector3 &normal, const Vec4f &c){}
void DrawToolRHI::drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
    const Vector3 &normal,
    const Vec4f &c1, const Vec4f &c2, const Vec4f &c3, const Vec4f &c4){}
void DrawToolRHI::drawQuad(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vector3 &p4,
    const Vector3 &normal1, const Vector3 &normal2, const Vector3 &normal3, const Vector3 &normal4,
    const Vec4f &c1, const Vec4f &c2, const Vec4f &c3, const Vec4f &c4){}
void DrawToolRHI::drawQuads(const std::vector<Vector3> &points, const Vec4f& color)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    drawQuads(points, colors);
}

void DrawToolRHI::drawQuads(const std::vector<Vector3> &points, const std::vector<Vec4f>& colors)
{
    std::vector<Vector3> normals;
    std::vector<Vec4i> indices;

    normals.reserve(points.size());
    indices.reserve(points.size() / 4);

    for(int i = 0 ; i< int(points.size()) ; i += 4)
    {
        int index[4] {int(i), int(i+1), int(i+2), int(i+3)};
        indices.push_back( Vec4i{index[0], index[1], index[2], index[3]} );

        Vector3 n0 = computeNormal(points[index[0]], points[index[1]], points[index[2]]);
        Vector3 n1 = computeNormal(points[index[2]], points[index[3]], points[index[0]]);
        Vector3 qn = (n0 + n1) * 0.5;
        qn.normalize();

        normals.push_back ( qn );
        normals.push_back ( qn );
        normals.push_back ( qn );
        normals.push_back ( qn );
    }

    internalDrawQuads(points, indices, normals, colors);

}

/// Tetrahedra methods
void DrawToolRHI::drawTetrahedron(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const Vec4f &colour){}
void DrawToolRHI::drawTetrahedra(const std::vector<Vector3> &points, const Vec4f& color)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    internalDrawTetrahedra(points,colors, 1.0f);
}

//Scale each tetrahedron
void DrawToolRHI::drawScaledTetrahedra(const std::vector<Vector3> &points, const Vec4f& color, const float scale)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    internalDrawTetrahedra(points,colors,scale);
}

/// Hexahedra methods

void DrawToolRHI::drawHexahedron(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3,
    const Vector3 &p4, const Vector3 &p5, const Vector3 &p6, const Vector3 &p7, const Vec4f &colour){}

void DrawToolRHI::drawHexahedra(const std::vector<Vector3> &points, const Vec4f& color)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    internalDrawHexahedra(points,colors, 1.0);
}

//Scale each hexahedron
void DrawToolRHI::drawScaledHexahedra(const std::vector<Vector3> &points, const Vec4f& color, const float scale)
{
    std::vector<Vec4f> colors;

    colors.resize(points.size());
    std::fill(colors.begin(), colors.end(), color);

    internalDrawHexahedra(points,colors, scale);
}

void DrawToolRHI::drawSphere(const Vector3 &p, float radius){}
void DrawToolRHI::drawSphere(const Vector3 &p, float radius, const Vec4f& colour) {}

void DrawToolRHI::drawEllipsoid(const Vector3 &p, const Vector3 &radii){}

void DrawToolRHI::drawBoundingBox(const Vector3 &min, const Vector3 &max, float size)
{
    std::vector<Vector3> points;
    std::vector<Vec2i> indices;

    points.reserve(8);
    points.push_back(Vector3{min[0], min[1], min[2]});
    points.push_back(Vector3{max[0], min[1], min[2]});
    points.push_back(Vector3{max[0], max[1], min[2]});
    points.push_back(Vector3{min[0], max[1], min[2]});

    points.push_back(Vector3{min[0], min[1], max[2]});
    points.push_back(Vector3{max[0], min[1], max[2]});
    points.push_back(Vector3{max[0], max[1], max[2]});
    points.push_back(Vector3{min[0], max[1], max[2]});

    indices.reserve(12);
    indices.push_back(Vec2i{0, 1});
    indices.push_back(Vec2i{1, 2});
    indices.push_back(Vec2i{2, 3});
    indices.push_back(Vec2i{3, 0});

    indices.push_back(Vec2i{4, 5});
    indices.push_back(Vec2i{5, 6});
    indices.push_back(Vec2i{6, 7});
    indices.push_back(Vec2i{7, 4});

    indices.push_back(Vec2i{0, 4});
    indices.push_back(Vec2i{1, 5});
    indices.push_back(Vec2i{2, 6});
    indices.push_back(Vec2i{3, 7});

    // color is supposed to be set with setMaterial
    // but still we set white by default
    std::vector<Vec4f> colors;
    colors.resize(8);
    std::fill(colors.begin(), colors.end(), Vec4f {1.0f, 1.0f, 1.0f, 1.0f});
    internalDrawLines(points, indices, size, colors);

}

void DrawToolRHI::draw3DText(const Vector3 &p, float scale, const Vec4f &color, const char* text)
{
    
}

void DrawToolRHI::draw3DText_Indices(const std::vector<Vector3> &positions, float scale, const Vec4f &color)
{

}

void DrawToolRHI::pushMatrix(){}

void DrawToolRHI::popMatrix(){}

void DrawToolRHI::multMatrix(float*){}

void DrawToolRHI::scale(float){}
void DrawToolRHI::translate(float x, float y, float z){}

void DrawToolRHI::setMaterial(const Vec4f &colour)
{

}

void DrawToolRHI::resetMaterial(const Vec4f &colour)
{
}

void DrawToolRHI::resetMaterial()
{
}

void DrawToolRHI::setPolygonMode(int _mode, bool _wireframe)
{

}

void DrawToolRHI::setLightingEnabled(bool _isAnabled){}

void DrawToolRHI::enablePolygonOffset( float factor, float units) {}
void DrawToolRHI::disablePolygonOffset(){}
void DrawToolRHI::enableBlending(){}
void DrawToolRHI::disableBlending(){}

void DrawToolRHI::enableLighting(){}
void DrawToolRHI::disableLighting(){}

void DrawToolRHI::enableDepthTest(){}
void DrawToolRHI::disableDepthTest(){}

void DrawToolRHI::saveLastState()
{
}

void DrawToolRHI::restoreLastState()
{

}

void DrawToolRHI::writeOverlayText(int x, int y, unsigned fontSize, const Vec4f &color, const char* text){}

void DrawToolRHI::readPixels(int x, int y, int w, int h, float* rgb, float* z){}

void DrawToolRHI::init()
{

}

void DrawToolRHI::clear()
{

}


} // namespace sofa::core::visual
