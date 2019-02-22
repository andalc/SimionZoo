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
using System.Xml;
using System.IO;

namespace Herd.Files
{
    public class ExperimentBatch
    {
        public List<Experiment> Experiments = new List<Experiment>();

        public string FileName = null;

        public ExperimentBatch()
        {

        }

        public void Load(string batchFilename, LoadOptions loadOptions)
        {
            Experiments.Clear();
            FileName = batchFilename;
            try
            {
                XmlDocument batchDoc = new XmlDocument();
                batchDoc.Load(batchFilename);
                XmlElement fileRoot = batchDoc.DocumentElement;

                if (fileRoot != null && fileRoot.Name == XMLTags.ExperimentBatchNodeTag)
                {
                    foreach (XmlNode experiment in fileRoot.ChildNodes)
                    {
                        if (experiment.Name == XMLTags.ExperimentNodeTag)
                            Experiments.Add(new Experiment(experiment, Utils.GetDirectory(batchFilename), loadOptions));
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        /// <summary>
        /// This method counts the number of experimental units loaded. Load() must be called before!!
        /// </summary>
        /// <returns></returns>
        public int CountExperimentalUnits()
        {
            int counter = 0;

            foreach(Experiment exp in Experiments)
                counter += exp.ExperimentalUnits.Count;

            return counter;
        }

        /// <summary>
        /// This method loads the batch file and deletes any log file found
        /// </summary>
        /// <param name="batchFilename"></param>
        /// <returns></returns>
        public static int DeleteLogFiles(string batchFilename)
        {
            int counter = 0;
            try
            {
                XmlDocument batchDoc = new XmlDocument();
                batchDoc.Load(batchFilename);
                XmlElement fileRoot = batchDoc.DocumentElement;

                if (fileRoot != null && fileRoot.Name == XMLTags.ExperimentBatchNodeTag)
                {
                    foreach (XmlNode experiment in fileRoot.ChildNodes)
                    {
                        if (experiment.Name == XMLTags.ExperimentNodeTag)
                        {
                            foreach(XmlNode child in experiment.ChildNodes)
                            {
                                if (child.Name == XMLTags.ExperimentalUnitNodeTag)
                                {
                                    string expUnitPath = child.Attributes[XMLTags.pathAttribute].Value;
                                    string baseDirectory = Utils.GetDirectory(batchFilename);
                                    string logDescPath = Utils.GetLogFilePath(baseDirectory + expUnitPath, true);
                                    string logPath = Utils.GetLogFilePath(baseDirectory + expUnitPath, false);
                                    bool logDescExists = File.Exists(logDescPath);
                                    bool logExists = File.Exists(logPath);
                                    if (logExists || logDescExists)
                                        counter++;
                                    if ( logDescExists )
                                        File.Delete(logDescPath);
                                    if ( logExists )
                                        File.Delete(logPath);
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            return counter;
        }


        /// <summary>
        /// This method loads the experiment batch file and counts the type of experimental units required: All, only finished, or only unfinished
        /// </summary>
        /// <param name="batchFilename"></param>
        /// <param name="selection"></param>
        /// <returns></returns>
        public static int CountExperimentalUnits(string batchFilename, LoadOptions.ExpUnitSelection selection)
        {
            int counter = 0;
            try
            {
                XmlDocument batchDoc = new XmlDocument();
                batchDoc.Load(batchFilename);
                XmlElement fileRoot = batchDoc.DocumentElement;

                if (fileRoot != null && fileRoot.Name == XMLTags.ExperimentBatchNodeTag)
                {
                    foreach (XmlNode experiment in fileRoot.ChildNodes)
                    {
                        if (experiment.Name == XMLTags.ExperimentNodeTag)
                        {
                            foreach (XmlNode child in experiment.ChildNodes)
                            {
                                if (child.Name == XMLTags.ExperimentalUnitNodeTag)
                                {
                                    if (selection == LoadOptions.ExpUnitSelection.All) counter++;
                                    else
                                    {                                 
                                        string expUnitPath = child.Attributes[XMLTags.pathAttribute].Value;
                                        string baseDirectory = Utils.GetDirectory(batchFilename);
                                        string logDescPath = Utils.GetLogFilePath(baseDirectory + expUnitPath, true);
                                        string logPath = Utils.GetLogFilePath(baseDirectory + expUnitPath, false);

                                        bool logExists = Utils.FileExistsAndNotEmpty(logDescPath) && Utils.FileExistsAndNotEmpty(logPath);

                                        if (logExists == (selection==LoadOptions.ExpUnitSelection.OnlyFinished))
                                            counter++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            return counter;
        }
    }
}
