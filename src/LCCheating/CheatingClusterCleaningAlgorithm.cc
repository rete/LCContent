/**
 *  @file   LCContent/src/LCCheating/CheatingClusterCleaningAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating cluster cleaning algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "LCCheating/CheatingClusterCleaningAlgorithm.h"

using namespace pandora;

namespace lc_content
{

StatusCode CheatingClusterCleaningAlgorithm::Run()
{
    const ClusterList *pClusterList = NULL;

    if (!m_inputClusterListName.empty())
    {
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_inputClusterListName, pClusterList));
    }
    else
    {
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
    }

    if (NULL == pClusterList)
        return STATUS_CODE_NOT_INITIALIZED;

    // Examine clusters, reducing each to just the hits corresponding to the main mc particle
    for (ClusterList::const_iterator itCluster = pClusterList->begin(), itClusterEnd = pClusterList->end(); itCluster != itClusterEnd; ++itCluster )
    {
        try
        {
            Cluster *pCluster = *itCluster;
            const MCParticle *const pMainMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));

            // Remove all calo hits that do not correspond to the cluster main mc particle
            OrderedCaloHitList orderedCaloHitList(pCluster->GetOrderedCaloHitList());

            for (OrderedCaloHitList::const_iterator itLyr = orderedCaloHitList.begin(), itLyrEnd = orderedCaloHitList.end(); itLyr != itLyrEnd; ++itLyr)
            {
                for (CaloHitList::const_iterator hitIter = itLyr->second->begin(), hitIterEnd = itLyr->second->end(); hitIter != hitIterEnd; ++hitIter)
                {
                    try
                    {
                        CaloHit *pCaloHit = *hitIter;
                        const MCParticle *pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));

                        if (pMainMCParticle != pMCParticle)
                        {
                            PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveFromCluster(*this, pCluster, pCaloHit));
                        }
                    }
                    catch (StatusCodeException &)
                    {
                    }
                }
            }

            // Repeat for isolated hits
            CaloHitList isolatedCaloHitList(pCluster->GetIsolatedCaloHitList());

            for (CaloHitList::const_iterator hitIter = isolatedCaloHitList.begin(), hitIterEnd = isolatedCaloHitList.end(); hitIter != hitIterEnd; ++hitIter)
            {
                try
                {
                    CaloHit *pCaloHit = *hitIter;
                    const MCParticle *pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));

                    if (pMainMCParticle != pMCParticle)
                    {
                        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveIsolatedFromCluster(*this, pCluster, pCaloHit));
                    }
                }
                catch (StatusCodeException &)
                {
                }
            }
        }
        catch (StatusCodeException &)
        {
        }
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode CheatingClusterCleaningAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle,
        "InputClusterListName", m_inputClusterListName));

    return STATUS_CODE_SUCCESS;
}

} // namespace lc_content