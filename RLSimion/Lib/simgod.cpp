/*
	SimionZoo: A framework for online model-free Reinforcement Learning on continuous
	control problems

	Copyright (c) 2016 SimionSoft. https://github.com/simionsoft

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "simgod.h"
#include "experiment.h"
#include "../Common/named-var-set.h"
#include "config.h"
#include "simion.h"
#include "app.h"
#include "deferred-load.h"
#include "utils.h"
#include "featuremap.h"
#include "experience-replay.h"
#include "parameters.h"
#include "features.h"
#include <algorithm>

std::vector<std::pair<DeferredLoad*, unsigned int>> SimGod::m_deferredLoadSteps;
CHILD_OBJECT<StateFeatureMap> SimGod::m_pGlobalStateFeatureMap;
CHILD_OBJECT<ActionFeatureMap> SimGod::m_pGlobalActionFeatureMap;

SimGod::SimGod(ConfigNode* pConfigNode)
{
	if (!pConfigNode) return;

	//the global parameterizations of the state/action spaces
	m_pGlobalStateFeatureMap = CHILD_OBJECT<StateFeatureMap>(pConfigNode, "State-Feature-Map", "The state feature map", true);
	m_pGlobalActionFeatureMap = CHILD_OBJECT<ActionFeatureMap>(pConfigNode, "Action-Feature-Map", "The state feature map", true);
	m_pExperienceReplay = CHILD_OBJECT<ExperienceReplay>(pConfigNode, "Experience-Replay", "The experience replay parameters", true);
	m_simions = MULTI_VALUE_FACTORY<Simion>(pConfigNode, "Simion", "Simions: learning agents and controllers");

	//Gamma is global: it is considered a parameter of the problem, not the learning algorithm
	m_gamma = DOUBLE_PARAM(pConfigNode, "Gamma", "Gamma parameter", 0.9);

	m_bFreezeTargetFunctions = BOOL_PARAM(pConfigNode, "Freeze-Target-Function", "Defers updates on the V-functions to improve stability", false);
	m_targetFunctionUpdateFreq = INT_PARAM(pConfigNode, "Target-Function-Update-Freq", "Update frequency at which target functions will be updated. Only used if Freeze-Target-Function=true", 100);
	m_bUseImportanceWeights = BOOL_PARAM(pConfigNode, "Use-Importance-Weights", "Use sample importance weights to allow off-policy learning -experimental-", false);
}


SimGod::~SimGod()
{
}


double SimGod::selectAction(State* s, Action* a)
{
	double probability = 1.0;

	for (unsigned int i = 0; i < m_simions.size(); i++)
		probability*= m_simions[i]->selectAction(s, a);

	return probability;
}

void SimGod::update(State* s, Action* a, State* s_p, double r, double probability)
{
	if (SimionApp::get()->pExperiment->isEvaluationEpisode()) return;

	m_bReplayingExperience = false;

	//update step
	for (unsigned int i = 0; i < m_simions.size(); i++)
		m_simions[i]->update(s, a, s_p, r, probability);

	if (m_pExperienceReplay->bUsing())
		m_pExperienceReplay->addTuple(s, a, s_p, r, probability);
}

void SimGod::postUpdate()
{
	ExperienceTuple* pExperienceTuple;

	//Experience Replay
	if (m_pExperienceReplay->bUsing() && m_pExperienceReplay->bHaveEnoughTuples())
	{
		m_bReplayingExperience = true;

		size_t updateBatchSize = m_pExperienceReplay->getUpdateBatchSize();
		for (size_t tuple = 0; tuple < updateBatchSize; ++tuple)
		{
			pExperienceTuple = m_pExperienceReplay->getRandomTupleFromBuffer();

			//update step
			for (size_t i = 0; i < m_simions.size(); i++)
				m_simions[i]->update(pExperienceTuple->s, pExperienceTuple->a, pExperienceTuple->s_p
					, pExperienceTuple->r, pExperienceTuple->probability);
		}
	}
}

void SimGod::registerDeferredLoadStep(DeferredLoad* deferredLoadObject, unsigned int orderLoad)
{
	m_deferredLoadSteps.push_back(std::pair<DeferredLoad*, unsigned int>(deferredLoadObject, orderLoad));
}

bool myComparison(const std::pair<DeferredLoad*, unsigned int> &a, const std::pair<DeferredLoad*, unsigned int> &b)
{
	return a.second < b.second;
}

void SimGod::deferredLoad()
{
	std::sort(m_deferredLoadSteps.begin(), m_deferredLoadSteps.end(), myComparison);

	for (auto it = m_deferredLoadSteps.begin(); it != m_deferredLoadSteps.end(); it++)
	{
		(*it).first->deferredLoadStep();
	}
}


std::shared_ptr<StateFeatureMap> SimGod::getGlobalStateFeatureMap()
{
	return m_pGlobalStateFeatureMap.sharedPtr();
}
std::shared_ptr<ActionFeatureMap> SimGod::getGlobalActionFeatureMap()
{
	return m_pGlobalActionFeatureMap.sharedPtr();
}


double SimGod::getGamma()
{
	return m_gamma.get();
}

//Returns the number of steps after which deferred V-Function updates are to be done
//0 if we don't use Freeze-V-Function
int SimGod::getTargetFunctionUpdateFreq()
{
	if (m_bFreezeTargetFunctions.get())
		return m_targetFunctionUpdateFreq.get();
	return 0;
}

//We return true if:
//	-freeze target functions is enabled
//	-current step is a multiple of the update freq
//	-we are not replaying experience
bool SimGod::bUpdateFrozenWeightsNow()
{
	int updateFreq = getTargetFunctionUpdateFreq();
	Experiment* pExperiment = SimionApp::get()->pExperiment.ptr();
	return (updateFreq && !bReplayingExperience() && 
		(pExperiment->getExperimentStep() % updateFreq == 0));
}

bool SimGod::useSampleImportanceWeights()
{
	return m_bUseImportanceWeights.get();
}

size_t SimGod::getExperienceReplayUpdateSize()
{
	return (size_t)m_pExperienceReplay->getUpdateBatchSize();
}