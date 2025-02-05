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
#include "Buffer.h"
#include "OsUtil.h"
#include "SubtitleReader.h"

// stage values
constexpr const int IndexLine = 0;
constexpr const int TimestampLine = 1;
constexpr const int TimestampLineH1 = 2;
constexpr const int TimestampLineM1 = 3;
constexpr const int TimestampLineS1 = 4;
constexpr const int TimestampLineMs1 = 5;
constexpr const int TimestampLineSeparator = 6;
constexpr const int TimestampLineH2 = 7;
constexpr const int TimestampLineM2 = 8;
constexpr const int TimestampLineS2 = 9;
constexpr const int TimestampLineMs2 = 10;
constexpr const int TextLine = 11;

SubtitleReader::SubtitleReader ()
: endTime (0)
, stage (IndexLine)
, parseStartTime (-1)
, parseEndTime (-1)
{
}
SubtitleReader::~SubtitleReader () {
}

void SubtitleReader::setFilePath (const StdString &filePathValue) {
	if (filePath.equals (filePathValue)) {
		return;
	}
	filePath.assign (filePathValue);
	entryList.clear ();
	endTime = 0;
}

OpResult SubtitleReader::readSubtitles () {
	OpResult result;

	if (filePath.empty ()) {
		return (OpResult::InvalidStateError);
	}
	if (OsUtil::getFileType (filePath) != OsUtil::RegularFile) {
		return (OpResult::FileOpenFailedError);
	}
	entryList.clear ();
	endTime = 0;
	stage = IndexLine;
	parseStartTime = -1;
	parseEndTime = -1;
	parseText.assign ("");
	result = OsUtil::readFileLines (filePath, SubtitleReader::parseLine, this);
	if (result == OpResult::Success) {
		if (stage == TextLine) {
			if ((parseStartTime >= 0) && (parseEndTime >= 0) && (! parseText.empty ())) {
				entryList.push_back (SubtitleReader::Entry (parseStartTime, parseEndTime, parseText));
				if (parseEndTime > endTime) {
					endTime = parseEndTime;
				}
			}
		}
	}
	return (result);
}
OpResult SubtitleReader::parseLine (void *itPtr, const StdString &line) {
	return (((SubtitleReader *) itPtr)->executeParseLine (line));
}
OpResult SubtitleReader::executeParseLine (const StdString &line) {
	StdString text;
	int h1, m1, s1, ms1, h2, m2, s2, ms2, linepos, linelen, fieldpos;
	char c;

	if (stage == IndexLine) {
		if (line.trimmed ().parsedInt (-1) >= 0) {
			stage = TimestampLine;
		}
	}
	else if (stage == TimestampLine) {
		h1 = -1;
		m1 = -1;
		s1 = -1;
		ms1 = -1;
		h2 = -1;
		m2 = -1;
		s2 = -1;
		ms2 = -1;
		stage = TimestampLineH1;
		text = line.trimmed ();
		linelen = (int) text.length ();
		linepos = 0;
		fieldpos = -1;
		while (linepos < linelen) {
			c = text.at (linepos);
			if (stage == TimestampLineH1) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else if (! isspace (c)) {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (c == ':') {
						h1 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (h1 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineM1;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineM1) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (c == ':') {
						m1 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (m1 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineS1;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineS1) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (c == ',') {
						s1 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (s1 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineMs1;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineMs1) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (isspace (c)) {
						ms1 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (ms1 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineSeparator;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineSeparator) {
				if (fieldpos < 0) {
					if (c == '-') {
						fieldpos = linepos;
					}
					else if (! isspace (c)) {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (isspace (c)) {
						if (! StdString (text.substr (fieldpos, linepos - fieldpos)).equals ("-->")) {
							return (OpResult::MalformedDataError);
						}
						fieldpos = -1;
						stage = TimestampLineH2;
					}
				}
			}
			else if (stage == TimestampLineH2) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else if (! isspace (c)) {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (c == ':') {
						h2 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (h2 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineM2;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineM2) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (c == ':') {
						m2 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (m2 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineS2;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineS2) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (c == ',') {
						s2 = StdString (text.substr (fieldpos, linepos - fieldpos)).parsedInt (-1);
						fieldpos = -1;
						if (s2 < 0) {
							return (OpResult::MalformedDataError);
						}
						stage = TimestampLineMs2;
					}
					else if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
				}
			}
			else if (stage == TimestampLineMs2) {
				if (fieldpos < 0) {
					if (isdigit (c)) {
						fieldpos = linepos;
					}
					else if (! isspace (c)) {
						return (OpResult::MalformedDataError);
					}
				}
				else {
					if (! isdigit (c)) {
						return (OpResult::MalformedDataError);
					}
					if (linepos == (linelen - 1)) {
						ms2 = StdString (text.substr (fieldpos, linepos - fieldpos + 1)).parsedInt (-1);
						fieldpos = -1;
						if (ms2 < 0) {
							return (OpResult::MalformedDataError);
						}
						parseStartTime = SubtitleReader::getEntryTime (h1, m1, s1, ms1);
						parseEndTime = SubtitleReader::getEntryTime (h2, m2, s2, ms2);
						if ((parseStartTime < 0) || (parseEndTime < 0)) {
							return (OpResult::MalformedDataError);
						}
						stage = TextLine;
					}
				}
			}
			++linepos;
		}
	}
	else if (stage == TextLine) {
		text = line.trimmed ();
		if (! text.empty ()) {
			text.markupStrip ("<", ">");
			text.markupStrip ("{", "}");
			if (! text.empty ()) {
				if (! parseText.empty ()) {
					parseText.append ("\n");
				}
				parseText.append (text);
			}
		}
		else {
			if (! parseText.empty ()) {
				entryList.push_back (SubtitleReader::Entry (parseStartTime, parseEndTime, parseText));
				if (parseEndTime > endTime) {
					endTime = parseEndTime;
				}
			}
			parseStartTime = -1;
			parseEndTime = -1;
			parseText.assign ("");
			stage = IndexLine;
		}
	}
	return (OpResult::Success);
}

int SubtitleReader::findEntry (int64_t seekTime) {
	std::vector<SubtitleReader::Entry>::const_iterator i1, i2;
	int count;

	if (entryList.empty () || (seekTime < 0) || (seekTime > endTime)) {
		return (-1);
	}
	count = 0;
	i1 = entryList.cbegin ();
	i2 = entryList.cend ();
	while (i1 != i2) {
		if ((seekTime >= i1->startTime) && (seekTime <= i1->endTime)) {
			return (count);
		}
		++count;
		++i1;
	}
	return (-1);
}

int64_t SubtitleReader::getEntryTime (int h, int m, int s, int ms) {
	int64_t t;

	if ((h < 0) || (m < 0) || (m > 59) || (s < 0) || (s > 59) || (ms < 0) || (ms > 999)) {
		return (-1);
	}
	t = ((int64_t) h) * (3600 * 1000);
	t += m * (60 * 1000);
	t += s * 1000;
	t += ms;
	return (t);
}
