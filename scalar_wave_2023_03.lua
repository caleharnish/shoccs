simulation = {
    mesh = {
        index_extents = {51, 51},
        domain_bounds = {2, 2}
    },
    domain_boundaries = {
        xmin = "dirichlet",
        ymin = "dirichlet"
    },
    shapes = {
       {
          type = "sphere",
          center = {0, 0},
          radius = math.pi / 10,
          boundary_condition = "dirichlet"
       },
       {
          type = "sphere",
          center = {2, 2},
          radius = math.pi / 10,
          boundary_condition = "floating"
       }
    },
    scheme = {
        order = 1,
        type = "E2-poly",
        floating_alpha = {
           -0.1363307024687572,
           -0.5543209998976423,
           -0.8703883271474994,
           0.8287804722613139,
           0.9930089463057048,
           -0.04468831634701048
        },
        dirichlet_alpha = {
           -0.7807082540430073, -0.242052654199171, 0.18210840187280186
        },
        interpolant_alpha = {
           -0.8029965159780618,
           -0.7068873718375119,
           -0.30143529863628166,
           -0.4007686091091105
        }
    },
    system = {
       type = "scalar wave",
       center = {-1, -1},
       radius = 0,
       max_error = 5.0
    },
    integrator = {
        type = "rk4"
    },
    step_controller = {
        max_time = 500,
        cfl = {
            hyperbolic = 0.8,
            parabolic = 0.2
        }
    },
    io = {
        dir = "io",
        -- write_every_step = 1
        write_every_time = 5
    }
}
