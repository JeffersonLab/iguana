#pragma once


#include "iguana/services/YAMLReader.h"
#include "iguana/algorithms/Algorithm.h"

namespace iguana::clas12
{

    /// @brief Filter the `REC::Particle` (or similar) bank by cutting on Z Vertex
    class ZVertexFilter : public Algorithm
    {

        DEFINE_IGUANA_ALGORITHM(ZVertexFilter, clas12::ZVertexFilter)

    public:
        /// Initialize an algorithm before any events are processed, with the intent to process _banks_;
        /// use this method if you intend to use `Algorithm::Run`.
        /// @param banks the list of banks this algorithm will use, so that `Algorithm::Run` can cache the indices
        ///        of the banks that it needs
        void Start(hipo::banklist &banks) override;
        /// Run an algorithm for an event
        /// @param banks the list of banks to process
        void Run(hipo::banklist &banks) const override;
        /// Finalize an algorithm after all events are processed
        void Stop() override;

        /// **Action function**: checks if the Z Vertex is within specified bounds
        /// @param zvertex the particle Z Vertex to check
        /// @returns `true` if `zvertex` is within specified bounds
        bool Filter(const double zvertex) const;

    private:
        /// `hipo::banklist` index for the particle bank
        hipo::banklist::size_type b_particle;

        /// Configuration options
        double zvertex_low;
        double zvertex_high;
    };

}
