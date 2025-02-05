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
#include "SdlUtil.h"
#include "TaskGroup.h"

TaskGroup *TaskGroup::instance = NULL;

TaskGroup::TaskGroup ()
: maxThreadCount (0)
, isStopped (false)
, runCount (0)
, threadCount (0)
, nextThreadIndex (1)
{
	SdlUtil::createMutex (&contextListMutex);
}
TaskGroup::~TaskGroup () {
	SdlUtil::destroyMutex (&contextListMutex);
}

void TaskGroup::createInstance () {
	if (! TaskGroup::instance) {
		TaskGroup::instance = new TaskGroup ();
	}
}
void TaskGroup::freeInstance () {
	if (TaskGroup::instance) {
		delete (TaskGroup::instance);
		TaskGroup::instance = NULL;
	}
}

bool TaskGroup::run (TaskGroup::RunContext fn, TaskGroup::EndCallbackContext endCallback) {
	TaskGroup::TaskContext ctx;

	if (isStopped) {
		return (false);
	}
	ctx.fn = fn;
	ctx.endCallback = endCallback;

	SDL_LockMutex (contextListMutex);
	contextList.push_back (ctx);
	runCount = (int) contextList.size ();
	SDL_UnlockMutex (contextListMutex);
	return (true);
}

int TaskGroup::executeTask (void *taskContextPtr) {
	TaskGroup::TaskContext *ctx;

	ctx = (TaskGroup::TaskContext *) taskContextPtr;
	ctx->fn.fn (ctx->fn.fnData);
	ctx->isRunning = false;
	return (0);
}

void TaskGroup::update (int msElapsed) {
	std::list<TaskGroup::TaskContext>::iterator i1, i2;
	std::list<TaskGroup::TaskContext> endlist;
	int result;
	bool found, shouldrun;

	SDL_LockMutex (contextListMutex);
	i1 = contextList.begin ();
	i2 = contextList.end ();
	while (i1 != i2) {
		shouldrun = false;
		if (! i1->thread) {
			if ((maxThreadCount <= 0) || (threadCount < maxThreadCount)) {
				if (i1->fn.queueId.empty ()) {
					shouldrun = true;
				}
				else {
					if (! queueIdMap.exists (i1->fn.queueId)) {
						queueIdMap.insert (i1->fn.queueId, true);
						shouldrun = true;
					}
				}
			}
		}
		if (shouldrun) {
			i1->isRunning = true;
			++threadCount;
			i1->thread = SDL_CreateThread (TaskGroup::executeTask, StdString::createSprintf ("TaskGroup::executeTask %i", nextThreadIndex).c_str (), (void *) &(*i1));
			++nextThreadIndex;
		}
		++i1;
	}
	while (true) {
		found = false;
		i1 = contextList.begin ();
		i2 = contextList.end ();
		while (i1 != i2) {
			if (i1->thread && (! i1->isRunning)) {
				found = true;
				SDL_WaitThread (i1->thread, &result);
				--threadCount;
				i1->thread = NULL;
				if (i1->endCallback.callback) {
					endlist.push_back (*i1);
				}
				if (! i1->fn.queueId.empty ()) {
					queueIdMap.remove (i1->fn.queueId);
				}
				contextList.erase (i1);
				runCount = (int) contextList.size ();
				break;
			}
			++i1;
		}
		if (! found) {
			break;
		}
	}
	SDL_UnlockMutex (contextListMutex);

	i1 = endlist.begin ();
	i2 = endlist.end ();
	while (i1 != i2) {
		i1->endCallback.callback (i1->endCallback.callbackData, i1->fn.fnData);
		++i1;
	}
}

void TaskGroup::stop () {
	isStopped = true;
}

bool TaskGroup::isStopComplete () {
	bool result;

	if (! isStopped) {
		return (false);
	}
	SDL_LockMutex (contextListMutex);
	result = contextList.empty ();
	SDL_UnlockMutex (contextListMutex);
	return (result);
}

void TaskGroup::waitThreads () {
	std::list<TaskGroup::TaskContext>::iterator i1, i2;
	int result;

	SDL_LockMutex (contextListMutex);
	i1 = contextList.begin ();
	i2 = contextList.end ();
	while (i1 != i2) {
		if (i1->thread) {
			SDL_WaitThread (i1->thread, &result);
			i1->thread = NULL;
			--threadCount;
		}
		++i1;
	}
	contextList.clear ();
	runCount = 0;
	SDL_UnlockMutex (contextListMutex);
}
