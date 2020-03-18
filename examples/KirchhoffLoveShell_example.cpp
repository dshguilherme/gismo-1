/** @file KirchhoffLoveShell_example.cpp

    @brief A Kirchhoff-Love example.

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Farahat
*/

# include <gismo.h>
# include <gsAssembler/gsKirchhoffLoveShellAssembler.h>
# include <gsG1Basis/gsG1AuxiliaryEdgeMultiplePatches.h>
# include <gsG1Basis/gsG1AuxiliaryVertexMultiplePatches.h>
# include <gsG1Basis/gsG1Mapper.h>


using namespace gismo;

int main(int argc, char *argv[])
{
    index_t numRefine = 5;
    index_t numDegree = 1;
    bool plot = true;

    gsCmdLine cmd("Example for solving the Kirchhoff-Love problem.");
    cmd.addInt("r", "refine", "Number of refinement steps", numRefine);
    cmd.addInt("p", "degree", "Polynomial degree", numDegree);
    cmd.addSwitch( "plot", "Plot result in ParaView format", plot );
    try { cmd.getValues(argc,argv); } catch (int rv) { return rv; }

    dirichlet::strategy dirStrategy = dirichlet::elimination;
    iFace::strategy intStrategy = iFace::glue;

    gsFunctionExpr<> source  ("256*pi*pi*pi*pi*(4*cos(4*pi*x)*cos(4*pi*y) - cos(4*pi*x) - cos(4*pi*y))",2);
    gsFunctionExpr<> laplace ("-16*pi*pi*(2*cos(4*pi*x)*cos(4*pi*y) - cos(4*pi*x) - cos(4*pi*y))",2);
    gsFunctionExpr<> solVal("(cos(4*pi*x) - 1) * (cos(4*pi*y) - 1)",2);
    gsFunctionExpr<>sol1der ("-4*pi*(cos(4*pi*y) - 1)*sin(4*pi*x)",
                             "-4*pi*(cos(4*pi*x) - 1)*sin(4*pi*y)",2);
    gsFunctionExpr<>sol2der ("-16*pi^2*(cos(4*pi*y) - 1)*cos(4*pi*x)",
                             "-16*pi^2*(cos(4*pi*x) - 1)*cos(4*pi*y)",
                             " 16*pi^2*sin(4*pi*x)*sin(4*pi*y)", 2);
    gsFunctionWithDerivatives<real_t> solution(solVal, sol1der, sol2der);

    gsFileData<> fileSrc("KirchhoffLoveGeo/geo_fivePatchDiffParam.xml");
//    gsFileData<> fileSrc("KirchhoffLoveGeo/square_diffParam.xml");

    gsInfo << "Loaded file " << fileSrc.lastPath() << "\n";

    gsMultiPatch<> geo;
    gsInfo << "Geometry taken correctly \n";
    fileSrc.getId(5, geo);
    geo.computeTopology();
    gsInfo << "Geometry computed correctly\n";


    gsOptionList optionList;
    optionList.addInt("p_tilde","Grad",1);
    optionList.addInt("r_tilde","Reg",0);
    optionList.addInt("regularity","Regularity of the initial geometry",1);
    optionList.addInt("refine","Plot in Paraview",numRefine);
    optionList.addInt("degree","Degree",numDegree);
    optionList.addSwitch("local","Local projection for gluing data",false);
    optionList.addSwitch("direct","Local projection for gluing data",false);
    optionList.addSwitch("plot","Plot in Paraview",false);

    geo.degreeElevate(optionList.getInt("degree"));
    geo.uniformRefine_withSameRegularity(optionList.getInt("refine"),optionList.getInt("regularity"));
    gsMultiBasis<> basis(geo);
    gsInfo << "Old: " << basis << "\n";


    size_t degU = basis.basis(0).component(0).maxDegree();
    size_t degV = basis.basis(0).component(1).maxDegree();

    gsTensorBSplineBasis<2, real_t> & temp_L = dynamic_cast<gsTensorBSplineBasis<2, real_t> &>(basis.basis(0));
    gsBSplineBasis<> temp_basisLU = dynamic_cast<gsBSplineBasis<> &>(temp_L.component(0));
    gsBSplineBasis<> temp_basisLV = dynamic_cast<gsBSplineBasis<> &>(temp_L.component(1));

    gsInfo << "maxDeg patch 0 along u: " << degU << "\n";
    gsInfo << "maxDeg patch 0 along v: " << degV << "\n";

    for(size_t i = 0; i < basis.nBases();i++)
    {
        gsInfo << "Basis patch "<< i << ": " << basis.basis(i).size() << "\n" ;
        gsTensorBSplineBasis<2, real_t> & temp = dynamic_cast<gsTensorBSplineBasis<2, real_t> &>(basis.basis(i));
        gsInfo << temp.size(0) << " " << temp.size(1) << "\n";
    }


//    gsInfo << "Basis number:" << basis.nBases() << "\n";
//    gsInfo << "Coefs number:" << geo.patch(0).coefs().size() /2 << "\n";
//    gsInfo << "dimU * dimV number:" << temp_L.size(0) * temp_L.size(1) << "\n";

//    gsInfo << "NumElem patch 0 along u: " << elemU << "\n";
//    gsInfo << "NumElem patch 0 along v: " << elemV << "\n";





    // ########### EDGE FUNCTIONS ###########
    // Interface loop
//    for (index_t numInt = 0; numInt < geo.interfaces().size(); numInt++ )
//    {
//        const boundaryInterface & item = geo.interfaces()[numInt];
//
//        std::string fileName;
//        std::string basename = "InterfaceBasisFunctions" + util::to_string(numInt);
//        gsParaviewCollection collection(basename);
//
//        /*
//        gsG1AuxiliaryEdgeMultiplePatches first(multiPatch, item.first().patch);
//        first.computeG1EdgeBasis(optionList,item.first().m_index,false);
//
//        gsG1AuxiliaryEdgeMultiplePatches second(multiPatch, item.second().patch);
//        second.computeG1EdgeBasis(optionList,item.second().m_index,false);
//        */
//        gsG1AuxiliaryEdgeMultiplePatches singleInt(geo, item.first().patch, item.second().patch);
//        singleInt.computeG1InterfaceBasis(optionList);
//
//        for (index_t i = 0; i < singleInt.getSinglePatch(0).getG1Basis().nPatches(); i++)
//        {
//            gsMultiPatch<> edgeSingleBF;
//
//            edgeSingleBF.addPatch(singleInt.getSinglePatch(0).getG1Basis().patch(i));
//            edgeSingleBF.addPatch(singleInt.getSinglePatch(1).getG1Basis().patch(i));
//
//            fileName = basename + "_0_" + util::to_string(i);
//            gsField<> temp_field(geo.patch(item.first().patch),edgeSingleBF.patch(0));
//            gsWriteParaview(temp_field,fileName,5000);
//            collection.addTimestep(fileName,i,"0.vts");
//            fileName = basename + "_1_" + util::to_string(i);
//            gsField<> temp_field_1(geo.patch(item.second().patch),edgeSingleBF.patch(1));
//            gsWriteParaview(temp_field_1,fileName,5000);
//            collection.addTimestep(fileName,i,"0.vts");
//        }
//        collection.save();
//    }




//     Vertices loop

    std::vector<std::vector<patchCorner>> allcornerLists = geo.vertices();
    //for(size_t i=0; i < allcornerLists.size(); i++)
    for(size_t i=0; i < 1; i++)
    {
        std::vector<size_t> patchIndex;
        std::vector<size_t> vertIndex;
        for(size_t j = 0; j < allcornerLists[i].size(); j++)
        {
            //patchIndex.push_back(allcornerLists[i][j].patch);
            //vertIndex.push_back(allcornerLists[i][j].m_index);
            gsInfo << "Patch: " << allcornerLists[i][j].patch << "\t Index: " << allcornerLists[i][j].m_index << "\n";

        }
        gsInfo << "\n";
        patchIndex.push_back(0);
        patchIndex.push_back(1);
        patchIndex.push_back(2);
        patchIndex.push_back(3);
        patchIndex.push_back(4);

        vertIndex.push_back(4);
        vertIndex.push_back(2);
        vertIndex.push_back(1);
        vertIndex.push_back(3);
        vertIndex.push_back(2);

        gsMultiPatch<> onebasisfunction;


        gsG1AuxiliaryVertexMultiplePatches a(geo, patchIndex, vertIndex);
        a.computeG1InternalVertexBasis(optionList);
//        index_t kindBdr = a.kindOfVertex();
        for (size_t j = 0; j < vertIndex.size(); j++)
        {
            onebasisfunction.addPatch(a.getSinglePatch(j).getG1Basis().patch(0));
        }

        gsWriteParaview(onebasisfunction,"geo",5000);


    }


//gsG1Mapper a(geo);
//    a.printInterfaceEdgeMapper();
//    a.printReducedEdgeMapper();
//    a.printReducedBoundaryEdgeMapper();





//    gsWriteParaview(newgeom1, "Geometry", 1000);

//    // Write file .xml of the new geometry
//    gsFileData<> fd;
//    fd << test;
//    // output is a string. The extention .xml is added automatically
//    fd.save("newGeo");


    //Setting up oundary conditions
    gsBoundaryConditions<> bcInfo;
    gsBoundaryConditions<> bcInfo2;
    for (gsMultiPatch<>::const_biterator
             bit = geo.bBegin(); bit != geo.bEnd(); ++bit)
    {
        bcInfo.addCondition( *bit, condition_type::dirichlet, &solution );
        bcInfo2.addCondition( *bit,  condition_type::neumann, &laplace);
    }


    //Initilize solver
    gsKirchhoffLoveShellAssembler<real_t> KirchhoffLoveShellAssembler( geo,basis,bcInfo,bcInfo2,source,
                                                       dirStrategy, intStrategy);

    gsInfo<<"Assembling..." << "\n";
    KirchhoffLoveShellAssembler.assemble();

    gsInfo<<"Solving with direct solver, "<< KirchhoffLoveShellAssembler.numDofs()<< " DoFs..."<< "\n";
    gsSparseSolver<real_t>::LU solver;
    solver.analyzePattern(KirchhoffLoveShellAssembler.matrix() );
    solver.factorize(KirchhoffLoveShellAssembler.matrix());
    gsMatrix<> solVector= solver.solve(KirchhoffLoveShellAssembler.rhs());

    //Reconstruct solution
    gsMultiPatch<> mpsol;
    KirchhoffLoveShellAssembler.constructSolution(solVector, mpsol);
    gsField<> solField(KirchhoffLoveShellAssembler.patches(), mpsol);

    //Contruct the H2 norm, part by part.
    real_t errorH2Semi = solField.distanceH2(solution, false);
    real_t errorH1Semi = solField.distanceH1(solution, false);
    real_t errorL2 = solField.distanceL2(solution, false);
    real_t errorH1 = math::sqrt(errorH1Semi*errorH1Semi + errorL2*errorL2);
    real_t errorH2 = math::sqrt(errorH2Semi*errorH2Semi + errorH1Semi*errorH1Semi + errorL2*errorL2);

    gsInfo << "The L2 error of the solution is : " << errorL2 << "\n";
    gsInfo << "The H1 error of the solution is : " << errorH1 << "\n";
    gsInfo << "The H2 error of the solution is : " << errorH2 << "\n";

    // Plot solution in paraview
    if (plot)
    {
        // Write approximate and exact solution to paraview files
        gsInfo<<"Plotting in ParaView...\n";
        gsWriteParaview<>(solField, "KirchhoffLoveShell2d", 5000);
        const gsField<> exact( geo, solution, false );
        gsWriteParaview<>( exact, "KirchhoffLoveShell2d_exact", 5000);
    }
    else
        gsInfo << "Done. No output created, re-run with --plot to get a ParaView "
                  "file containing the solution.\n";

    return  0;
}