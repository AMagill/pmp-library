//=============================================================================
// Copyright (C) 2001-2005 by Computer Graphics Group, RWTH Aachen
// Copyright (C) 2011 by Graphics & Geometry Group, Bielefeld University
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation, version 2.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <surface_mesh/EdgeSet.h>
#include <cmath>
#include <exception>

//=============================================================================

namespace surface_mesh {

//=============================================================================

EdgeSet::EdgeSet() : PointSet()
{
    // allocate standard properties
    // same list is used in operator=() and assign()
    m_vconn    = addVertexProperty<VertexConnectivity>("v:connectivity");
    m_hconn    = addHalfedgeProperty<HalfedgeConnectivity>("h:connectivity");
    m_edeleted = addEdgeProperty<bool>("e:deleted", false);
    m_deletedEdges = 0;
}

//-----------------------------------------------------------------------------

EdgeSet::~EdgeSet()
{
}

//-----------------------------------------------------------------------------

EdgeSet& EdgeSet::operator=(const EdgeSet& rhs)
{
    PointSet::operator=(rhs);

    if (this != &rhs)
    {
        // deep copy of property containers
        m_hprops = rhs.m_hprops;
        m_eprops = rhs.m_eprops;

        // property handles contain pointers, have to be reassigned
        m_vconn    = vertexProperty<VertexConnectivity>("v:connectivity");
        m_hconn    = halfedgeProperty<HalfedgeConnectivity>("h:connectivity");
        m_edeleted = edgeProperty<bool>("e:deleted");

        // how many elements are deleted?
        m_deletedEdges = rhs.m_deletedEdges;
    }

    return *this;
}

//-----------------------------------------------------------------------------

EdgeSet& EdgeSet::assign(const EdgeSet& rhs)
{
    PointSet::assign(rhs);

    if (this != &rhs)
    {
        // clear properties
        m_hprops.clear();
        m_eprops.clear();

        // allocate standard properties
        m_vconn = addVertexProperty<VertexConnectivity>("v:connectivity");
        m_hconn = addHalfedgeProperty<HalfedgeConnectivity>("h:connectivity");
        m_edeleted = addEdgeProperty<bool>("e:deleted", false);

        // copy properties from other mesh
        m_vconn.array()    = rhs.m_vconn.array();
        m_hconn.array()    = rhs.m_hconn.array();
        m_edeleted.array() = rhs.m_edeleted.array();

        // resize (needed by property containers)
        m_hprops.resize(rhs.halfedgesSize());
        m_eprops.resize(rhs.edgesSize());

        // how many elements are deleted?
        m_deletedEdges = rhs.m_deletedEdges;
    }

    return *this;
}

//-----------------------------------------------------------------------------

// bool EdgeSet::read(const std::string& filename)
// {
//     // can also read point sets of course
//     if (read(filename)) return true;

//     if (PointSet::read(filename)) return true;
//     return false;
// }

// bool EdgeSet::read_knot(const std::string& filename)
// {
//     LOG(Log_info) << "Knot_structure: reading " << filename << std::flush;

//     // check file extension
//     std::string::size_type dot(filename.rfind("."));
//     if (dot == std::string::npos) return false;
//     std::string ext = filename.substr(dot + 1, filename.length() - dot - 1);
//     std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
//     //if (ext != "knt")
//     if ((ext != "knt") && (ext != "vect"))
//     {
//         std::cerr << "wrong extension\n";
//         return false;
//     }
//     LOG(Log_info) << ext << std::endl;

//     // open file
//     FILE* in = fopen(filename.c_str(), "r");
//     if (!in)
//     {
//         std::cerr << "cannot open file\n";
//         return false;
//     }

//     // clear vectors first
//     this->clear();

//     char         line[100], dummy_string[9];
//     unsigned int nV, nE;
//     unsigned int i, items;
//     int          idx_i, idx_j;
//     Point        p;

//     if (ext == "knt")
//     {
//         // skip every line before "vertices..."
//         do
//         {
//             if (fgets(line, 100, in) != NULL)
//             {
//                 items = sscanf(line, "%s", dummy_string);
//             }
//         } while (std::strncmp(dummy_string, "vertices", 8));

//         // read #vertices, #edges
//         items =
//             sscanf(line, "%s %u %s %u", dummy_string, &nV, dummy_string, &nE);

//         // read vertices
//         for (i = 0; i < nV && !feof(in); ++i)
//         {
//             if (fgets(line, 100, in) != NULL)
//             {
//                 items = sscanf(line, "%f %f %f", &p[0], &p[1], &p[2]);
//                 addVertex(p);
//             }
//         }

//         // read edges
//         for (i = 0; i < nE && !feof(in); ++i)
//         {
//             if (fgets(line, 100, in) != NULL)
//             {
//                 items = sscanf(line, "%i %i", &idx_i, &idx_j);
//                 insertEdge(Vertex(idx_i), Vertex(idx_j));
//             }
//         }

//         fclose(in);

//         LOG(Log_info) << "done (" << verticesSize() << " vertices, "
//                       << edgesSize() / 2 << " edges)\n";
//     }
//     else if (ext == "vect")
//     {
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-result"

//         unsigned int n_comp, h;

//         // skip 1st line
//         fgets(line, 100, in);
//         fgets(line, 100, in);
//         // read #components, #vertices
//         items = sscanf(line, "%u %u", &n_comp, &nV);
//         std::vector<unsigned int> v_per_comp(n_comp);

//         fgets(line, 100, in);
//         fgets(line, 100, in);

//         if (n_comp == 1)
//         {
//             items = sscanf(line, "%u", &v_per_comp[0]);
//         }
//         else if (n_comp == 2)
//         {
//             items = sscanf(line, "%u %u", &v_per_comp[0], &v_per_comp[1]);
//         }
//         else if (n_comp == 3)
//         {
//             items = sscanf(line, "%u %u %u", &v_per_comp[0], &v_per_comp[1],
//                            &v_per_comp[2]);
//         }
//         else
//         {
//             LOG(Log_info) << "to many components" << std::endl;
//             return false;
//         }
//         fgets(line, 100, in);

//         LOG(Log_info) << nV << ' ' << v_per_comp[1] << ' ' << n_comp
//                       << std::endl;
//         // read vertices
//         for (unsigned int j = 0; j < n_comp; ++j)
//         {
//             fgets(line, 100, in);
//             for (i = 0; i < v_per_comp[j] && !feof(in); ++i)
//             {
//                 if (fgets(line, 100, in) != NULL)
//                 {
//                     items = sscanf(line, "%f %f %f", &p[0], &p[1], &p[2]);
//                     addVertex((Point)p);
//                 }
//             }
//         }

// #pragma GCC diagnostic pop

//         // set edges
//         h = 0;
//         for (unsigned int j = 0; j < n_comp; ++j)
//         {
//             for (i = 0; i < v_per_comp[j]; ++i)
//             {

//                 if (i < v_per_comp[j] - 1)
//                 {
//                     insertEdge(Vertex(h + i), Vertex(h + i + 1));
//                 }
//                 else
//                 {
//                     insertEdge(Vertex(h + i), Vertex(h));
//                 }
//             }
//             h += v_per_comp[j];
//         }
//         fclose(in);

//         LOG(Log_info) << "done (" << verticesSize() << " vertices, "
//                       << edgesSize() / 2 << " edges)\n";
//     }

//     /*calc_nbrs_of_edges();
//          default_length = 0;
//          for (unsigned int i = 0; i < edges_.size(); ++ ++i)
//          {
//              default_length += norm(vertices_[edges_[i]] - vertices_[edges_[i + 1]]);
//          }*/
//     //std::cout << "def len read" << default_length << std::endl;
//     return true;
// }

// //-----------------------------------------------------------------------------

// bool EdgeSet::write(const std::string& filename) const
// {
//     return PointSet::write(filename);
//     //return write_mesh(*this, filename);
// }

//-----------------------------------------------------------------------------

void EdgeSet::clear()
{
    m_hprops.resize(0);
    m_eprops.resize(0);

    freeMemory();

    m_deletedEdges = 0;

    PointSet::clear();
}

//-----------------------------------------------------------------------------

void EdgeSet::freeMemory()
{
    m_hprops.freeMemory();
    m_eprops.freeMemory();

    PointSet::freeMemory();
}

//-----------------------------------------------------------------------------

void EdgeSet::reserve(unsigned int nvertices, unsigned int nedges)
{
    PointSet::reserve(nvertices);

    m_hprops.reserve(2 * nedges);
    m_eprops.reserve(nedges);
}

//-----------------------------------------------------------------------------

void EdgeSet::propertyStats() const
{
    std::vector<std::string> props;

    PointSet::propertyStats();

    std::cout << "halfedge properties:\n";
    props = halfedgeProperties();
    for (unsigned int i = 0; i < props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;

    std::cout << "edge properties:\n";
    props = edgeProperties();
    for (unsigned int i = 0; i < props.size(); ++i)
        std::cout << "\t" << props[i] << std::endl;
}

//-----------------------------------------------------------------------------

EdgeSet::Halfedge EdgeSet::findHalfedge(Vertex start, Vertex end) const
{
    assert(isValid(start) && isValid(end));

    Halfedge       h  = halfedge(start);
    const Halfedge hh = h;

    if (h.isValid())
    {
        do
        {
            if (toVertex(h) == end)
                return h;
            h = cwRotatedHalfedge(h);
        } while (h != hh);
    }

    return Halfedge();
}

//-----------------------------------------------------------------------------

EdgeSet::Edge EdgeSet::findEdge(Vertex a, Vertex b) const
{
    Halfedge h = findHalfedge(a, b);
    return h.isValid() ? edge(h) : Edge();
}

//-----------------------------------------------------------------------------

unsigned int EdgeSet::valence(Vertex v) const
{
    unsigned int count(0);

    VertexAroundVertexCirculator vvit  = vertices(v);
    VertexAroundVertexCirculator vvend = vvit;
    if (vvit)
        do
        {
            ++count;
        } while (++vvit != vvend);

    return count;
}

//-----------------------------------------------------------------------------

void EdgeSet::updateVertexNormals()
{
    if (!m_vnormal)
        m_vnormal = vertexProperty<Point>("v:normal");

    for (auto v : vertices())
        m_vnormal[v] = computeVertexNormal(v);
}

//-----------------------------------------------------------------------------

Normal EdgeSet::computeVertexNormal(Vertex v) const
{
    Point    nn(0, 0, 0);
    Halfedge h = halfedge(v);

    if (h.isValid())
    {
        const Halfedge hend = h;
        const Point    p0   = m_vpoint[v];

        Point  n, p1, p2;
        Scalar cosine, angle, denom;

        do
        {
            if (!isBoundary(h))
            {
                p1 = m_vpoint[toVertex(h)];
                p1 -= p0;

                p2 = m_vpoint[fromVertex(prevHalfedge(h))];
                p2 -= p0;

                // check whether we can robustly compute angle
                denom = sqrt(dot(p1, p1) * dot(p2, p2));
                if (denom > std::numeric_limits<Scalar>::min())
                {
                    cosine = dot(p1, p2) / denom;
                    if (cosine < -1.0)
                        cosine = -1.0;
                    else if (cosine > 1.0)
                        cosine = 1.0;
                    angle      = acos(cosine);

                    n = cross(p1, p2);

                    // check whether normal is != 0
                    denom = norm(n);
                    if (denom > std::numeric_limits<Scalar>::min())
                    {
                        n *= angle / denom;
                        nn += n;
                    }
                }
            }

            h = cwRotatedHalfedge(h);
        } while (h != hend);

        nn.normalize();
    }

    return nn;
}

//-----------------------------------------------------------------------------

Scalar EdgeSet::edgeLength(Edge e) const
{
    return norm(m_vpoint[vertex(e, 0)] - m_vpoint[vertex(e, 1)]);
}

//-----------------------------------------------------------------------------

EdgeSet::Halfedge EdgeSet::insertVertex(Halfedge h0, Vertex v)
{
    // before:
    //
    // v0      h0       v2
    //  o--------------->o
    //   <---------------
    //         o0
    //
    // after:
    //
    // v0  h0   v   h1   v2
    //  o------>o------->o
    //   <------ <-------
    //     o0       o1

    Halfedge h2 = nextHalfedge(h0);
    Halfedge o0 = oppositeHalfedge(h0);
    Halfedge o2 = prevHalfedge(o0);
    Vertex   v2 = toVertex(h0);

    Halfedge h1 = newEdge(v, v2);
    Halfedge o1 = oppositeHalfedge(h1);

    // adjust halfedge connectivity
    setNextHalfedge(h1, h2);
    setNextHalfedge(h0, h1);
    setVertex(h0, v);
    setVertex(h1, v2);

    setNextHalfedge(o1, o0);
    setNextHalfedge(o2, o1);
    setVertex(o1, v);

    // adjust vertex connectivity
    setHalfedge(v2, o1);
    setHalfedge(v, h1);

    return o1;
}

//-----------------------------------------------------------------------------

EdgeSet::Halfedge EdgeSet::insertEdge(Vertex v0, Vertex v1)
{
    //  ^                       ^
    //  |                       |
    // he(v0)                 he(v1)
    //  |                       |
    // v0 -------> h0 -------> v1
    // v0 <------- h1 -------- v1
    //  ^                       ^
    //  |                       |
    // prev(he(v0))        prev(he(v1))
    //  |                       |

    // edge already exists
    assert(!findEdge(v0, v1).isValid());

    const Vertex   v[2] = {v0, v1};
    const Halfedge h[2] = {newEdge(v0, v1), oppositeHalfedge(h[0])};

    Halfedge he0 = halfedge(v[0]);
    Halfedge he1 = halfedge(v[1]);

    if (he1.isValid())
    {
        Halfedge p1 = prevHalfedge(he1);

        setNextHalfedge(h[0], he1);
        setPrevHalfedge(h[1], p1);
    }
    else
    {
        setNextHalfedge(h[0], h[1]);
    }

    if (he0.isValid())
    {
        Halfedge p0  = prevHalfedge(he0);
        Halfedge op0 = oppositeHalfedge(p0);

        setNextHalfedge(h[1], op0);
        setPrevHalfedge(h[0], oppositeHalfedge(he0));
    }
    else
    {
        setNextHalfedge(h[1], h[0]);
    }

    setHalfedge(v[0], h[0]);
    setHalfedge(v[1], h[1]);

    return h[0];
}

//-----------------------------------------------------------------------------

void EdgeSet::deleteVertex(Vertex v)
{
    if (isDeleted(v))
        return;

    // collect edges to be deleted
    std::vector<Edge> edges;
    for (auto h : halfedges(v))
        if (h.isValid())
            edges.push_back(edge(h));

    for (auto e : edges)
        deleteEdge(e);

    PointSet::deleteVertex(v);
}

//-----------------------------------------------------------------------------

void EdgeSet::deleteEdge(Edge e)
{
    if (isDeleted(e))
        return;

    Halfedge h0, h1, next0, next1, prev0, prev1;
    Vertex   v0, v1;

    h0    = halfedge(e, 0);
    v0    = toVertex(h0);
    next0 = nextHalfedge(h0);
    prev0 = prevHalfedge(h0);

    h1    = halfedge(e, 1);
    v1    = toVertex(h1);
    next1 = nextHalfedge(h1);
    prev1 = prevHalfedge(h1);

    // adjust next and prev handles
    setNextHalfedge(prev0, next1);
    setNextHalfedge(prev1, next0);

    // update v0
    if (halfedge(v0) == h1)
    {
        if (next0 == h1)
        {
            PointSet::deleteVertex(v0);
        }
        else
            setHalfedge(v0, next0);
    }

    // update v1
    if (halfedge(v1) == h0)
    {
        if (next1 == h0)
        {
            PointSet::deleteVertex(v1);
        }
        else
            setHalfedge(v1, next1);
    }

    // mark the edge for deletion
    markEdgeRemoved(e);
}

//-----------------------------------------------------------------------------

void EdgeSet::markEdgeRemoved(Edge e)
{
    if (!m_edeleted)
        m_edeleted = edgeProperty<bool>("e:deleted", false);

    m_edeleted[e] = true;

    ++m_deletedEdges;

    setGarbage();
}

//-----------------------------------------------------------------------------

void EdgeSet::beginGarbage()
{
    // delete marked vertices
    PointSet::beginGarbage();

    const int nV = m_garbageprops["nV"];

    int i0, i1, nE(edgesSize()), nH(halfedgesSize());

    Vertex   v;
    Halfedge h;

    // setup handle mapping
    const VertexProperty<Vertex> vmap =
        getVertexProperty<Vertex>("v:garbage-collection");
    HalfedgeProperty<Halfedge> hmap =
        addHalfedgeProperty<Halfedge>("h:garbage-collection");

    for (int i(0); i < nH; ++i)
        hmap[Halfedge(i)] = Halfedge(i);

    // remove deleted edges
    if (nE > 0)
    {
        i0 = 0;
        i1 = nE - 1;

        while (1)
        {
            // find first deleted and last un-deleted
            while (!m_edeleted[Edge(i0)] && i0 < i1)
                ++i0;
            while (m_edeleted[Edge(i1)] && i0 < i1)
                --i1;
            if (i0 >= i1)
                break;

            // swap
            m_eprops.swap(i0, i1);
            m_hprops.swap(2 * i0, 2 * i1);
            m_hprops.swap(2 * i0 + 1, 2 * i1 + 1);
        };

        // remember new size
        nE = m_edeleted[Edge(i0)] ? i0 : i0 + 1;
        nH = 2 * nE;
    }

    // update vertex connectivity
    for (int i = 0; i < nV; ++i)
    {
        v = Vertex(i);
        if (!isIsolated(v))
            setHalfedge(v, hmap[halfedge(v)]);
    }

    // update halfedge connectivity
    for (int i = 0; i < nH; ++i)
    {
        h = Halfedge(i);
        setVertex(h, vmap[toVertex(h)]);
        setNextHalfedge(h, hmap[nextHalfedge(h)]);
    }

    m_garbageprops["nH"] = nH;
    m_garbageprops["nE"] = nE;
}

//-----------------------------------------------------------------------------

void EdgeSet::finalizeGarbage()
{
    HalfedgeProperty<Halfedge> hmap =
        getHalfedgeProperty<Halfedge>("h:garbage-collection");

    // remove handle maps
    removeHalfedgeProperty(hmap);

    // finally resize arrays
    m_hprops.resize(m_garbageprops["nH"]);
    m_hprops.freeMemory();
    m_eprops.resize(m_garbageprops["nE"]);
    m_eprops.freeMemory();

    m_deletedEdges = 0;

    PointSet::finalizeGarbage();
}

//=============================================================================
} // namespace surface_mesh
//=============================================================================