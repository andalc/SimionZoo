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
using Herd.Files;
using Badger.ViewModels;

namespace Badger.Data
{
    public class LogFileUtils
    {
        public static Series GetVariableData(Log.EpisodesData episode, Report trackParameters, int variableIndex)
        {

            if (episode.steps[episode.steps.Count - 1].episodeSimTime < trackParameters.MinEpisodeLength)
                return null;

            Series data = new Series();

            foreach (Log.StepData step in episode.steps)
            {
                if (step.episodeSimTime >= trackParameters.TimeOffset)
                    data.AddValue(step.episodeSimTime
                     , ProcessFunc.Get(trackParameters.ProcessFunc, step.data[variableIndex]));
            }

            if (data.Values.Count == 0)
                return null;

            data.CalculateStats(trackParameters);

            if (trackParameters.Resample)
                data.Resample(trackParameters.NumSamples);
            return data;
        }

        public static double GetEpisodeAverage(Log.EpisodesData episode, int variableIndex, Report trackParameters)
        {
            double avg = 0.0;
            int count = 0;
            if (episode.steps.Count == 0) return 0.0;
            foreach (Log.StepData step in episode.steps)
            {
                if (step.episodeSimTime >= trackParameters.TimeOffset)
                {
                    avg += ProcessFunc.Get(trackParameters.ProcessFunc, step.data[variableIndex]);
                    count++;
                }
            }
            return avg / count;
        }

        public static SeriesGroup GetAveragedData(List<Log.EpisodesData> episodes, Report trackParameters, int variableIndex)
        {
            SeriesGroup data = new SeriesGroup(trackParameters);
            Series xYSeries = new Series();

            foreach (Log.EpisodesData episode in episodes)
            {
                xYSeries.AddValue(episode.index
                    , GetEpisodeAverage(episode, variableIndex, trackParameters));
            }
            xYSeries.CalculateStats(trackParameters);
            if (trackParameters.Resample)
                xYSeries.Resample(trackParameters.NumSamples);
            data.AddSeries(xYSeries);
            return data;
        }

        /// <summary>
        /// Reads the log file and returns in a track the data for each of the reports.
        /// </summary>
        /// <param name="reports">Parameters of each of the reporters: variable, type, ...</param>
        /// <returns></returns>
        public static Track LoadTrackData(LoggedExperimentalUnitViewModel expUnit, List<Report> reports)
        {
            Log.SimionLog Log = new Log.SimionLog();
            Log.LoadBinaryLog(expUnit.LogFileName);

            if (!Log.SuccessfulLoad || Log.TotalNumEpisodes == 0) return null;

            Track track = new Track(expUnit.ForkValues, expUnit.LogFileName, expUnit.LogDescriptorFileName, expUnit.ExperimentFileName);
            SeriesGroup dataSeries;
            int variableIndex;
            foreach (Report report in reports)
            {
                variableIndex = expUnit.GetVariableIndex(report.Variable);
                switch (report.Type)
                {
                    case ReportType.LastEvaluation:
                        Log.EpisodesData lastEpisode = Log.EvaluationEpisodes[Log.EvaluationEpisodes.Count - 1];
                        dataSeries = new SeriesGroup(report);
                        Series series = LogFileUtils.GetVariableData(lastEpisode, report, variableIndex);
                        if (series != null)
                        {
                            dataSeries.AddSeries(series);
                            track.AddVariableData(report, dataSeries);
                        }
                        break;
                    case ReportType.EvaluationAverages:
                        track.AddVariableData(report
                            , LogFileUtils.GetAveragedData(Log.EvaluationEpisodes, report, variableIndex));
                        break;
                    case ReportType.AllEvaluationEpisodes:
                    case ReportType.AllTrainingEpisodes:
                        dataSeries = new SeriesGroup(report);
                        List<Log.EpisodesData> episodes;
                        if (report.Type == ReportType.AllEvaluationEpisodes)
                            episodes = Log.EvaluationEpisodes;
                        else episodes = Log.TrainingEpisodes;
                        foreach (Log.EpisodesData episode in episodes)
                        {
                            Series subSeries = LogFileUtils.GetVariableData(episode, report, variableIndex);
                            if (subSeries != null)
                            {
                                subSeries.Id = episode.index.ToString();
                                dataSeries.AddSeries(subSeries);
                            }
                        }
                        track.AddVariableData(report, dataSeries);
                        break;
                }
            }
            return track;
        }
    }
}
