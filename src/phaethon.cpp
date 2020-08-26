/* Phaethon - A FLOSS resource explorer for BioWare's Aurora engine games
 *
 * Phaethon is the legal property of its developers, whose names
 * can be found in the AUTHORS file distributed with this source
 * distribution.
 *
 * Phaethon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * Phaethon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Phaethon. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file
 *  The project's main entry point.
 */

#include "src/common/platform.h"

#ifdef WIN32
	#include <windows.h>
#endif

#include <cstdio>

#include <QApplication>

#include "src/version/version.h"

#include "src/common/util.h"
#include "src/common/error.h"
#include "src/common/ustring.h"

#include "src/gui/icons.h"
#include "src/gui/mainwindow.h"

#include "src/sound/sound.h"

#include "src/cline.h"

void initPlatform();

void openGamePath(const Common::UString &path);

int main(int argc, char **argv) {
	initPlatform();

	std::vector<Common::UString> args;
	Common::Platform::getParameters(argc, argv, args);

	try {
		// Find out what we're supposed to do
		Job job = parseCommandLine(args);

		// Handle the job
		switch (job.operation) {
			case kOperationHelp:
				std::printf("%s\n", createHelpText(args[0]).c_str());
				break;

			case kOperationVersion:
				std::printf("%s\n", createVersionText().c_str());
				break;

			case kOperationPath:
				openGamePath(job.path);
				break;

			case kOperationInvalid:
			default:
				std::printf("%s\n", createHelpText(args[0]).c_str());
				return 1;
		}
	} catch (Common::Exception &e) {
		Common::printException(e);
		return 2;
	} catch (std::exception &e) {
		Common::Exception se(e);

		Common::printException(se);
		return 2;
	}

	return 0;
}

void initPlatform() {
	try {
		Common::Platform::init();
	} catch (Common::Exception &e) {
		e.add("Failed to initialize the low-level platform-specific subsytem");

		Common::printException(e);
		std::exit(1);
	}
}


class Phaethon {
public:
	Phaethon(const Common::UString &path = "");
	~Phaethon();

private:
	Common::UString _path;

	void initSubsystems();
	void deinitSubsystems();
};

Phaethon::Phaethon(const Common::UString &path) : _path(path) {
	initSubsystems();

	int argc = 1; // QApplication requires argc to be at least 1
	char empty[] = ""; // Silence -Wwrite-string warning
	char *argv[] = {empty}; // QApplication requires argv to be at least 1

	QApplication app(argc, argv);

	app.setWindowIcon(GUI::createWindowIcon());

	GUI::MainWindow mainWindow(0, Version::getProjectNameVersion(), QSize(800, 600), path.c_str());
	mainWindow.show();

	app.exec();
}

Phaethon::~Phaethon() {
	deinitSubsystems();
}

void Phaethon::initSubsystems() {
	try {
		SoundMan.init();
	} catch (Common::Exception &e) {
		e.add("Failed to initialize subsystems");

		Common::printException(e);
		std::exit(1);
	}
}

void Phaethon::deinitSubsystems() {
	try {
		SoundMan.deinit();

		Sound::SoundManager::destroy();
	} catch (Common::Exception &e) {
		e.add("Failed to deinitialize subsystems");

		Common::printException(e, "WARNING: ");
	}
}

void openGamePath(const Common::UString &path) {
	Phaethon phaethon(path);
}

#ifdef WIN32
#ifdef UNICODE
	int WINAPI wWinMain(HINSTANCE UNUSED(hInstance), HINSTANCE UNUSED(hPrevInstance), PWSTR UNUSED(pCmdLine), int UNUSED(nCmdShow)) {
		return main(0, 0);
	}
#else
	int WINAPI WinMain(HINSTANCE UNUSED(hInstance), HINSTANCE UNUSED(hPrevInstance), PSTR UNUSED(pCmdLine), int UNUSED(nCmdShow)) {
		return main(0, 0);
	}
#endif
#endif
