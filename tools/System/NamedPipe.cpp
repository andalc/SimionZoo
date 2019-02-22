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

#include "NamedPipe.h"
#include <Windows.h>
#include <string>
#include <thread>

#define NUM_MAX_CONNECTION_ATTEMPTS 10

//NamedPipe: Common stuff
NamedPipe::NamedPipe()
{
	m_pipeHandle = (unsigned long long int)INVALID_HANDLE_VALUE;
}

NamedPipe::~NamedPipe()
{
	if (m_pipeHandle != (unsigned long long int)INVALID_HANDLE_VALUE)
	{
		logMessage("Pipe closed");
		CloseHandle((void*)m_pipeHandle);
	}
}

bool NamedPipe::isConnected()
{
	return (m_pipeHandle != (unsigned long long int)INVALID_HANDLE_VALUE);
}

void NamedPipe::setPipeName(const char* pipeName,bool bAddPrefix,int id)
{
	//this method builds the name of the pipe in m_pipeFullName
	//if bAddPrefix, it prepends the pipeName (i.e. "myPipe") with the standard pipe prefix in windows
	//if id>0, then the base name is appended with the id number until a new pipe can be opened with that name
	if (bAddPrefix)
	{
		if (id < 0)
			sprintf_s(m_pipeFullName, "\\\\.\\pipe\\%s", pipeName);
		else
			sprintf_s(m_pipeFullName, "\\\\.\\pipe\\%s-%d", pipeName, id);
	}
	else
	{
		if (id < 0)
			sprintf_s(m_pipeFullName, "%s", pipeName);
		else
			sprintf_s(m_pipeFullName, "%s-%d", pipeName, id);
	}
}

int NamedPipe::writeBuffer(const void* pBuffer, int numBytes)
{
	DWORD bytesWritten= 0;
	if (m_pipeHandle != (unsigned long long int) INVALID_HANDLE_VALUE)
	{
		WriteFile((void*)m_pipeHandle, pBuffer, numBytes, &bytesWritten, NULL);
		logMessage("Written in pipe");
	}
	else logMessage("Error: couldn't write in pipe");

	return bytesWritten;
}

int NamedPipe::readToBuffer(void *pBuffer, int numBytes)
{
	DWORD bytesRead= 0;
	if (m_pipeHandle != (unsigned long long int) INVALID_HANDLE_VALUE)
	{
		ReadFile((void*)m_pipeHandle, pBuffer, numBytes, &bytesRead, NULL);
		logMessage("Read from pipe");
	}
	return bytesRead;
}


//CNamedPipeServer
NamedPipeServer::NamedPipeServer()
{
	m_pipeHandle = (unsigned long long int) INVALID_HANDLE_VALUE;
}

NamedPipeServer::~NamedPipeServer()
{
	closeServer();
}

bool NamedPipeServer::openNamedPipeServer()
{
	m_pipeHandle = (unsigned long long int) CreateNamedPipe(m_pipeFullName,
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | FILE_FLAG_FIRST_PIPE_INSTANCE,
		PIPE_WAIT,
		1,
		1024 * 16,
		1024 * 16,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);

	if (m_pipeHandle == (unsigned long long int) INVALID_HANDLE_VALUE)
	{
		logMessage("Created pipe server");
		return false;
	}
	logMessage("Error: couldn't create pipe server");
	return true;
}


bool NamedPipeServer::waitForClientConnection()
{
	logMessage("Waiting for client connection on pipe");

	if (ConnectNamedPipe((void*)m_pipeHandle, NULL) != FALSE)   // wait for someone to connect to the pipe
	{
		logMessage("Client connected to pipe server");
		return true;
	}
	return false;
}


void NamedPipeServer::closeServer()
{
	if (m_pipeHandle != (unsigned long long int)INVALID_HANDLE_VALUE)
	{
		logMessage("Pipe server closed");
		DisconnectNamedPipe((void*) m_pipeHandle);
		CloseHandle((void*) m_pipeHandle);
		m_pipeHandle = (unsigned long long int) INVALID_HANDLE_VALUE;
	}
}

//CNamedPipeClient
NamedPipeClient::NamedPipeClient()
{
	m_pipeHandle = (unsigned long long int) INVALID_HANDLE_VALUE;
}

NamedPipeClient::~NamedPipeClient()
{
	closeConnection();
}
bool NamedPipeClient::connectToServer(const char* pipeName, bool addPipePrefix)
{
	int numAttempts = 0;

	if (m_pipeHandle != (unsigned long long int) INVALID_HANDLE_VALUE)
		return false;

	setPipeName(pipeName, addPipePrefix);
	do
	{
		m_pipeHandle = (unsigned long long int) CreateFile(
			m_pipeFullName,   // pipe name 
			PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			PIPE_WAIT,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);
		numAttempts++;
		
		if (m_pipeHandle == (unsigned long long int) INVALID_HANDLE_VALUE)
			std::this_thread::sleep_for(std::chrono::milliseconds(500));

	} while (m_pipeHandle == (unsigned long long int) INVALID_HANDLE_VALUE && numAttempts<NUM_MAX_CONNECTION_ATTEMPTS);

	if (m_pipeHandle == (unsigned long long int) INVALID_HANDLE_VALUE)
	{
		logMessage("Error: couldn't connect to pipe server");
		return false;
	}

	logMessage("Client connected to pipe server");
	return true;
}

void NamedPipeClient::closeConnection()
{
	if (m_pipeHandle != (unsigned long long int)INVALID_HANDLE_VALUE)
	{
		logMessage("Closing connection to pipe server");
		FlushFileBuffers((void*) m_pipeHandle);
		CloseHandle((void*) m_pipeHandle);
		m_pipeHandle = (unsigned long long int) INVALID_HANDLE_VALUE;
	}
}