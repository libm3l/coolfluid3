// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/CGroup.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CDomain.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/Gmsh/CWriter.hpp"
#include "Mesh/Gmsh/CReader.hpp"
#include "Mesh/Actions/CBuildFaces.hpp"
#include "Mesh/Actions/CBuildVolume.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CRegion.hpp"

#include "Solver/CModelUnsteady.hpp"
#include "Solver/CPhysicalModel.hpp"
#include "Solver/CIterativeSolver.hpp"
#include "Solver/CDiscretization.hpp"
#include "Solver/Actions/CForAllElements2.hpp"
#include "Solver/Actions/CForAllFaces.hpp"
#include "Solver/Actions/CLoop.hpp"

#include "Tools/MeshGeneration/MeshGeneration.hpp"

#include "FVM/FiniteVolume.hpp"
#include "FVM/ShockTube.hpp"

namespace CF {
namespace FVM {

using namespace boost::assign;
using namespace CF::Common;
using namespace CF::Common::String;
using namespace CF::Mesh;
using namespace CF::Mesh::Actions;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

Common::ComponentBuilder < ShockTube, Component, LibFVM > ShockTube_Builder;

////////////////////////////////////////////////////////////////////////////////

ShockTube::ShockTube ( const std::string& name  ) :
  Component ( name )
{
  std::string brief;
  std::string description;
  brief       += "This wizard creates and sets up a finite volume 1D shocktube problem.\n";
  description += "  1) Run signal \"Create Model\" to create the shocktube model\n";
  description += "  2) Load a 1D mesh in the domain of the shocktube model";
  description += "  3) Run signal \"Setup Model\" to configure and allocate datastorage\n";
  description += "  4) Configure time step and end time in model/Time\n";
  description += "  5) Run signal \"Simulate\" in the shocktube model\n";
  m_properties["brief"] = brief;
  m_properties["description"] = description;

  // signals

  signal("create_component").is_hidden = true;
  signal("rename_component").is_hidden = true;
  signal("delete_component").is_hidden = true;
  signal("move_component").is_hidden   = true;

  regist_signal ( "create_model" , "Creates a shocktube model", "Create Model" )->connect ( boost::bind ( &ShockTube::signal_create_model, this, _1 ) );
  signal("create_model").signature->connect( boost::bind( &ShockTube::signature_create_model, this, _1));

  regist_signal ( "setup_model" , "Setup the shocktube model after mesh has been loaded", "Setup Model" )->connect ( boost::bind ( &ShockTube::signal_setup_model, this, _1 ) );
  signal("setup_model").signature->connect( boost::bind( &ShockTube::signature_setup_model, this, _1));

}

////////////////////////////////////////////////////////////////////////////////

ShockTube::~ShockTube()
{
}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signal_create_model ( Common::XmlNode& node )
{
  XmlParams p ( node );

// create the model

  std::string name  = p.get_option<std::string>("Model name");

  CModel::Ptr model = Core::instance().root()->create_component<CModelUnsteady>( name );

  // create the Physical Model
  CPhysicalModel::Ptr pm = model->create_component<CPhysicalModel>("Physics");
  pm->mark_basic();

  pm->configure_property( "DOFs", 1u );
  pm->configure_property( "Dimensions", 1u );

  // setup iterative solver
  CIterativeSolver::Ptr solver = create_component_abstract_type<CIterativeSolver>("CF.FVM.ForwardEuler", "IterativeSolver");
  solver->mark_basic();
  model->add_component( solver );

  // setup discretization method
  CDiscretization::Ptr cdm = create_component_abstract_type<CDiscretization>("CF.FVM.FiniteVolume", "Discretization");
  cdm->mark_basic();
  solver->add_component( cdm );
  
  CGroup& tools = *model->create_component<CGroup>("tools");
  tools.mark_basic();
  CBuildFaces& build_faces = *tools.create_component<CBuildFaces>("build_faces");
  CBuildVolume& build_volume = *tools.create_component<CBuildVolume>("build_volume");

  Gmsh::CReader& gmsh_reader = *tools.create_component<Gmsh::CReader>("gmsh_reader");
  Gmsh::CWriter& gmsh_writer = *tools.create_component<Gmsh::CWriter>("gmsh_writer");

}

////////////////////////////////////////////////////////////////////////////////

void ShockTube::signature_create_model( XmlNode& node )
{
  XmlParams p(node);

  p.add_option<std::string>("Model name", std::string(), "Name for created model" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShockTube::signal_setup_model ( Common::XmlNode& node )
{
  XmlParams p ( node );
  std::string name  = p.get_option<std::string>("Model name");

  CModelUnsteady::Ptr model = Core::instance().root()->get_child<CModelUnsteady>( name );
  if (is_null(model))
    throw ValueNotFound (FromHere(), "invalid model"); 
  
  CIterativeSolver& solver = find_component<CIterativeSolver>(*model);
  FiniteVolume& finite_volume = find_component<FiniteVolume>(solver);
  
  ////////////////////////////////////////////////////////////////////////////////
  // Generate mesh
  ////////////////////////////////////////////////////////////////////////////////
  
  CMesh::Ptr mesh = model->domain().create_component<CMesh>("line");
  Tools::MeshGeneration::create_line(*mesh, 10. , p.get_option<Uint>("Number of Cells"));
  // path file_in("line.msh");
  //   model->look_component<CMeshReader>("cpath:./tools/gmsh_reader")->read_from_to(file_in,mesh);

  model->look_component<CBuildFaces>("cpath:./tools/build_faces")->transform(mesh);
  model->look_component<CBuildVolume>("cpath:./tools/build_volume")->transform(mesh);
  model->configure_option_recursively("volume",find_component_recursively_with_tag<CField2>(model->domain(),"volume").full_path());
    
  ////////////////////////////////////////////////////////////////////////////////
  // Create basic fields
  ////////////////////////////////////////////////////////////////////////////////
  
  CField2& solution = mesh->create_field2("solution","CellBased","rho[1],rhoU[1],rhoE[1]");

  ////////////////////////////////////////////////////////////////////////////////
  // Initial condition
  ////////////////////////////////////////////////////////////////////////////////
  
  RealVector left(3);
  RealVector right(3);
  
  Real g=1.4;
  
  const Real r_L = 4.696;     const Real r_R = 1.408;
  const Real u_L = 0.;        const Real u_R = 0.;
  const Real p_L = 404400;    const Real p_R = 101100;

  left <<  r_L, r_L*u_L, p_L/(g-1.) + 0.5*r_L*u_L*u_L;
  right << r_R, r_R*u_R, p_R/(g-1.) + 0.5*r_R*u_R*u_R;
  
  RealMatrix node_coordinates;
  RealVector centroid(1);
  CFieldView solution_view("solution_view");
  solution_view.set_field(solution);
  boost_foreach(const CCells& cells, find_components_recursively<CCells>(*mesh))
  {
    solution_view.set_elements(cells);
    solution_view.allocate_coordinates(node_coordinates);
    for (Uint e=0; e<cells.size(); ++e)
    {
      solution_view.put_coordinates(node_coordinates,e);
      solution_view.space().shape_function().compute_centroid(node_coordinates,centroid);
      if (centroid[XX] <= 5.)
      {
        for(Uint i=0; i<left.size(); ++i)
          solution_view[e][i] = left[i];
      }
      else
      {
        for(Uint i=0; i<right.size(); ++i)
          solution_view[e][i] = right[i];
      }
    }
  }
  
  
  ////////////////////////////////////////////////////////////////////////////////
  // Boundary conditions
  ////////////////////////////////////////////////////////////////////////////////
  
  CRegion& inlet = find_component_recursively_with_name<CRegion>(solution.topology(),"xneg");
  CAction& inlet_bc = finite_volume.create_bc("inlet",inlet,"CF.FVM.BCDirichlet");
  inlet_bc.configure_property("rho",r_L);
  inlet_bc.configure_property("u",u_L);
  inlet_bc.configure_property("p",p_L);
  
  
  CRegion& outlet = find_component_recursively_with_name<CRegion>(solution.topology(),"xpos");
  CAction& outlet_bc = finite_volume.create_bc("outlet",outlet,"CF.FVM.BCDirichlet");
  outlet_bc.configure_property("rho",r_R);
  outlet_bc.configure_property("u",u_R);
  outlet_bc.configure_property("p",p_R);  
  
  ////////////////////////////////////////////////////////////////////////////////
  // Solver / Discretization configuration
  ////////////////////////////////////////////////////////////////////////////////

  solver.configure_property("Number of Iterations", 1u);
  
  model->time().configure_property("Time Step", p.get_option<Real>("Time Step"));
  model->time().configure_property("End Time", p.get_option<Real>("End Time"));  
  
  solver.configure_property("Solution",solution.full_path());
  model->configure_option_recursively("time_accurate",true);
  model->configure_option_recursively("time",model->time().full_path());
  model->configure_option_recursively("cfl",1.);

  ////////////////////////////////////////////////////////////////////////////////
  // Writer
  ////////////////////////////////////////////////////////////////////////////////
  
  std::vector<URI> fields;
  boost_foreach(const CField2& field, find_components_recursively<CField2>(*mesh))
    fields.push_back(field.full_path());
  model->look_component<Gmsh::CWriter>("cpath:./tools/gmsh_writer")->configure_property("Fields",fields);
  model->look_component<Gmsh::CWriter>("cpath:./tools/gmsh_writer")->configure_property("File",model->name()+".msh");
  model->look_component<Gmsh::CWriter>("cpath:./tools/gmsh_writer")->configure_property("Mesh",mesh->full_path());

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ShockTube::signature_setup_model( XmlNode& node )
{
  XmlParams p(node);

  p.add_option<std::string>("Model name", std::string(), "Name for created model" );
  p.add_option<Uint>("Number of Cells", 100u , "Number of Cells to be generated");
  p.add_option<Real>("End Time", 0.008, "Time to stop simulation");
  p.add_option<Real>("Time Step", 0.0004, "Maximum allowed time step to take");
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF