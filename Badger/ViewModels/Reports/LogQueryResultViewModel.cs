﻿using System;
using System.Collections.Generic;
using Caliburn.Micro;
using Badger.Simion;
using Badger.Data;
using System.ComponentModel;
using System.Windows.Data;
using System.Globalization;
using System.IO;

namespace Badger.ViewModels
{
    public class LogQueryResultViewModel : PropertyChangedBase
    {
        static int numQueryResults = 0;

        string getDefaultQueryResultName()
        {
            numQueryResults++;
            return "Query-" + numQueryResults;
        }

        public LogQueryViewModel Query { get; set; } = new LogQueryViewModel();

        string m_name = "Unnamed";
        public string Name
        {
            get { return m_name; }
            set { m_name = value; NotifyOfPropertyChange(() => Name); }
        }

        //report list
        public BindableCollection<ReportViewModel> Reports
        { get; } = new BindableCollection<ReportViewModel>();

        //report selection in tab control
        private ReportViewModel m_selectedReport;

        public ReportViewModel SelectedReport
        {
            get { return m_selectedReport; }
            set
            {
                m_selectedReport = value;
                NotifyOfPropertyChange(() => SelectedReport);
            }
        }

        public LogQueryResultViewModel(LogQueryViewModel query)
        {
            Name = getDefaultQueryResultName();
            Query.DeepCopy(query);

            // Display the reports
            foreach (Report report in query.Reports)
            {
                ReportViewModel newReport = new ReportViewModel(query, report);
                Reports.Add(newReport);
            }

            //Set the last as selected
            if (Reports.Count > 0)
                SelectedReport = Reports[Reports.Count-1];
        }

        public void Export(string outputBaseFolder)
        {
            if (outputBaseFolder != "")
            {
                foreach (ReportViewModel report in Reports)
                {
                    // If there is more than one report, we store each one in a subfolder
                    string outputFolder = outputBaseFolder + "\\" + Utility.RemoveSpecialCharacters(report.Name);

                    if (!Directory.Exists(outputFolder))
                        Directory.CreateDirectory(outputFolder);

                    report.Export(outputFolder);
                }
            }
        }
    }
}
