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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Text.RegularExpressions;

namespace SimionSrcParser
{
    
    class SimionSrcParser
    {
        //PRIVATE stuff///////////////////////////////////////////////////
        string m_currentFile = "";
        static List<ParameterizedObject> m_parameterizedObjects= new List<ParameterizedObject>();
        public static ParameterizedObject getNamedParamObject(string name)
        {
            return m_parameterizedObjects.Find(obj=>obj.name==name);
        }

        public static void getEnclosedBody(string content, int startIndex,string openChar,string closeChar
            , out string definition, out string prefix)
        {
            int contentLength = content.Length;
            int numOpenChars= 0;
            int firstOpenPos = content.IndexOf(openChar, startIndex);
            prefix = content.Substring(startIndex, firstOpenPos-startIndex);
            int pos = firstOpenPos+1;
            string c = content.Substring(pos, 1);
            while (pos<contentLength && (c!=closeChar || numOpenChars!=0))
            {
                if (c ==openChar)
                    numOpenChars++;
                if (c == closeChar)
                    numOpenChars--;
                pos++;
                c = content.Substring(pos, 1);
            }
            definition = content.Substring(firstOpenPos + 1, pos -firstOpenPos- 1);
        }
        public static void addIndentation(ref string definition, int level)
        {
            for (int i = 0; i < level; i++) definition += "  ";
        }

        void parseConstructors(string content)
        {
            string sPattern = @"(\w+)::\1\(\s*ConfigNode\s*\*\s*([^)]+)\)";

            string className, paramName, prefix,definition;
            foreach (Match match in Regex.Matches(content, sPattern))
            {
                className = match.Groups[1].Value;
                //Console.WriteLine("Found constructor definition: " + className);
                paramName = match.Groups[2].Value;
                getEnclosedBody(content,match.Index + match.Length,"{","}"
                    , out definition, out prefix);
                m_parameterizedObjects.Add(new Constructor(className, paramName, definition, prefix));
            }
        }
        void parseFactories(string content)
        {
            string sPattern = @"(\w+)::getInstance\(\s*ConfigNode\s*\*\s*([^)]+)\)";

            string className, paramName, prefix, definition;
            foreach (Match match in Regex.Matches(content, sPattern))
            {
                className = match.Groups[1].Value + "-Factory";
                //Console.WriteLine("Found factory definition: " + className);
                paramName = match.Groups[2].Value;
                getEnclosedBody(content, match.Index + match.Length, "{", "}"
                    , out definition, out prefix);
                m_parameterizedObjects.Add(new Factory(className, paramName, definition, prefix));
            }
        }
        void parseEnumerations(string content)
        {
            string sPattern = @"enum class\s*(\w+)\s*{([^}]+)}";

            string enumName;
            foreach (Match match in Regex.Matches(content, sPattern))
            {
                enumName = match.Groups[1].Value;
                m_parameterizedObjects.Add(new Enumeration(match.Groups[1].Value, match.Groups[2].Value));
            }
        }
        //PUBLIC methods//////////////////////////////////////////////////
        public int numCharsProcessed = 0;
        public SimionSrcParser() { }
        public void parseSrcFile(string filename)
        {
            //Console.WriteLine("Parsing source file " + filename);
            m_currentFile = filename;
            string fileContents = File.ReadAllText(filename, Encoding.UTF8);
            fileContents = fileContents.Replace('\r', ' ');
            fileContents = fileContents.Replace('\n', ' ');

            numCharsProcessed += fileContents.Length;
            parseConstructors(fileContents);
            parseFactories(fileContents);
            parseEnumerations(fileContents);
        }
        public void parseHeaderFile(string filename)
        {
            //Console.WriteLine("Parsing header file " + filename);
            m_currentFile = filename;
            string fileContents = File.ReadAllText(filename, Encoding.UTF8);
            fileContents = fileContents.Replace('\r', ' ');
            fileContents = fileContents.Replace('\n', ' ');

            numCharsProcessed += fileContents.Length;
            parseEnumerations(fileContents);
        }
        public int postProcess()
        {
            return 0;
        }
        public void saveDefinitions(string filename)
        {
            FileStream file = new FileStream(filename,FileMode.Create);
            StreamWriter outputFile = new StreamWriter(file);

            //// encoding=\"utf-8\"
            outputFile.WriteLine("<?xml version=\"1.0\"?>\n<DEFINITIONS>");
            foreach (ParameterizedObject paramObj in m_parameterizedObjects)
                outputFile.Write(paramObj.outputXML(1));
            outputFile.Write(@"</DEFINITIONS>");
            outputFile.Close();
        }
    }
}
