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

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Xml;

using Herd.Files;

namespace Badger.ViewModels
{
    class LinkedNodeViewModel : ConfigNodeViewModel
    {
        private ConfigNodeViewModel m_origin;

        public ConfigNodeViewModel Origin
        {
            get { return m_origin; }
            set
            {
                m_origin = value;
                NotifyOfPropertyChange(() => Origin);
            }
        }

        private string m_originName;

        public string OriginName
        {
            get { return m_originName; }
            set
            {
                m_originName = value;
                NotifyOfPropertyChange(() => OriginName);
            }
        }

        private string m_originAlias;

        /// <summary>
        /// Used when the link origin is a fork
        /// </summary>
        public string OriginAlias
        {
            get { return m_originAlias; }
            set
            {
                m_originAlias = value;
                NotifyOfPropertyChange(() => OriginAlias);
            }
        }

        private ConfigNodeViewModel m_linkedNode;

        public ConfigNodeViewModel LinkedNode
        {
            get { return m_linkedNode; }
            set
            {
                m_linkedNode = value;
                NotifyOfPropertyChange(() => LinkedNode);
            }
        }

        /// <summary>
        /// Constructor used from the experiment editor
        /// </summary>
        /// <param name="parentExperiment"></param>
        /// <param name="originNode"></param>
        /// <param name="targetNode"></param>
        public LinkedNodeViewModel(ExperimentViewModel parentExperiment, ConfigNodeViewModel originNode,
            ConfigNodeViewModel targetNode)
        {
            m_parent = targetNode.parent;
            m_parentExperiment = parentExperiment;
            Origin = originNode;

            nodeDefinition = targetNode.nodeDefinition;
            name = targetNode.name;
            comment = nodeDefinition.Attributes[XMLTags.commentAttribute].Value;
            content = originNode.content;

            CreateLinkedNode();
        }

        /// <summary>
        /// Constructor called when loading an experiment config file
        /// </summary>
        /// <param name="parentExperiment"></param>
        /// <param name="parentNode"></param>
        /// <param name="classDefinition">Class of the node in app definitions</param>
        /// <param name="parentXPath"></param>
        /// <param name="configNode">Node in simion.exp file with the configuration for a node class</param>
        public LinkedNodeViewModel(ExperimentViewModel parentExperiment, ConfigNodeViewModel parentNode,
            XmlNode classDefinition, XmlNode configNode = null)
        {
            m_parentExperiment = parentExperiment;
            nodeDefinition = classDefinition;
            m_parent = parentNode;

            foreach (XmlNode configChildNode in configNode)
            {
                if (configChildNode.Name.Equals(XMLTags.linkedNodeTag)
                    && configChildNode.Attributes[XMLTags.nameAttribute].Value
                    .Equals(classDefinition.Attributes[XMLTags.nameAttribute].Value))
                {
                    OriginName = configChildNode.Attributes[XMLTags.originNodeAttribute].Value;
                    if (configChildNode.Attributes[XMLTags.aliasAttribute] != null)
                        OriginAlias = configChildNode.Attributes[XMLTags.aliasAttribute].Value;
                }
            }

            name = nodeDefinition.Attributes[XMLTags.nameAttribute].Value;
            LinkedNode = getInstance(parentExperiment, parentNode, classDefinition,
                parentExperiment.AppName, configNode);
        }

        //constructor used in clone()
        public LinkedNodeViewModel() { }


        public override ConfigNodeViewModel clone()
        {
            return null;
        }


        public override void outputXML(StreamWriter writer, SaveMode mode, string leftSpace)
        {
            if (mode == SaveMode.AsProject || mode == SaveMode.AsExperiment)
            {
                writer.WriteLine(leftSpace + "<" + XMLTags.linkedNodeTag + " "
                    + XMLTags.nameAttribute + "=\"" + name.TrimEnd(' ') + "\" " + XMLTags.originNodeAttribute
                    + "=\"" + m_origin.name + "\"/>");
            }
        }

        public override bool Validate()
        {
            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        public void CreateLinkedNode()
        {
            // We need the linked node to be shown exactly as the origin node
            LinkedNode = Origin.clone();
            // But we need to protects its identity, so lets put a few things back to normal
            LinkedNode.nodeDefinition = nodeDefinition;
            LinkedNode.LinkedNodes = LinkedNodes;
            LinkedNode.name = name;
            LinkedNode.comment = nodeDefinition.Attributes[XMLTags.commentAttribute].Value;

            if (LinkedNode is NestedConfigNode)
            {
                NestedConfigNode node = (NestedConfigNode)LinkedNode;
                
            }
        }
    }
}
