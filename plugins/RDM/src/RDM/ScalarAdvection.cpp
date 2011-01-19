// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CDomain.hpp"

#include "Solver/CModelSteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CDiscretization.hpp"


#include "RDM/ScalarAdvection.hpp"

namespace CF {
namespace RDM {

using namespace CF::Common;
using namespace CF::Common::String;
using namespace CF::Mesh;
using namespace CF::Solver;

Common::ComponentBuilder < ScalarAdvection, Component, LibRDM > ScalarAdvection_Builder;

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::ScalarAdvection ( const std::string& name  ) :
  Component ( name )
{
  // signals

  this->regist_signal ( "create_model" , "Creates a scalar advection model", "Create Model" )->connect ( boost::bind ( &ScalarAdvection::create_model, this, _1 ) );
  signal("create_model").signature
      .insert<std::string>("Model name", "Name for created model" );

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;
}

////////////////////////////////////////////////////////////////////////////////

ScalarAdvection::~ScalarAdvection()
{
}

////////////////////////////////////////////////////////////////////////////////

void ScalarAdvection::create_model ( Common::XmlNode& node )
{
  XmlParams p ( node );

// create the model

  std::string name  = p.get_option<std::string>("Model name");

  CModel::Ptr model = Core::instance().root()->create_component<CModelSteady>( name );

  // create the CDomain
  // CDomain::Ptr domain =
      model->create_component<CDomain>("Domain");

  // create the Physical Model
  CPhysicalModel::Ptr pm = model->create_component<CPhysicalModel>("Physics");
  pm->mark_basic();

  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 2u );

  // setup iterative solver
  CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("CF.RDM.RungeKutta", "IterativeSolver");
  solver->mark_basic();
  model->add_component( solver );
  solver->configure_property("Domain" , URI("cpath:../Domain"));

  // setup discretization method
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("CF.RDM.ResidualDistribution", "Discretization");
  cdm->mark_basic();
  solver->add_component( cdm );

//  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.Neu.CReader", "NeutralReader" );
//  CMeshReader::Ptr mesh_reader = create_component_abstract_type<CMeshReader>( "CF.Mesh.CGNS.CReader", "CGNSReader" );
//  mesh_reader->mark_basic();
//  model->add_component( mesh_reader );

}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
