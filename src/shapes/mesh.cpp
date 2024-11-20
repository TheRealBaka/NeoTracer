#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which
 * share an index and vertex buffer. Since individual triangles are rarely
 * needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of
     * the element corresponds to one vertex index (into @c m_vertices ) of the
     * triangle. This list will always contain as many elements as there are
     * triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be
     * fewer than @code 3 * numTriangles @endcode vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging
    /// purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the
    /// geometric normal instead.
    bool m_smoothNormals;

protected:
    int numberOfPrimitives() const override { return int(m_triangles.size()); }

    bool intersect(int primitiveIndex, const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        // NOT_IMPLEMENTED

        // hints:
        // * use m_triangles[primitiveIndex] to get the vertex indices of the
        // triangle that should be intersected
        // * if m_smoothNormals is true, interpolate the vertex normals from
        // m_vertices
        //   * make sure that your shading frame stays orthonormal!
        // * if m_smoothNormals is false, use the geometrical normal (can be
        // computed from the vertex positions)
        
        int i0 = m_triangles[primitiveIndex][0];
        int i1 = m_triangles[primitiveIndex][1];
        int i2 = m_triangles[primitiveIndex][2];

        Vertex p0 = m_vertices[i0];
        Vertex p1 = m_vertices[i1];
        Vertex p2 = m_vertices[i2];

        // compute edges
        Vector e0 = p1.position - p0.position;
        Vector e1 = p2.position - p0.position;

        Vector normal = e0.cross(e1).normalized();

        //check if triangle is degenerate 
        if (normal.lengthSquared() == 0)
            return false;

        Vector p_vec = ray.direction.cross(e1);
        float det = e0.dot(p_vec);

        //check for backface culling
        // if (det < Epsilon) return false;

        if (fabs(det) < 1e-6) return false; // Reducing epsilon made it work for bunny (?)

        float inv_det = 1 / det;

        Vector t_vec = ray.origin - p0.position;
        float u = t_vec.dot(p_vec) * inv_det;
        if (u < 0 || u > 1) return false;

        Vector q_vec = t_vec.cross(e0);
        float v = ray.direction.dot(q_vec) * inv_det;
        if (v < 0 || u + v > 1) return false;
        
        float t = e1.dot(q_vec) * inv_det;

        if (t < Epsilon || t > its.t) return false;

        its.t = t;
        its.geometryNormal = normal;

        its.uv = interpolateBarycentric(Vector2(u, v), p0.uv, p1.uv, p2.uv);

        if (m_smoothNormals)
            // its.shadingNormal = ((1 - u - v) * p0.normal + u * p1.normal + v * p2.normal).normalized();
            its.shadingNormal = interpolateBarycentric(Vector2(u, v), p0.normal, p1.normal, p2.normal).normalized();
        else
            its.shadingNormal = normal;

        its.tangent = e0.normalized(); // Creating tangent from a vector on the mesh plane

        its.pdf = 0.0f;
        its.position = ray(its.t);
        
        return true;
    
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        // NOT_IMPLEMENTED
        int i0 = m_triangles[primitiveIndex][0];
        int i1 = m_triangles[primitiveIndex][1];
        int i2 = m_triangles[primitiveIndex][2];

        Vertex p0 = m_vertices[i0];
        Vertex p1 = m_vertices[i1];
        Vertex p2 = m_vertices[i2];

        auto x_min = std::min({p0.position[0], p1.position[0], p2.position[0]});
        auto y_min = std::min({p0.position[1], p1.position[1], p2.position[1]});
        auto z_min = std::min({p0.position[2], p1.position[2], p2.position[2]});

        
        auto x_max = std::max({p0.position[0], p1.position[0], p2.position[0]});
        auto y_max = std::max({p0.position[1], p1.position[1], p2.position[1]});
        auto z_max = std::max({p0.position[2], p1.position[2], p2.position[2]});

        return Bounds(Point(x_min, y_min, z_min), Point(x_max, y_max, z_max));
    }

    Point getCentroid(int primitiveIndex) const override {
        // NOT_IMPLEMENTED

        int i0 = m_triangles[primitiveIndex][0];
        int i1 = m_triangles[primitiveIndex][1];
        int i2 = m_triangles[primitiveIndex][2];

        Vertex p0 = m_vertices[i0];
        Vertex p1 = m_vertices[i1];
        Vertex p2 = m_vertices[i2];

        auto x = (p0.position[0] + p1.position[0] + p2.position[0]) / 3;
        auto y = (p0.position[1] + p1.position[1] + p2.position[1]) / 3;
        auto z = (p0.position[2] + p1.position[2] + p2.position[2]) / 3;

        Point centroid = Point(x, y, z);
        return centroid;

    }

public:
    TriangleMesh(const Properties &properties) {
        m_originalPath  = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath, m_triangles, m_vertices);
        logger(EInfo,
               "loaded ply with %d triangles, %d vertices",
               m_triangles.size(),
               m_vertices.size());
        buildAccelerationStructure();
    }

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        PROFILE("Triangle mesh")
        return AccelerationStructure::intersect(ray, its, rng);
    }

    AreaSample sampleArea(Sampler &rng) const override{
        // only implement this if you need triangle mesh area light sampling for
        // your rendering competition
        NOT_IMPLEMENTED
    }

    std::string toString() const override {
        return tfm::format(
            "Mesh[\n"
            "  vertices = %d,\n"
            "  triangles = %d,\n"
            "  filename = \"%s\"\n"
            "]",
            m_vertices.size(),
            m_triangles.size(),
            m_originalPath.generic_string());
    }
};

} // namespace lightwave

REGISTER_SHAPE(TriangleMesh, "mesh")
