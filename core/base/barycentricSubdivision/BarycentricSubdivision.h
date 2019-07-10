/// \ingroup base
/// \class ttk::BarycentricSubdivision
/// \author Pierre Guillou (pierre.guillou@lip6.fr
/// \date July 2019
///
/// \brief TTK %barycentricSubdivision processing package.
///
/// %BarycentricSubdivision is a TTK processing package that takes a scalar
/// field on the input and produces a scalar field on the output.
///
/// \sa ttk::Triangulation
/// \sa ttkBarycentricSubdivision.cpp %for a usage example.

#pragma once

// base code includes
#include <Triangulation.h>
#include <Wrapper.h>

namespace ttk {

  class BarycentricSubdivision : public Debug {

  public:
    inline void setOutputTriangulation(Triangulation *const triangulation) {
      outputTriangl_ = triangulation;
    }
    inline void setInputPoints(const void *const addr) {
      inputPoints_ = static_cast<const float *>(addr);
    }
    inline void setupTriangulation(Triangulation *const triangulation) {
      inputTriangl_ = triangulation;
      if(inputTriangl_ != nullptr) {
        inputTriangl_->preprocessVertexNeighbors();
        inputTriangl_->preprocessEdges();
        inputTriangl_->preprocessTriangles();
        inputTriangl_->preprocessTriangleEdges();
      }
    }

    inline SimplexId getNumberOfVertices() {
      if(inputTriangl_ == nullptr) {
        return 0;
      }
      return inputTriangl_->getNumberOfVertices()
             + inputTriangl_->getNumberOfEdges()
             + inputTriangl_->getNumberOfTriangles();
    }

    int execute();

    /**
     * @brief Interpolate floating-point point data on subdivised triangulation
     *
     * Copy values on parent vertices, interpolate on edges and barycenters
     *
     * @param[in] data Pointer to input data on parent triangulation
     * @param[out] output Allocated buffer to be filled
     *
     * @return 0 in case of success
     */
    template <typename T>
    int interpolateContinuousScalarField(const T *data, T *output) {
      if(inputTriangl_ == nullptr || outputTriangl_ == nullptr) {
        return 1;
      }
      const auto nInVerts = inputTriangl_->getNumberOfVertices();
      const auto nInEdges = inputTriangl_->getNumberOfEdges();
      const auto nInTriangles = inputTriangl_->getNumberOfTriangles();
      const auto nOutVerts = outputTriangl_->getNumberOfVertices();
      if(nOutVerts < 0 || nOutVerts != nInVerts + nInEdges + nInTriangles) {
        return 1;
      }

      // copy data on parent vertices
      std::copy(data, data + nInVerts, output);

      // interpolate on edges
      for(SimplexId i = 0; i < nInEdges; ++i) {
        SimplexId a{}, b{};
        inputTriangl_->getEdgeVertex(i, 0, a);
        inputTriangl_->getEdgeVertex(i, 0, b);
        output[nInVerts + i] = (data[a] + data[b]) / T{2.0};
      }

      // interpolate on triangle barycenters
      for(SimplexId i = 0; i < nInTriangles; ++i) {
        SimplexId a{}, b{}, c{};
        inputTriangl_->getTriangleVertex(i, 0, a);
        inputTriangl_->getTriangleVertex(i, 1, b);
        inputTriangl_->getTriangleVertex(i, 2, c);
        output[nInVerts + nInEdges + i]
          = (data[a] + data[b] + data[c]) / T{3.0};
      }
      return 0;
    }

    /**
     * @brief Interpolate integer point data on subdivised triangulation
     *
     * Copy values on parent vertices, put 0 elsewhere.
     *
     * @param[in] data Pointer to input data on parent triangulation
     * @param[out] output Allocated buffer to be filled
     *
     * @return 0 in case of success
     */
    template <typename T>
    int interpolateDiscreteScalarField(const T *data, T *output) {
      if(inputTriangl_ == nullptr || outputTriangl_ == nullptr) {
        return 1;
      }
      const auto nInVerts = inputTriangl_->getNumberOfVertices();
      const auto nOutVerts = outputTriangl_->getNumberOfVertices();
      if(nOutVerts < 0 || nOutVerts < nInVerts) {
        return 1;
      }
      std::fill(output, output + nOutVerts, T{0});
      std::copy(data, data + nInVerts, output);
      return 0;
    }

    /**
     * @brief Interpolate cell data on subdivised triangulation
     *
     * Copy parent triangle value on the six children triangles.
     *
     * @param[in] data Pointer to input data on parent triangulation
     * @param[out] output Allocated buffer to be filled
     *
     * @return 0 in case of success
     */
    template <typename T>
    int interpolateCellDataField(const T *data, T *output) {
      if(inputTriangl_ == nullptr || outputTriangl_ == nullptr) {
        return 1;
      }
      const size_t newTrianglesPerParent{6};
      const auto nInTriangles = inputTriangl_->getNumberOfTriangles();
      const auto nOutTriangles = outputTriangl_->getNumberOfTriangles();
      if(nOutTriangles < 0
         || nOutTriangles != newTrianglesPerParent * nInTriangles) {
        return 1;
      }
      for(SimplexId i = 0; i < nInTriangles; ++i) {
        output[i * newTrianglesPerParent + 0] = data[i];
        output[i * newTrianglesPerParent + 1] = data[i];
        output[i * newTrianglesPerParent + 2] = data[i];
        output[i * newTrianglesPerParent + 3] = data[i];
        output[i * newTrianglesPerParent + 4] = data[i];
        output[i * newTrianglesPerParent + 5] = data[i];
      }
      return 0;
    }

  private:
    int subdiviseTriangulation();
    int buildOutputTriangulation();

    // input triangulation
    Triangulation *inputTriangl_{};
    // array of input points coordinates
    const float *inputPoints_{};
    // list of input point data
    std::vector<void *> pointData_{};
    // list of input cell data
    std::vector<void *> cellData_{};

  public:
    // output 3D coordinates of generated points: old points first, then edge
    // middles, then triangle barycenters
    std::vector<float> points_{};
    // output triangles
    std::vector<LongSimplexId> cells_{};
    // output triangulation built on output points & output cells
    Triangulation *outputTriangl_{};
    // generated point cell id
    std::vector<SimplexId> pointId_{};
    // generated points dimension: 0 vertex of parent triangulation, 1 edge
    // middle, 2 triangle barycenter
    std::vector<SimplexId> pointDim_{};
  };
} // namespace ttk
