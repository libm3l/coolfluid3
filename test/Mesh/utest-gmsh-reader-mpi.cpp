// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for CF::Mesh::Gmsh::CReader"

#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp>

#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

#include "Mesh/CDynTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Mesh;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

struct GmshReaderMPITests_Fixture
{
  /// common setup for each test case
  GmshReaderMPITests_Fixture()
  {
		m_argc = boost::unit_test::framework::master_test_suite().argc;
		m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~GmshReaderMPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
	int    m_argc;
	char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( GmshReaderMPITests_TestSuite, GmshReaderMPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
	//PE::instance().init(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");
	BOOST_CHECK_EQUAL( meshreader->name() , "meshreader" );
	BOOST_CHECK_EQUAL( meshreader->get_format() , "Gmsh" );
	std::vector<std::string> extensions = meshreader->get_extensions();
	BOOST_CHECK_EQUAL( extensions[0] , ".msh" );
	
//	meshreader->configure_property("Repartition",true);
//	meshreader->configure_property("OutputRank",(Uint) 0);
//	meshreader->configure_property("Unified Zones",false);
	
  // the file to read from
  boost::filesystem::path fp_in ("rectangle-tg-p1.msh");
	
  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );
  
  // CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    create_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
boost::filesystem::path file ("rectangle-tg-p1-out.msh");
mesh_writer->write_from_to(mesh,file);
	
  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

} 

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_triag_p2 )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("rectangle-tg-p2.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    create_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
boost::filesystem::path file ("rectangle-tg-p2-out.msh");
mesh_writer->write_from_to(mesh,file);

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p1 )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("rectangle-qd-p2.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    create_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
boost::filesystem::path file ("rectangle-qd-p1-out.msh");
mesh_writer->write_from_to(mesh,file);

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_quad_p2 )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("rectangle-qd-p2.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    create_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
boost::filesystem::path file ("rectangle-qd-p2-out.msh");
mesh_writer->write_from_to(mesh,file);

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p1 )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("rectangle-mix-p1.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    create_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
boost::filesystem::path file ("rectangle-mix-p1-out.msh");
mesh_writer->write_from_to(mesh,file);

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( read_2d_mesh_mix_p2 )
{

  CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Gmsh.CReader","meshreader");

  // the file to read from
  boost::filesystem::path fp_in ("rectangle-mix-p2.msh");

  // the mesh to store in
  CMesh::Ptr mesh ( allocate_component<CMesh>  ( "mesh" ) );

  // CFinfo.setFilterRankZero(false);
  meshreader->read_from_to(fp_in,mesh);
  // CFinfo.setFilterRankZero(true);

  // CFinfo << mesh->tree() << CFendl;

CMeshWriter::Ptr mesh_writer =
    create_component_abstract_type<CMeshWriter> ("CF.Mesh.Gmsh.CWriter", "GmshWriter" );
boost::filesystem::path file ("rectangle-mix-p2-out.msh");
mesh_writer->write_from_to(mesh,file);

  CFinfo << "elements count = " << find_component<CRegion>(*mesh).recursive_elements_count() << CFendl;
  CFinfo << "nodes count    = " << find_component<CRegion>(*mesh).recursive_nodes_count() << CFendl;

}

////////////////////////////////////////////////////////////////////////////////


BOOST_AUTO_TEST_CASE( finalize_mpi )
{
	//PE::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

