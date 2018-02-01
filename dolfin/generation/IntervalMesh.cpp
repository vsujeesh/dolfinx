// Copyright (C) 2007 Kristian B. Oelgaard
//
// This file is part of DOLFIN (https://www.fenicsproject.org)
//
// SPDX-License-Identifier:    LGPL-3.0-or-later

#include "IntervalMesh.h"
#include "dolfin/common/MPI.h"
#include "dolfin/common/constants.h"
#include "dolfin/mesh/CellType.h"
#include "dolfin/mesh/MeshPartitioning.h"

#include <Eigen/Dense>
#include <cmath>

using namespace dolfin;

//-----------------------------------------------------------------------------
Mesh IntervalMesh::build(MPI_Comm comm, std::size_t nx, std::array<double, 2> x)
{
  // Receive mesh according to parallel policy
  if (MPI::rank(comm) != 0)
  {
    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
                     geom(0, 1);
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
                     topo(0, 2);
    Mesh mesh(comm, CellType::Type::interval, geom, topo);
    MeshPartitioning::build_distributed_mesh(mesh);
    return mesh;
  }

  const double a = x[0];
  const double b = x[1];
  const double ab = (b - a) / static_cast<double>(nx);

  if (std::abs(a - b) < DOLFIN_EPS)
  {
    dolfin_error(
        "Interval.cpp", "create interval",
        "Length of interval is zero. Consider checking your dimensions");
  }

  if (b < a)
  {
    dolfin_error("Interval.cpp", "create interval",
                 "Length of interval is negative. Consider checking the order "
                 "of your arguments");
  }

  if (nx < 1)
  {
    dolfin_error("Interval.cpp", "create interval",
                 "Number of points on interval is (%d), it must be at least 1",
                 nx);
  }

  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> geom((nx + 1), 1);
  Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> topo(nx, 2);

  // Create vertices
  for (std::size_t ix = 0; ix <= nx; ix++)
    geom(ix, 0) = a + ab * static_cast<double>(ix);

  // Create intervals
  for (std::size_t ix = 0; ix < nx; ix++)
    topo.row(ix) << ix, ix + 1;

  Mesh mesh(comm, CellType::Type::interval, geom, topo);
  MeshPartitioning::build_distributed_mesh(mesh);
  return mesh;
}
//-----------------------------------------------------------------------------
