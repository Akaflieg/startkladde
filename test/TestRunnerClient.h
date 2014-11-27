/*******************************************************************************
 * Copyright (c) 2008 Gerhard Leonhartsberger.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/

#ifndef TESTRUNNERSERVER_H_
#define TESTRUNNERSERVER_H_

#ifdef CPPUNIT_MAIN

#include "cppunit/TestListener.h"

#include <vector>

/*!
 * Class <code>TestRunnerClient</code> handles the network connection to the
 * <code>CppUnitServer</code> and executes all registered CppUnit tests.
 * <p>
 * The meta data of the CppUnit tests and the test results are sent to the
 * <code>CppUnitServer</code> for displaying.
 * </p>
 * <p>
 * For debugging purposes class <code>TestRunnerClient</code> displays debug
 * messages on <code>std.cerr</code>. The debug mode is activated by specifying
 * <code>debug</code> command line option. The command line is parsed by method
 * <code>Init()</code>.
 * </p>
 * <p>
 * Note: This class is not intended to be subclassed by clients. This class is
 * based on the original <code>RemoteTestRunner</code> class provided by
 * <code>org.eclipse.cdt.cppunit</code>.
 * </p>
 *
 * @author Gerhard Leonhartsberger
 */
class TestRunnerClient: public CppUnit::TestListener
{
private:
	CppUnit::TestResult *fTestResult;
	int fClientSocket;
	char *fHost;
	int fPort;
	int fDebugMode;
	int fKeepAlive;

public:
	TestRunnerClient();
	virtual ~TestRunnerClient();

	/*!
	 * Initializes the <code>TestRunnerClient</code>.
	 * <p>
	 * The given <code>args</code> are parsed. The syntax for the arguments is
	 * defined in EBNF as follows: <code>{-option=value}</code>
	 * </p>
	 *
	 * @param n The number of arguments.
	 * @param args The argument values. Valid options are:
	 * 			   <li><code>-debug</code> When present the TestRunnerClient
	 * 				is run in debug mode and displays debug messages.</li>
	 * 			   <li><code>-port=number</code> Defines the port where
	 * 			   CppUnitServer is listening for client connections.</li>
	 */
	void Init(int n,char *args[]);

	/*!
	 * Runs the TestRunnerClient. A trial to connect to the CppUnitServer is done.
	 * The test results are sent to the CppUnitServer.
	 *
	 * @return The return value is <code>0</code> when the CppUnitServer was connected successfully
	 * otherwise the return value is <code>-1</code>.
	 */
	int Run();

	/*!
	 * Stops processing <code>CppUnit</code> from executing tests.
	 */
	void Stop();

	/*!
	 * Method defined in <code>CppUnit::TestListener</code>.
	 */
	void startTest(CppUnit::Test *test);

	/*!
	 * Method defined in <code>CppUnit::TestListener</code>.
	 */
	void addFailure(const CppUnit::TestFailure &failure);

	/*!
	 * Method defined in <code>CppUnit::TestListener</code>.
	 */
	void endTest(CppUnit::Test *test);

private:
	int Connect();
	void RunTests();
	void ShutDown();

	// utility methods
	void ParseCommandLine(int n, char *args[]);
	void DefineHostName();
	void InstallListeners();
	void UninstallListeners();

	/*!
	 * Sends the given test to the CppUnitView.
	 * <p>
	 * In case of test is of type CppUnit::Test the following protocol is sent:
	 * <code>TSTTREE name, false, testCaseCount</code>
	 * </p>
	 * <p>
	 * In case of test is of type CppUnit::TestSuite the following protocol is sent:
	 * TSTTREE name, true, testCount
	 * </p>
	 * @param test the CppUnit test
	 */
	void SendTestTree(CppUnit::Test *test);
	void SendMessage(std::string msg);

	// Notification methods
	void NotifyTestRunStarted(int testCount);
	void NotifyTestRunEnded(long elapsedTime);
	void NotifyTestRunStopped(long elapsedTime);
	void NotifyTestTreeEntry(std::string treeEntry);
	void NotifyTestStarted(std::string testName);
	void NotifyTestEnded(std::string testName);
	void NotifyTestFailed(std::string status, std::string testName, std::string trace);

	std::string GetTrace(const CppUnit::TestFailure &failure);
	long CurrentTimeMillis();
	void PrivateSleep(int millisecs);
};

#endif /*CPPUNIT_MAIN*/
#endif /*TESTRUNNERSERVER_H_*/
