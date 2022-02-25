# -*- coding: utf-8 -*-
""" *==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

 *==LICENSE==* """

from __future__ import annotations
from collections import defaultdict
import importlib.util
import inspect
import io
import os
from pathlib import Path
import sys
import traceback
from typing import *

# Environment variables are being used so that you can copy this outside of the workflow
# and trivually run the tests yourself.
repo_path = Path(os.environ['PLASMA_PATH'])
input_path = repo_path.joinpath("Scripts", "Python")

# Setup PYTHONPATH like Plasma does in internal builds.
sys.path.append(str(input_path))
sys.path.append(str(input_path.joinpath("plasma")))

# Results
found_ids: Dict[int, Set[Path]] = defaultdict(set)
exitcode = 0

# Baseline format string for a parsable thingy.
result_str = "---{status} {file}:{line} {message}---"

def dump_exc(what: str) -> str:
    global exitcode
    exitcode = 1

    # Because format_exc() and print_exc() don't give the same result.
    buf = io.StringIO()
    traceback.print_exc(file=buf)
    result = buf.getvalue().rstrip()
    print(f"... Blast! Error during {what}:", result, sep="\n")

    # Parsable result
    exc_type, exc_value, exc_tb = sys.exc_info()

    # For some reason, Python 3.8 uses the name of THIS file first, so let's just walk the trace
    # until we get something inside the scripts path.
    for frame, line in traceback.walk_tb(exc_tb):
        naughty_path = Path(frame.f_code.co_filename)
        if not naughty_path.is_absolute():
            naughty_path = repo_path.joinpath(naughty_path)

        # Python 3.8 lacks is_relative_to(). dang, so fake it with this goofy construct.
        try:
            naughty_path.relative_to(input_path)
        except ValueError:
            continue
        else:
            if naughty_path.exists():
                break

    try:
        error_path = naughty_path.relative_to(repo_path)
    except ValueError:
        print("Crap, I couldn't figure this one out!\n")
        sys.exit(1)

    print(
        result_str.format(
            status="ERROR",
            file=error_path,
            line=line,
            message=f"{exc_type.__name__}: {exc_value}"
        )
    )

    return result

def dump_missing_id(filename: Path):
    global exitcode
    exitcode = 1

    print(
        result_str.format(
            status="WARNING",
            file=filename.relative_to(repo_path),
            line="",
            message="PythonFileMod(TM) missing an `id` attribute"
        )
    )

def dump_dupe_id():
    global exitcode

    for id, dupe_files in get_duplicated_ids().items():
        exitcode = 1
        for py_file in dupe_files:
            other_dupes = dupe_files - set((py_file,))
            print(
                result_str.format(
                    status="WARNING",
                    file=py_file.relative_to(repo_path),
                    line="",
                    message=f"PythonFileMod(TM) has the same `id` ({id}) as: {','.join((i.name for i in other_dupes))}"
                )
            )

def get_duplicated_ids() -> Dict[int, Set[Path]]:
    return { selfid: files for selfid, files in found_ids.items() if len(files) > 1 }

# Try to import PlasmaTypes first. If that fails, we have big problems.
try:
    print("--- Importing PlasmaTypes by force ---")
    import PlasmaTypes
except Exception:
    dump_exc("import")
    sys.exit(0)
else:
    print("... Woo-hoo, it worked!")

# Run through all of the Python scripts, logging errors and taking names.
for py_file in input_path.glob("*.py"):
    print(f"--- Importing {py_file.name} ---")
    try:
        # This is slightly verbose but it ensures that we import only the Plasma
        # python scripts, which roughly matches the behavior of Plasma itself -- especially
        # when using an external client.
        spec = importlib.util.spec_from_file_location(py_file.stem, py_file)
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
    except Exception:
        dump_exc("import")
    else:
        modcls = getattr(mod, py_file.stem, None)
        if modcls is not None and inspect.isclass(modcls) and issubclass(modcls, PlasmaTypes.ptModifier):
            print("It's a PythonFileMod(TM)!")
            try:
                modobj = modcls()
            except Exception:
                dump_exc(f"instantiating {py_file.stem}")
            else:
                if clsid := getattr(modobj, "id", None):
                    found_ids[clsid].add(py_file)
                else:
                    dump_missing_id(py_file)
        else:
            print("It's an import file!")

dump_dupe_id()
sys.exit(exitcode)
