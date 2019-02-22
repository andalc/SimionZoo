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

#include "OptimizerSetting.h"
#include "xmltags.h"
#include "Exceptions.h"

OptimizerSettings::OptimizerSettings(tinyxml2::XMLElement * pParentNode)
{
	const char* type = pParentNode->Attribute(XML_ATTRIBUTE_Type);
	if (!strcmp(type, "OptimizerSGD"))
		m_optimizerType = OptimizerType::SGD;
	else if (!strcmp(type, "OptimizerMomentumSGD"))
		m_optimizerType = OptimizerType::MomentumSGD;
	else if (!strcmp(type, "OptimizerNesterov"))
		m_optimizerType = OptimizerType::Nesterov;
	else if (!strcmp(type, "OptimizerFSAdaGrad"))
		m_optimizerType = OptimizerType::FSAdaGrad;
	else if (!strcmp(type, "OptimizerAdam"))
		m_optimizerType = OptimizerType::Adam;
	else if (!strcmp(type, "OptimizerAdaGrad"))
		m_optimizerType = OptimizerType::AdaGrad;
	else if (!strcmp(type, "OptimizerRMSProp"))
		m_optimizerType = OptimizerType::RMSProp;
	else if (!strcmp(type, "OptimizerAdaDelta"))
		m_optimizerType = OptimizerType::AdaDelta;
	else
		throw ProblemParserElementValueNotValid(XML_ATTRIBUTE_Type);

	tinyxml2::XMLElement *pNode = pParentNode->FirstChildElement(XML_TAG_Parameters);
	if (pNode == nullptr)
		throw ProblemParserElementNotFound(XML_TAG_Parameters);
	loadChildren<OptimizerParameter>(pNode, XML_TAG_OptimizerParameter, m_parameters);
}

OptimizerSettings::OptimizerSettings()
{
}


OptimizerSettings::~OptimizerSettings()
{
}

OptimizerSettings * OptimizerSettings::getInstance(tinyxml2::XMLElement * pNode)
{
	return new OptimizerSettings(pNode);
}

const OptimizerParameter * OptimizerSettings::getParameterByKey(std::string key) const
{
	for (auto pParameter : m_parameters)
	{
		if (pParameter->getKey() == key)
		{
			return pParameter;
		}
	}

	return nullptr;
}

OptimizerParameter::OptimizerParameter(tinyxml2::XMLElement * pParentNode)
{
	tinyxml2::XMLElement *pNode = pParentNode->FirstChildElement(XML_TAG_Value);
	if (pNode == nullptr)
		throw ProblemParserElementNotFound(XML_TAG_Value);
	m_value = atof(pNode->GetText());

	pNode = pParentNode->FirstChildElement(XML_TAG_Key);
	if (pNode == nullptr)
		throw ProblemParserElementNotFound(XML_TAG_Key);
	m_key = pNode->GetText();
}

OptimizerParameter * OptimizerParameter::getInstance(tinyxml2::XMLElement * pNode)
{
	if (!strcmp(pNode->Name(), XML_TAG_OptimizerParameter))
		return new OptimizerParameter(pNode);
	return nullptr;
}
