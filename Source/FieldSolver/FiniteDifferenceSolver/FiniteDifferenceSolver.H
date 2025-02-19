/* Copyright 2020 Remi Lehe
 *
 * This file is part of WarpX.
 *
 * License: BSD-3-Clause-LBNL
 */

#ifndef WARPX_FINITE_DIFFERENCE_SOLVER_H_
#define WARPX_FINITE_DIFFERENCE_SOLVER_H_

#include "EmbeddedBoundary/WarpXFaceInfoBox_fwd.H"
#include "FiniteDifferenceSolver_fwd.H"

#include "BoundaryConditions/PML_fwd.H"
#include "Evolve/WarpXDtType.H"
#include "HybridPICModel/HybridPICModel_fwd.H"
#include "MacroscopicProperties/MacroscopicProperties_fwd.H"

#include <AMReX_GpuContainers.H>
#include <AMReX_REAL.H>

#include <AMReX_BaseFwd.H>

#include <array>
#include <memory>

/**
 * \brief Top-level class for the electromagnetic finite-difference solver
 *
 * Stores the coefficients of the finite-difference stencils,
 * and has member functions to update fields over one time step.
 */
class FiniteDifferenceSolver
{
    public:

        // Constructor
        /** \brief Initialize the finite-difference Maxwell solver (for a given refinement level)
         *
         * This function initializes the stencil coefficients for the chosen finite-difference algorithm
         *
         * \param fdtd_algo Identifies the chosen algorithm, as defined in WarpXAlgorithmSelection.H
         * \param cell_size Cell size along each dimension, for the chosen refinement level
         * \param grid_type Whether the solver is applied to a collocated or staggered grid
         */
        FiniteDifferenceSolver (
            int fdtd_algo,
            std::array<amrex::Real,3> cell_size,
            short grid_type );

        void EvolveB ( std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Bfield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
                       std::unique_ptr<amrex::MultiFab> const& Gfield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& face_areas,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& area_mod,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 >& ECTRhofield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Venl,
                       std::array< std::unique_ptr<amrex::iMultiFab>, 3 >& flag_info_cell,
                       std::array< std::unique_ptr<amrex::LayoutData<FaceInfoBox> >, 3 >& borrowing,
                       int lev, amrex::Real dt );

        void EvolveE ( std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Efield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Bfield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jfield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& face_areas,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 >& ECTRhofield,
                       std::unique_ptr<amrex::MultiFab> const& Ffield,
                       int lev, amrex::Real dt );

        void EvolveF ( std::unique_ptr<amrex::MultiFab>& Ffield,
                       std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
                       std::unique_ptr<amrex::MultiFab> const& rhofield,
                       int rhocomp,
                       amrex::Real dt );

        void EvolveG (std::unique_ptr<amrex::MultiFab>& Gfield,
                      std::array<std::unique_ptr<amrex::MultiFab>,3> const& Bfield,
                      amrex::Real dt);

        void EvolveECTRho ( std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
                            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
                            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& face_areas,
                            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& ECTRhofield,
                            int lev );

        void ApplySilverMuellerBoundary(
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Efield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Bfield,
            amrex::Box domain_box,
            amrex::Real dt,
            amrex::Vector<int> field_boundary_lo,
            amrex::Vector<int> field_boundary_hi);

        void ComputeDivE ( const std::array<std::unique_ptr<amrex::MultiFab>,3>& Efield,
                           amrex::MultiFab& divE );

        /**
          * \brief Macroscopic E-update for non-vacuum medium using the user-selected
          * finite-difference algorithm and macroscopic sigma-method defined in
          * WarpXAlgorithmSelection.H
          *
          * \param[out] Efield  vector of electric field MultiFabs updated at a given level
          * \param[in] Bfield   vector of magnetic field MultiFabs at a given level
          * \param[in] Jfield   vector of current density MultiFabs at a given level
          * \param[in] edge_lengths length of edges along embedded boundaries
          * \param[in] dt       timestep of the simulation
          * \param[in] macroscopic_properties contains user-defined properties of the medium.
          */
        void MacroscopicEvolveE ( std::array< std::unique_ptr<amrex::MultiFab>, 3>& Efield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Bfield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jfield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
                      amrex::Real dt,
                      std::unique_ptr<MacroscopicProperties> const& macroscopic_properties);

        void EvolveBPML ( std::array< amrex::MultiFab*, 3 > Bfield,
                      std::array< amrex::MultiFab*, 3 > Efield,
                      amrex::Real dt,
                      bool dive_cleaning);

       void EvolveEPML ( std::array< amrex::MultiFab*, 3 > Efield,
                      std::array< amrex::MultiFab*, 3 > Bfield,
                      std::array< amrex::MultiFab*, 3 > Jfield,
                      std::array< amrex::MultiFab*, 3 >  edge_lengths,
                      amrex::MultiFab* Ffield,
                      MultiSigmaBox const& sigba,
                      amrex::Real dt, bool pml_has_particles );

       void EvolveFPML ( amrex::MultiFab* Ffield,
                     std::array< amrex::MultiFab*, 3 > Efield,
                     amrex::Real dt );

        /**
          * \brief E-update in the hybrid PIC algorithm as described in
          * Winske et al. (2003) Eq. 10.
          * https://link.springer.com/chapter/10.1007/3-540-36530-3_8
          *
          * \param[out] Efield  vector of electric field MultiFabs updated at a given level
          * \param[in] Jfield   vector of total current MultiFabs at a given level
          * \param[in] Jifield  vector of ion current density MultiFabs at a given level
          * \param[in] Jextfield  vector of external current density MultiFabs at a given level
          * \param[in] Bfield   vector of magnetic field MultiFabs at a given level
          * \param[in] rhofield scalar ion charge density Multifab at a given level
          * \param[in] Pefield  scalar electron pressure MultiFab at a given level
          * \param[in] edge_lengths length of edges along embedded boundaries
          * \param[in] lev  level number for the calculation
          * \param[in] hybrid_model instance of the hybrid-PIC model
          * \param[in] include_resistivity_term boolean flag for whether to include resistivity
          */
        void HybridPICSolveE ( std::array< std::unique_ptr<amrex::MultiFab>, 3>& Efield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3>& Jfield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jifield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jextfield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Bfield,
                      std::unique_ptr<amrex::MultiFab> const& rhofield,
                      std::unique_ptr<amrex::MultiFab> const& Pefield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
                      int lev, HybridPICModel const* hybrid_model,
                      bool include_resistivity_term );

        /**
          * \brief Calculation of total current using Ampere's law (without
          * displacement current): J = (curl x B) / mu0.
          *
          * \param[out] Jfield  vector of current MultiFabs at a given level
          * \param[in] Bfield   vector of magnetic field MultiFabs at a given level
          * \param[in] edge_lengths length of edges along embedded boundaries
          * \param[in] lev  level number for the calculation
          */
        void CalculateCurrentAmpere (
                      std::array< std::unique_ptr<amrex::MultiFab>, 3>& Jfield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Bfield,
                      std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
                      int lev );

    private:

        int m_fdtd_algo;
        short m_grid_type;

#ifdef WARPX_DIM_RZ
        amrex::Real m_dr, m_rmin;
        int m_nmodes;
        // host-only
        amrex::Vector<amrex::Real> m_h_stencil_coefs_r, m_h_stencil_coefs_z;
        // device copy after init
        amrex::Gpu::DeviceVector<amrex::Real> m_stencil_coefs_r;
        amrex::Gpu::DeviceVector<amrex::Real> m_stencil_coefs_z;
#else
        // host-only
        amrex::Vector<amrex::Real> m_h_stencil_coefs_x, m_h_stencil_coefs_y, m_h_stencil_coefs_z;
        // device copy after init
        amrex::Gpu::DeviceVector<amrex::Real> m_stencil_coefs_x;
        amrex::Gpu::DeviceVector<amrex::Real> m_stencil_coefs_y;
        amrex::Gpu::DeviceVector<amrex::Real> m_stencil_coefs_z;
#endif

    public:
        // The member functions below contain extended __device__ lambda.
        // In order to compile with nvcc, they need to be public.

#ifdef WARPX_DIM_RZ
        template< typename T_Algo >
        void EvolveBCylindrical (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
            int lev,
            amrex::Real dt );

        template< typename T_Algo >
        void EvolveECylindrical (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Efield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jfield,
            std::unique_ptr<amrex::MultiFab> const& Ffield,
            int lev,
            amrex::Real dt );

        template< typename T_Algo >
        void EvolveFCylindrical (
            std::unique_ptr<amrex::MultiFab>& Ffield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
            std::unique_ptr<amrex::MultiFab> const& rhofield,
            int rhocomp,
            amrex::Real dt );

        template< typename T_Algo >
        void ComputeDivECylindrical (
            const std::array<std::unique_ptr<amrex::MultiFab>,3>& Efield,
            amrex::MultiFab& divE );

        template<typename T_Algo>
        void HybridPICSolveECylindrical (
            std::array< std::unique_ptr<amrex::MultiFab>, 3>& Efield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Jfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Jifield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jextfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Bfield,
            std::unique_ptr<amrex::MultiFab> const& rhofield,
            std::unique_ptr<amrex::MultiFab> const& Pefield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            int lev, HybridPICModel const* hybrid_model,
            bool include_resistivity_term );

        template<typename T_Algo>
        void CalculateCurrentAmpereCylindrical (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Jfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            int lev
        );

#else
        template< typename T_Algo >
        void EvolveBCartesian (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
            std::unique_ptr<amrex::MultiFab> const& Gfield,
            int lev, amrex::Real dt );

        template< typename T_Algo >
        void EvolveECartesian (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Efield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            std::unique_ptr<amrex::MultiFab> const& Ffield,
            int lev, amrex::Real dt );

        template< typename T_Algo >
        void EvolveFCartesian (
            std::unique_ptr<amrex::MultiFab>& Ffield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
            std::unique_ptr<amrex::MultiFab> const& rhofield,
            int rhocomp,
            amrex::Real dt );

        template< typename T_Algo >
        void EvolveGCartesian (
            std::unique_ptr<amrex::MultiFab>& Gfield,
            std::array<std::unique_ptr<amrex::MultiFab>,3> const& Bfield,
            amrex::Real dt);

        void EvolveRhoCartesianECT (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Efield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& face_areas,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& ECTRhofield, int lev);

        void EvolveBCartesianECT (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& face_areas,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& area_mod,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& ECTRhofield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Venl,
            std::array< std::unique_ptr<amrex::iMultiFab>, 3 >& flag_info_cell,
            std::array< std::unique_ptr<amrex::LayoutData<FaceInfoBox> >, 3 >& borrowing,
            int lev, amrex::Real dt
        );

        template< typename T_Algo >
        void ComputeDivECartesian (
            const std::array<std::unique_ptr<amrex::MultiFab>,3>& Efield,
            amrex::MultiFab& divE );

        template< typename T_Algo, typename T_MacroAlgo >
        void MacroscopicEvolveECartesian (
            std::array< std::unique_ptr< amrex::MultiFab>, 3>& Efield,
            std::array< std::unique_ptr< amrex::MultiFab>, 3> const& Bfield,
            std::array< std::unique_ptr< amrex::MultiFab>, 3> const& Jfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            amrex::Real dt,
            std::unique_ptr<MacroscopicProperties> const& macroscopic_properties);

        template< typename T_Algo >
        void EvolveBPMLCartesian (
            std::array< amrex::MultiFab*, 3 > Bfield,
            std::array< amrex::MultiFab*, 3 > Efield,
            amrex::Real dt,
            bool dive_cleaning);

        template< typename T_Algo >
        void EvolveEPMLCartesian (
            std::array< amrex::MultiFab*, 3 > Efield,
            std::array< amrex::MultiFab*, 3 > Bfield,
            std::array< amrex::MultiFab*, 3 > Jfield,
            std::array< amrex::MultiFab*, 3 > edge_lengths,
            amrex::MultiFab* Ffield,
            MultiSigmaBox const& sigba,
            amrex::Real dt, bool pml_has_particles );

        template< typename T_Algo >
        void EvolveFPMLCartesian ( amrex::MultiFab* Ffield,
                      std::array< amrex::MultiFab*, 3 > Efield,
                      amrex::Real dt );

        template<typename T_Algo>
        void HybridPICSolveECartesian (
            std::array< std::unique_ptr<amrex::MultiFab>, 3>& Efield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Jfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Jifield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Jextfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3> const& Bfield,
            std::unique_ptr<amrex::MultiFab> const& rhofield,
            std::unique_ptr<amrex::MultiFab> const& Pefield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            int lev, HybridPICModel const* hybrid_model,
            bool include_resistivity_term );

        template<typename T_Algo>
        void CalculateCurrentAmpereCartesian (
            std::array< std::unique_ptr<amrex::MultiFab>, 3 >& Jfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& Bfield,
            std::array< std::unique_ptr<amrex::MultiFab>, 3 > const& edge_lengths,
            int lev
        );
#endif

};

#endif // WARPX_FINITE_DIFFERENCE_SOLVER_H_
