/*
* Membrane Software Reference Source License
* Version 2024 Sep 18
* This license is a legal agreement between you and Membrane Software
*
* This license agreement governs use of the accompanying source code. If you use the source code, you accept this license. If you do not accept the license, do not use the source code.
*
* DEFINITIONS
* "compilation" means to compile the code from source code to machine code.
* “non-commercial distribution” means distribution of the code or any compilation of the code, or of any other application or program containing the code or any compilation of the code, where such distribution is not intended for or directed towards commercial advantage or monetary compensation.
* "review" means to access, analyse, test and otherwise review the code as a reference
* "you" means the licensee of rights set out in this license.
*
* GRANT OF RIGHTS
* Subject to the terms of this license, we grant you a non-transferable, non-exclusive, worldwide, royalty-free license to access and use the source code solely for the purposes of review, compilation and non-commercial distribution.
*
* LIMITATIONS
* This license does not grant you any rights to use Membrane Software's name, logo, or trademarks.
*
* If you issue proceedings in any jurisdiction against Membrane Software because you consider Membrane Software has infringed copyright or any patent right in respect of the code (including any joinder or counterclaim), your license to the code is automatically terminated.
*
* This source code is provided by the copyright holders and contributors "as is" and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the copyright holder or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this source code, even if advised of the possibility of such damage.
*
* NO IMPLIED RIGHTS
* All rights not expressly granted by Membrane Software to you in this License Agreement are hereby reserved by Membrane Software and its suppliers. There are no implied rights in this License Agreement.
*
* GOVERNING LAW
* This EULA agreement, and any dispute arising out of or in connection with this EULA agreement, shall be governed by and construed in accordance with the laws of Washington State, United States of America.
*
* QUESTIONS OR ADDITIONAL INFORMATION
* If you have questions regarding this License Agreement, please contact Membrane Software by sending an email to support@membranesoftware.com.
*/
#include "Config.h"
#include <signal.h>
#include "StringList.h"
#include "App.h"

// Execute operations appropriate when the process is about to exit
static void cleanup ();

#if PLATFORM_LINUX || PLATFORM_MACOS
// Handle a signal by halting the application
static void sighandleExit (int signum);

// Handle a signal by taking no action
static void sighandleDiscard (int signum);
#endif

#if PLATFORM_WINDOWS
int CALLBACK WinMain (_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#else
int main (int argc, char **argv)
#endif
{
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct sigaction action;
#endif
#if PLATFORM_WINDOWS
	int argc, count;
	char **argv;
	StdString cmdline;
	StringList cmdargs;
	StringList::const_iterator i1, i2;
#endif
	int result, exitstatus;

	atexit (cleanup);

#if PLATFORM_LINUX || PLATFORM_MACOS
	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleExit;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	sigaction (SIGINT, &action, NULL);

	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleExit;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	sigaction (SIGQUIT, &action, NULL);

	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleExit;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	sigaction (SIGTERM, &action, NULL);

	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleDiscard;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = SA_RESTART;
	sigaction (SIGPIPE, &action, NULL);
#endif

	exitstatus = 0;
	App::createInstance ();
#if PLATFORM_WINDOWS
	cmdline.assign (lpCmdLine);
	cmdline.split (" ", &cmdargs);
	cmdargs.push_front (StdString ());
	argc = (int) cmdargs.size ();
	argv = (char **) malloc (argc * sizeof (char *));
	count = 0;
	i1 = cmdargs.cbegin ();
	i2 = cmdargs.cend ();
	while (i1 != i2) {
		argv[count] = (char *) i1->c_str ();
		++count;
		++i1;
	}
#endif
	result = App::instance->run (argc, argv);
#if PLATFORM_WINDOWS
	free (argv);
	cmdargs.clear ();
#endif
	if (result != OpResult::Success) {
		if (App::instance->log.isFileWriteEnabled) {
			printf ("Failed to execute application. Visit membranesoftware.com for help or an updated version. Errors written to log file: %s\n", App::instance->log.writeFilename.c_str ());
		}
		else {
			printf ("Failed to execute application. Visit membranesoftware.com for help or an updated version. error=%i\n", result);
		}
		exitstatus = 1;
	}
	exit (exitstatus);
}

void cleanup () {
	App::freeInstance ();
}

#if PLATFORM_LINUX || PLATFORM_MACOS
void sighandleExit (int signum) {
	App::instance->shutdown ();
}

void sighandleDiscard (int signum) {
}
#endif
