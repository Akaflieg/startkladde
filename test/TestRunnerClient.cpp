/*******************************************************************************
 * Copyright (c) 2008 Gerhard Leonhartsberger.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

#include "TestRunnerClient.h"

#ifdef CPPUNIT_MAIN

#include "cppunit/TestResultCollector.h"
#include "cppunit/XmlOutputter.h"
#include "cppunit/TestSuite.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestFailure.h"
#include "cppunit/SourceLine.h"
#include "cppunit/Exception.h"
#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/extensions/TestDecorator.h"
#include "cppunit/ui/text/TestRunner.h"

#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>

#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef _WIN32 // Bugzilla 40710
#include <windows.h>
#include <winbase.h>
#include <winsock.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define MAX_HOSTNAME_SIZE		255

/*
 * CppUnitServer protocol constants
 */
static const std::string TRACE_START = "%TRACES ";
static const std::string TRACE_END = "%TRACEE ";
static const std::string TEST_RUN_START = "%TESTC  ";
static const std::string TEST_START = "%TESTS  ";
static const std::string TEST_END = "%TESTE  ";
static const std::string TEST_ERROR = "%ERROR  ";
static const std::string TEST_FAILED = "%FAILED ";
static const std::string TEST_RUN_END = "%RUNTIME";
static const std::string TEST_STOPPED = "%TSTSTP ";
static const std::string TEST_TREE = "%TSTTREE";

TestRunnerClient::TestRunnerClient()
{
	fTestResult = 0;
	fClientSocket = -1;
	fPort = 0;
	fKeepAlive = 0;
	fDebugMode = 0;

	fHost = (char *) malloc(MAX_HOSTNAME_SIZE);
	strcpy(fHost, "");
}

TestRunnerClient::~TestRunnerClient() {

	if (fHost != NULL) {
		free(fHost);
	}
}

int TestRunnerClient::Run()
{
	if (fDebugMode)
	{
		std::cerr << "TestRunnerClient: Starting client." << std::endl;
	}

	if (Connect() == -1) {
		return -1;
	}

	InstallListeners();

	RunTests();

	UninstallListeners();

	if(fTestResult != NULL)
	{
		fTestResult->stop();
		fTestResult= NULL;
	}

	ShutDown();

	return 0;
}

void TestRunnerClient::Init(int n, char *args[])
{
	ParseCommandLine(n, args);
	DefineHostName();
}

void TestRunnerClient::ParseCommandLine(int n, char *args[])
{
	// parse all arguments passed by args
	for(int i = 0; i < n; i++)
	{
		std::string arg(args[i]);

		// port option
		std::string portOption("-port=");
		int pos = arg.find(portOption);
		if(pos> -1)
		{
			std::string v = arg.substr(pos + portOption.length(), arg.length());
			fPort = atoi(v.c_str());
		}

		// debug option
		std::string debugOption("-debug");
		pos = arg.find(debugOption);
		if(pos> - 1)
		{
			fDebugMode = 1;
		}
	}
}

void TestRunnerClient::DefineHostName()
{
	// set fHost to hostname or localhost
	int ret = gethostname(fHost, MAX_HOSTNAME_SIZE);
	if (ret == -1)
	{
		strcpy(fHost, "localhost");
	}
}

int TestRunnerClient::Connect()
{

#ifdef _WIN32 // Bugzilla 40710
	if (fDebugMode)
	{
		std::cerr << "TestRunnerClient: Starting Windows Sockets WSAStartup()." << std:endl;
	}

	// start up Windows Sockets
	WSADATA WSAData;
	int result = WSAStartup (MAKEWORD(1, 1), &WSAData);
	if (result != NO_ERROR)
	{
		std::cerr << "TestRunnerClient: WSAStartup() failed! Error code: " << result << std::endl;
		return -1;
	}
#endif

	if (fDebugMode)
	{
		std::cerr << "TestRunnerClient: Trying to connect to " << fHost << ":" << fPort << std::endl;
	}

	fClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (fClientSocket == -1)
	{
		std::cerr << "TestRunnerClient: Socket creation failed! error code: " << fClientSocket << std::endl;
		return -1;
	}

	struct hostent *host = gethostbyname(fHost);
	if (host == NULL)
	{
		std::cerr << "TestRunnerClient: Cannot find host address for " << fHost << "." << std::endl;
		fClientSocket = -1;
		return -1;
	}

	struct sockaddr_in name;
	memset((void *)&name, 0, sizeof(struct sockaddr_in));
	name.sin_family = AF_INET;
	name.sin_port = htons(fPort);

	memcpy(&name.sin_addr, host->h_addr, host->h_length);

	if (fDebugMode) {
		std::cerr << "TestRunnerClient: Waiting for the JVM to listen ... (trying 3 times)" << std::endl;
	}

	int ret = -1;
	int j = 0;
	while ((j < 3) && (ret == -1))
	{
		ret = ::connect(fClientSocket, (struct sockaddr *) &name, sizeof(struct sockaddr_in));
		if (ret == -1)
		{
			if (fDebugMode) {
				std::cerr << "TestRunnerClient: Connection request, waiting 1 second. "
				<< ((j-3)*-1) << " times left." << std::endl;
			}
			PrivateSleep(1000);
			j++;
		}
	}
	if (ret == -1)
	{
		std::cerr << "TestRunnerClient: No connection established. Error code: " << errno << std::endl;
		fClientSocket = -1;
		return -1;
	}

	if (fDebugMode) {
		std::cerr << "TestRunnerClient: Connection established." << std::endl;
	}
	return 0;
}

void TestRunnerClient::InstallListeners()
{
	fTestResult = new CppUnit::TestResult();
	fTestResult->addListener(this);
}

void TestRunnerClient::UninstallListeners()
{
	fTestResult->removeListener(this);
}

void TestRunnerClient::RunTests()
{

	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
	CppUnit::Test *suite = registry.makeTest();
	int count = suite->countTestCases();
	NotifyTestRunStarted(count);

	if (count == 0)
	{
		NotifyTestRunEnded(0);
	}

	long startTime = CurrentTimeMillis();
	if (fDebugMode)
	{
		std::cerr <<"TestRunnerClient: Start sending test case tree ..." << std::endl;
	}

	SendTestTree(suite);

	int elapsedTime = CurrentTimeMillis() - startTime;
	if (fDebugMode) {
		std::cerr << "TestRunnerClient: Done sending test case tree. Elapsed time is "
		<< elapsedTime << "ms." << std::endl;
	}

	long testStartTime = CurrentTimeMillis();
	if (fDebugMode) {
		std::cerr << "TestRunnerClient: Test start time is " << testStartTime
		<< "ms." << std::endl;
	}

	suite->run(fTestResult);

	if (fTestResult == NULL || fTestResult->shouldStop())
	{
		NotifyTestRunStopped(CurrentTimeMillis() - testStartTime);
	}
	else
	{
		NotifyTestRunEnded(CurrentTimeMillis() - testStartTime);
	}
}

void TestRunnerClient::ShutDown()
{
	if (fClientSocket != -1)
	{
		if (fDebugMode) {
			std::cerr << "TestRunnerClient: Closing connection to CppUnit sever at "
			<< fHost << ":" << fPort << std::endl;
		}

#ifdef _WIN32 // Bugzilla 40710
		// TODO: std:err output for error return codes
		closesocket(fClientSocket);
		WSACleanup();
#else
		int result = close(fClientSocket);
		if (result != 0)
		{
			std::cerr << "TestRunnerClient: Close connection error: " << errno << std::endl;
		}
#endif

		fClientSocket = -1;
	}
}

void TestRunnerClient::Stop()
{
	if (fTestResult != NULL)
	{
		fTestResult->stop();
	}
}

void TestRunnerClient::SendTestTree(CppUnit::Test *test)
{
	if (typeid(*test) == typeid(CppUnit::TestDecorator))
	{
		class TmpClass : public CppUnit::TestDecorator {

		public:
			TmpClass(Test *t):CppUnit::TestDecorator(t)
			{
				// nothing to do
			}

			~TmpClass() // Bugzilla 39894
			{
				// nothing to do
			}

			CppUnit::Test *getTest()
			{
				return m_test;
			}
		};

		TmpClass *t = (TmpClass *)test;
		SendTestTree(t->getTest());
	}
	else if (typeid(*test) == typeid(CppUnit::TestSuite))
	{
		CppUnit::TestSuite *suite = (CppUnit::TestSuite *)test;
		const std::vector<CppUnit::Test *> &x = suite->getTests();

		std::ostringstream os;
		os << suite->getName() << ",true," << x.size();
		NotifyTestTreeEntry(os.str());

		for(unsigned int i=0; i < x.size(); i++)
		{
			SendTestTree(x[i]);
		}
	}
	else
	{
		std::ostringstream os;
		os << test->getName() << ",false," << test->countTestCases();
		NotifyTestTreeEntry(os.str());
	}
}

void TestRunnerClient::SendMessage(std::string msg)
{
	if (fClientSocket == -1)
	{
		return;
	}

#ifdef _WIN32 // Bugzilla 40710
	send (fClientSocket, msg.c_str(), msg.length(), 0);
	send (fClientSocket, "\n", 1, 0);
#else
	write(fClientSocket, msg.c_str(), msg.length());
	write(fClientSocket, "\n", 1);
#endif

	if (fDebugMode)
	{
		std::cerr << "TestRunnerClient: Sent message \"" << msg << "\""
		<< std::endl;
	}
}

void TestRunnerClient::NotifyTestRunStarted(int testCount)
{
	std::ostringstream os;
	os << TEST_RUN_START << testCount;
	SendMessage(os.str());
}

void TestRunnerClient::NotifyTestRunEnded(long elapsedTime)
{
	std::ostringstream os;
	os << TEST_RUN_END << elapsedTime;
	SendMessage(os.str());
}

void TestRunnerClient::NotifyTestRunStopped(long elapsedTime)
{
	std::ostringstream os;
	os << TEST_STOPPED << elapsedTime;
	SendMessage(os.str());
}

void TestRunnerClient::NotifyTestTreeEntry(std::string treeEntry)
{
	SendMessage(TEST_TREE + treeEntry);
}

void TestRunnerClient::NotifyTestStarted(std::string testName)
{
	SendMessage(TEST_START + testName);
}

void TestRunnerClient::NotifyTestEnded(std::string testName)
{
	SendMessage(TEST_END + testName);
}

void TestRunnerClient::NotifyTestFailed(std::string status, std::string testName, std::string trace)
{
	SendMessage(status + testName);
	SendMessage(TRACE_START);
	SendMessage(trace);
	SendMessage(TRACE_END);
}

// From TestListener
void TestRunnerClient::startTest(CppUnit::Test *test)
{
	NotifyTestStarted(test->getName());
}

// From TestListener
void TestRunnerClient::addFailure(const CppUnit::TestFailure &failure)
{
	if(failure.isError())
	{
		NotifyTestFailed(TEST_ERROR,failure.failedTestName(),GetTrace(failure));
	}
	else
	{
		NotifyTestFailed(TEST_FAILED,failure.failedTestName(),GetTrace(failure));
	}
}

// From TestListener
void TestRunnerClient::endTest(CppUnit::Test *test)
{
	NotifyTestEnded(test->getName());
}

std::string TestRunnerClient::GetTrace(const CppUnit::TestFailure &failure)
{
	std::ostringstream os;

	CppUnit::Exception *e=failure.thrownException();
	if(e->sourceLine().lineNumber()!=-1)
	{
		os << "File " << e->sourceLine().fileName() << ":" << e->sourceLine().lineNumber() << "\n";
	}
	else
	{
		os << "File Unknown:1\n";
	}
	/* TODO: expected, actual value implementation
	 if(typeid(*e)==typeid(CppUnit::NotEqualException))
	 {
	 CppUnit::NotEqualException *ne=(CppUnit::NotEqualException *)e;

	 os << "Expected Value: " << ne->expectedValue() << "\n";
	 os << "Actual Value: " << ne->expectedValue() << "\n";
	 os << "Additional Message: " << ne->additionalMessage() << "\n";
	 }
	 else
	 {
	 End		*/
	os << "Message: " << std::string(e->what()) << "\n";
	/*		} */

	return(os.str());
}

long TestRunnerClient::CurrentTimeMillis()
{
#ifdef _WIN32 // Bugzilla 40710
	unsigned long long p;
	__asm__ __volatile__ ("rdtsc" : "=A" (p));
	return (unsigned long)p;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return((long)(tv.tv_sec*1000) + (tv.tv_usec/1000));
#endif
}

void TestRunnerClient::PrivateSleep(int millisecs)
{
	struct timeval delta;
	delta.tv_sec = (millisecs * 1000L) / 1000000L;
	delta.tv_usec = (millisecs * 1000L) % 1000000L;
	select (0, NULL, NULL, NULL, &delta);
}

/*!
 * This is the main routine. The TestRunnerClient is initialized and run. The
 * CppUnit tests are created, executed, and sent to the CppUnitServer.
 * If no connection to the CppUnitServer was established the CppUnit tests are
 * displayed on the console.
 *
 * @return <code>0</code> if the results of the CppUnit tests were sent to the
 *         CppUnitServer successfully.
 *         <code>-1</code> if a connection could not be established to the
 *         CppUnitServer.
 */
int CPPUNIT_MAIN(int n, char *arg[])
{
	TestRunnerClient client;

	client.Init(n, arg);
	int ret = client.Run();
	if (ret == -1)
	{
		CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
		CppUnit::Test *suite = registry.makeTest();

		CppUnit::TextUi::TestRunner *runner = NULL;
		runner = new CppUnit::TextUi::TestRunner();
		runner->addTest(suite);
		runner->run();
	}

	exit(ret);
}

#endif /*CPPUNIT_MAIN*/
