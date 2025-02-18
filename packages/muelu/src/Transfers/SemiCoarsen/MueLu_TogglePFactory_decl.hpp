// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_TOGGLEPFACTORY_DECL_HPP
#define MUELU_TOGGLEPFACTORY_DECL_HPP

#include "MueLu_ConfigDefs.hpp"
#include "MueLu_TogglePFactory_fwd.hpp"

#include "MueLu_Level_fwd.hpp"
#include "MueLu_ParameterListAcceptor.hpp"
#include "MueLu_PFactory.hpp"

namespace MueLu {

  /*!
    @class TogglePFactory
    @ingroup MueLuTransferClasses
    @brief Prolongator factory which allows switching between two different prolongator strategies.

    ## Input/output of TogglePFactory ##

    ### User parameters of TogglePFactory ###
    Parameter | type | default | master.xml | validated | requested | description
    ----------|------|---------|:----------:|:---------:|:---------:|------------
    | P         | Factory | null  |   | * | * | Generating factories of prolongators. Note that the TogglePFactory needs at least two different prolongator subfactories.
    | Ptent     | Factory | null  |   | * | * | Generating factories of base prolongators. Note that the TogglePFactory needs at least two different base prolongator subfactories. Usually, the factory for Ptent is the same as for the coarse null space.
    | Nullspace | Factory | null  |   | * | * | Generating factories for fine level null space information. Note, that you have to provide a null space information source factory for each prolongator factory.
    | NumZLayers| Factory | NoFactory | | * | * | Factory which provides information about z layers. Usually it is stored with MueLu::NoFactory as generating factory. The data is generated by the SemiCoarsenPFactory.
    | semicoarsen: number of levels | int | ? | * | * |  | number of levels which are reserved for semi-coarsening. Note that the TogglePFactory stops the semi-coarsening process either when we have more levels than declared in this parameter or as soon as the number of remaining z-levels is 1.

    The * in the @c master.xml column denotes that the parameter is defined in the @c master.xml file.<br>
    The * in the @c validated column means that the parameter is declared in the list of valid input parameters (see TogglePFactory::GetValidParameters).<br>
    The * in the @c requested column states that the data is requested as input with all dependencies (see TogglePFactory::DeclareInput).

    ### Variables provided by TogglePFactory ###

    After TogglePFactory::Build the following data is available (if requested)

    Parameter | generated by | description
    ----------|--------------|------------
    | P       | TogglePFactory | Prolongator
    | Ptent   | TogglePFactory | Base prolongator (e.g. tentative prolongator)
    | Nullspace | TogglePFactory | associated coarse null space

  */

template <class Scalar = DefaultScalar,
        class LocalOrdinal = DefaultLocalOrdinal,
        class GlobalOrdinal = DefaultGlobalOrdinal,
        class Node = DefaultNode>
  class TogglePFactory : public PFactory {
#undef MUELU_TOGGLEPFACTORY_SHORT
#include "MueLu_UseShortNames.hpp"

  public:

    //! @name Constructors/Destructors.
    //@{

    /*! @brief Constructor. */
    TogglePFactory() : hasDeclaredInput_(false) { }

    //! Destructor.
    virtual ~TogglePFactory() { }

    RCP<const ParameterList> GetValidParameterList() const;

    //@}

    //! Input
    //@{

    void DeclareInput(Level &fineLevel, Level &coarseLevel) const;

    //@}

    //! @name Build methods.
    //@{

    /*!  @brief Build method.   */
    void Build(Level& fineLevel, Level &coarseLevel) const;

    void BuildP(Level &/* fineLevel */, Level &/* coarseLevel */) const { /* empty */ };
    //@}

    //@{

    /*! @brief Add a prolongator factory in the end of list of prolongator factories.    */
    void AddProlongatorFactory(const RCP<const FactoryBase>& factory);

    //! Returns number of prolongator factories.
    size_t NumProlongatorFactories() const { return prolongatorFacts_.size(); }

    /*! @brief Add a tentative prolongator factory in the end of list of prolongator factories.    */
    void AddPtentFactory(const RCP<const FactoryBase>& factory);

    //! Returns number of tentative prolongator factories.
    size_t NumPtentFactories() const { return ptentFacts_.size(); }

    /*! @brief Add a coarse nullspace factory in the end of list of coarse nullspace factories.    */
    void AddCoarseNullspaceFactory(const RCP<const FactoryBase>& factory);

    //! Returns number of coarse null space factories.
    size_t NumCoarseNullspaceFactories() const { return prolongatorFacts_.size(); }

    RCP<const FactoryBase>  getProlongatorFactory(size_t t) const {return prolongatorFacts_[t]; }
    //@}
  private:
    //! list of user-defined prolongation operator factories
    mutable std::vector<RCP<const FactoryBase> > prolongatorFacts_;

    //! list of user-defined tentative prolongation operator factories
    mutable std::vector<RCP<const FactoryBase> > ptentFacts_;

    //! list of user-defined nullspace factories (i.e. the prolongator factories which also generate the coarse level nullspace)
    mutable std::vector<RCP<const FactoryBase> > nspFacts_;

    mutable bool hasDeclaredInput_;
  }; //class TogglePFactory

} //namespace MueLu

#define MUELU_TOGGLEPFACTORY_SHORT
#endif // MUELU_TOGGLEPFACTORY_DECL_HPP
