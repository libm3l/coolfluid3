// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"

#include "sdm/navierstokes/BCSubsonicInlet2D.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace navierstokes {
	
//////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder<BCSubsonicInletUT2D,BC,LibNavierStokes> bcsubsonicinletUT2d_builder;
common::ComponentBuilder<BCSubsonicInletTtPtAlpha2D,BC,LibNavierStokes> bcsubsonicinletTtPtAlpha2d_builder;

/////////////////////////////////////////////////////////////////////////////

} // navierstokes
} // sdm
} // cf3
