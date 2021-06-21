#pragma once

#include "fields/field.hpp"
#include "mesh/mesh.hpp"
#include "mms/manufactured_solutions.hpp"
#include "operators/laplacian.hpp"
#include "temporal/step_controller.hpp"
#include <sol/forward.hpp>

namespace ccs::systems
{
//
// solve dT/dt = k lap T
//
class heat
{

    mesh m;
    bcs::Grid grid_bcs;
    bcs::Object object_bcs;
    manufactured_solution m_sol;

    laplacian lap;
    real diffusivity;

    scalar_real neumann_u;

    std::vector<std::string> io_names = {"U"};

public:
    heat() = default;

    heat(mesh&& m,
         bcs::Grid&& grid_bcs,
         bcs::Object&& object_bcs,
         manufactured_solution&& m_sol,
         stencil st,
         real diffusivity);

    static std::optional<heat> from_lua(const sol::table&);

    void operator()(field&, const step_controller&);

    system_stats stats(const field& u0, const field& u1, const step_controller&) const;

    bool valid(const system_stats&) const;

    real timestep_size(const field&, const step_controller&) const;

    void rhs(field_view, real, field_span) const;

    void update_boundary(field_span, real time);

    void log(const system_stats&, const step_controller&);

    std::span<const std::string> names() const;

    real3 summary(const system_stats&) const;

    system_size size() const;
};
} // namespace ccs::systems
