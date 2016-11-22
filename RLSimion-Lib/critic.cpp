#include "stdafx.h"
#include "critic.h"
#include "vfa-critic.h"
#include "config.h"
#include "logger.h"
#include "vfa.h"

CCritic::CCritic(CConfigNode* pConfigNode)
{
	m_pVFunction = CHILD_OBJECT<CLinearStateVFA>(pConfigNode, "V-Function", "The V-function to be learned");
	//CHILD_CLASS(m_pVFunction, "V-Function","The parameterization of the V-Function to be learned",false, CLinearStateVFA);

	//END_CLASS();
}

std::shared_ptr<CCritic> CCritic::getInstance(CConfigNode* pConfigNode)
{
	return CHOICE<CCritic>(pConfigNode, "Critic", "Critic type",
	{
		{"TD-Lambda",CHOICE_ELEMENT_NEW<CTDLambdaCritic>},
		{"True-Online-TD-Lambda",CHOICE_ELEMENT_NEW<CTrueOnlineTDLambdaCritic>},
		{"TDC-Lambda",CHOICE_ELEMENT_NEW<CTDCLambdaCritic>}
	});
	//CHOICE("Critic","Critic type");
	//CHOICE_ELEMENT("TD-Lambda", CTDLambdaCritic,"TD-Lambda algorithm");
	//CHOICE_ELEMENT("True-Online-TD-Lambda", CTrueOnlineTDLambdaCritic,"True-online TD-Lambda algorithm");
	//CHOICE_ELEMENT("TDC-Lambda", CTDCLambdaCritic,"TDC-Lambda algorithm");
	//END_CHOICE();

	//END_CLASS();

	//return 0;
}

CCritic::~CCritic()
{
}
