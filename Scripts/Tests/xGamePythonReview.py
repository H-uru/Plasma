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
from pprint import pp
import sys
import traceback
from typing import *

# Environment variables are being used so that you can copy this outside of the workflow
# and trivually run the tests yourself.
input_path = Path(f"{os.environ['PLASMA_PATH']}/Scripts/Python")
output_path = Path(os.environ['PLASMA_PR_OUTPUT_PATH'])
output_path.mkdir(parents=True, exist_ok=True)

# Setup PYTHONPATH like Plasma does in internal builds.
sys.path.append(str(input_path))
sys.path.append(str(input_path.joinpath("plasma")))

# Results
import_errors: Dict[str, str] = {}
init_errors: Dict[str, str] = {}
found_ids: Dict[int, Set[str]] = defaultdict(set)
missing_ids: List[str] = []

def dump_exc(what: str) -> str:
  # Because format_exc() and print_exc() don't give the same result.
  buf = io.StringIO()
  traceback.print_exc(file=buf)
  result = buf.getvalue().rstrip()
  print(f"... Blast! Error during {what}:", result, sep="\n")
  return result

def dump_output():
  success = True
  with output_path.joinpath("pr_body.html").open("w") as out_file:
    if import_errors:
      out_file.write("<details>\n")
      out_file.write("  <summary><strong>All Python files must import correctly.</strong></summary>\n")
      for file, e in import_errors.items():
        out_file.write( "  <details>\n")
        out_file.write(f"    <summary>{file}</summary>\n")
        out_file.write(f"    <pre>{e}</pre>\n")
        out_file.write( "  </details>\n")
      out_file.write("</details>")
      success = False

    if init_errors:
      out_file.write("<details>\n")
      out_file.write("  <summary><strong>All PythonFileMods must initialize correctly.</strong></summary>\n")
      for file, e in init_errors.items():
        out_file.write( "  <details>\n")
        out_file.write(f"    <summary>{file}</summary>\n")
        out_file.write(f"    <pre>{e}</pre>\n")
        out_file.write( "  </details>\n")
      out_file.write("</details>")
      success = False

    dupe_ids = get_duplicated_ids()
    if missing_ids or dupe_ids:
      out_file.write("<details>\n")
      out_file.write("  <summary><strong>All PythonFileMods must have a unique <tt>self.id</tt></strong></summary>\n")
      if missing_ids:
        out_file.write("  <details>\n")
        out_file.write("    <summary>Python files missing <tt>self.id</tt>:</summary>\n")
        out_file.write("    <ul>\n")
        out_file.writelines([f"      <li>{i}</li>" for i in missing_ids])
        out_file.write("    </ul>\n")
        out_file.write("  </details>\n")
      if dupe_ids:
        out_file.write("  <details>\n")
        out_file.write("    <summary>Python files with duplicated <tt>self.id</tt>s:</summary>\n")
        out_file.write("    <ul>\n")
        out_file.writelines([f"      <li><strong>{clsid}</strong>: {', '.join((f'<tt>{i}</tt>' for i in filenames))}</li>" for clsid, filenames in dupe_ids.items()])
        out_file.write("    </ul>\n")
        out_file.write("  </details>\n")
      out_file.write("</details>\n")
      success = False

  with output_path.joinpath("pr_event").open("w") as out_file:
    out_file.write("APPROVE" if success else "REQUEST_CHANGES")

def get_duplicated_ids() -> Dict[int, Sequence[str]]:
  return { selfid: files for selfid, files in found_ids.items() if len(files) > 1 }

# Try to import PlasmaTypes first. If that fails, we have big problems.
try:
  print("--- Importing PlasmaTypes by force ---")
  import PlasmaTypes
except Exception:
  import_errors["plasma/PlasmaTypes.py"] = dump_exc("import")
  dump_output()
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
    import_errors[py_file.name] = dump_exc("import")
  else:
    modcls = getattr(mod, py_file.stem, None)
    if modcls is not None and inspect.isclass(modcls) and issubclass(modcls, PlasmaTypes.ptModifier):
      print("It's a PythonFileMod(TM)!")
      try:
        modobj = modcls()
      except Exception:
        init_errors[py_file.name] = dump_exc(f"instantiating {py_file.stem}")
      else:
        if clsid := getattr(modobj, "id"):
          found_ids[clsid].add(py_file.name)
        else:
          missing_ids.append(py_file.name)
    else:
      print("It's an import file!")

dupe_ids = get_duplicated_ids()
if dupe_ids:
  print("--- Duplicate IDs Found: ---")
  pp(dupe_ids)

# Finalize everybody.
dump_output()
