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

#include "utils.h"
#include "config.h"
#include <algorithm>
#include <fstream>

Table::Table()
{
}

Table::~Table()
{
}

unsigned int parseHeader(const char* pInputString, vector<double>& outVector, char delimChar= '\t')
{
	unsigned int numParsedValues = 0;
	if (!pInputString) return 0;

	const char* pt = pInputString;
	if (*pt == delimChar || *pt == '\n') ++pt; //skip initial space/line jump
	while (*pt != 0)
	{
		outVector.push_back (atof(pt));
		++numParsedValues;

		while (*pt != 0 && *pt != delimChar && *pt != '\n') ++pt;
		if (*pt == delimChar || *pt == '\n') ++pt;
	}
	return numParsedValues;
}

bool Table::readFromFile(string filename)
{
	//Reads a matrix from a file. The format should be:
//		3	5	7	9
//9		1.2	3.2	-3	-3.4
//10	2.3	2.5	-3	-4.2
	char line[1024];

	ifstream inputFile(filename);
	if (inputFile.is_open())
	{
		//read the column headers
		inputFile.getline(line,1024);
		int numColumns= parseHeader(line, m_columns, '\t');
		int numRows = 0;
		double value;
		while (!inputFile.eof())
		{
			inputFile >> value;
			m_rows.push_back(value);
			for (int i = 0; i < numColumns; ++i)
			{
				inputFile >> value;
				m_values.push_back(value);
			}
			++numRows;
		}

		inputFile.close();

		if (numRows*numColumns == (int) m_values.size())
		{
			m_bSuccess = true;
			return true;
		}
	}
	m_bSuccess = false;
	return false;
}

double Table::getInterpolatedValue(double columnValue, double rowValue) const
{
	//check column and row values are within the defined ranges
	columnValue = std::max(m_columns[0], std::min(m_columns[m_columns.size() - 1], columnValue));
	rowValue = std::max(m_rows[0], std::min(m_rows[m_rows.size() - 1], rowValue));

	//search for the columns/rows where the given values are in
	int colIndex = 1, rowIndex = 1;
	while (m_columns[colIndex] < columnValue) ++colIndex;
	while (m_rows[rowIndex] < rowValue) ++rowIndex;
	--colIndex;
	--rowIndex;

	//interpolation factors
	double colU, rowU, colRange, rowRange;
	colRange = m_columns[colIndex + 1] - m_columns[colIndex];
	rowRange = m_rows[rowIndex + 1] - m_rows[rowIndex];
	colU = (columnValue - m_columns[colIndex]) / colRange;
	rowU = (rowValue - m_rows[rowIndex]) / rowRange;
	double value= 0.0;
	value += (1 - colU) * (1- rowU) * getValue(colIndex,rowIndex);
	value += (1 - colU) * rowU * getValue(colIndex,rowIndex + 1);
	value += colU * (1 - rowU) * getValue(colIndex + 1, rowIndex);
	value += colU * rowU * getValue(colIndex + 1, rowIndex + 1);
	return value;
}

double Table::getValue(size_t col, size_t row) const
{
	col = std::max((size_t)0, std::min(col, m_columns.size() - 1));
	row = std::max((size_t)0, std::min(row, m_rows.size() - 1));

	return m_values[row*m_columns.size() + col];
}

double Table::getMinCol() const
{
	if (!m_bSuccess) return 0.0;
	return m_columns[0];
}

double Table::getMaxCol() const
{
	if (!m_bSuccess) return 0.0;
	return m_columns[m_columns.size() - 1];
}

double Table::getMinRow() const
{
	if (!m_bSuccess) return 0.0;
	return m_rows[0];
}

double Table::getMaxRow() const
{
	if (!m_bSuccess) return 0.0;
	return m_rows[m_rows.size() - 1];
}

double Table::getNumCols() const
{
	if (!m_bSuccess) return (double) m_columns.size();
	return 0;
}

double Table::getNumRows() const
{
	if (!m_bSuccess) return (double) m_rows.size();
	return 0;
}