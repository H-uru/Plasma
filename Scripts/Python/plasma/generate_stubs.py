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

import os.path
import types

from typing import Iterable, Literal

import Plasma
import PlasmaConstants
import PlasmaGame
import PlasmaGameConstants
import PlasmaNetConstants
import PlasmaVaultConstants

all_plasma_modules = [
    Plasma,
    PlasmaConstants,
    PlasmaGame,
    PlasmaGameConstants,
    PlasmaNetConstants,
    PlasmaVaultConstants,
]

docstring_params_prefix = "Params: "

def attr_sort_key(item: tuple[str, object]) -> tuple[int, str]:
    name, value = item

    # Put functions, classes, and enums after "simple" attributes (constants and instance properties).
    # (Note: callable includes both functions and classes).
    if callable(value) or isinstance(value, PlasmaConstants.Enum):
        order = 1
    else:
        order = 0

    # Generally, we sort attributes by name (case-sensitive).
    return order, name

def iter_attributes(obj: object) -> Iterable[tuple[str, object]]:
    # Note: dir also includes class attributes inherited from superclasses!
    # We currently want this, to match the existing stubs,
    # but in the future we should switch to iterating over __dict__ instead
    # so that only directly defined attributes are included.
    # TODO Replace with __dict__ and manually add inherited methods
    attrs = [(name, getattr(obj, name)) for name in dir(obj)]
    attrs.sort(key=attr_sort_key)

    # Ensure that every class has its base classes defined first.
    # We manage the iteration index manually
    # because we intentionally process some indices more than once
    # (and also because we reorder items during iteration).
    already_seen_classes = set()
    i = 0
    while i < len(attrs):
        name, value = attrs[i]

        # Ignore dunder attributes for the purposes of discovering base class dependencies.
        # Otherwise, the logic will get confused by special attributes
        # whose value is a type object, such as __class__.
        if not name.startswith("__") and isinstance(value, type):
            # Find all base classes that haven't been seen yet
            # and wrap them in (name, value) tuples.
            missing_bases = [
                (base.__name__, base) for base in value.__bases__
                # Only check base classes from the same module.
                # Classes from other modules are accessed by their fully qualified name
                # and are assumed to be always defined already.
                if base.__module__ == value.__module__ and base not in already_seen_classes
            ]

            if missing_bases:
                # Remove all of the missing base classes from their original list position...
                for missing_base_item in missing_bases:
                    attrs.remove(missing_base_item)
                # (...verify that we didn't somehow remove anything at or before index i...)
                assert attrs[i] == (name, value)
                # ...and re-add them right before the current class.
                attrs[i:i] = missing_bases

                # We intentionally continue WITHOUT incrementing i here!
                # The next iteration must process the classes that we just moved around,
                # so that if the bases themselves have any missing bases,
                # those are handled recursively.
                # Iteration continues normally once the class at index i has no more missing bases.
                continue
            else:
                # All base classes are already defined - nothing more to be done.
                already_seen_classes.add(value)

        i += 1

    return attrs

def parse_params_from_doc(doc: str) -> tuple[str, str]:
    if doc is None:
        return "", ""
    elif doc.startswith(docstring_params_prefix):
        params_line, _, doc_body = doc.partition("\n")
        params = params_line.removeprefix(docstring_params_prefix)
        return params, doc_body
    else:
        # Assume that functions without a "Params: " line have no parameters.
        # Safer would be to default to "*args, **kwargs" and require some explicit marker for no parameters,
        # but this would require extensive manual updating of the docstrings.
        return "", doc

def format_qualified_name(cls: type, context_module_name: str) -> str:
    if cls.__module__ in {"builtins", context_module_name}:
        return cls.__qualname__
    else:
        return cls.__module__ + "." + cls.__qualname__

def add_indents(indent: str, lines: Iterable[str]) -> Iterable[str]:
    for line in lines:
        if line:
            yield indent + line
        else:
            # Don't add extra whitespace on blank lines.
            yield ""

FunctionKind = Literal["function", "method", "classmethod", "staticmethod", "property"]

def generate_function_stub(kind: FunctionKind, name: str, params: str, doc: str) -> Iterable[str]:
    if kind == "function":
        decorator = None
        self_param = None
    elif kind == "method":
        decorator = None
        self_param = "self"
    elif kind == "classmethod":
        decorator = "@classmethod"
        self_param = "cls"
    elif kind == "staticmethod":
        decorator = "@staticmethod"
        self_param = None
    elif kind == "property":
        decorator = "@property"
        self_param = "self"
    else:
        raise ValueError(f"Unsupported function kind: {kind!r}")

    all_params_parts = []
    if self_param is not None:
        all_params_parts.append(self_param)
    if params:
        all_params_parts.append(params)
    # No space after the comma for now, to match the existing stubs.
    all_params = ",".join(all_params_parts)

    if decorator is not None:
        yield decorator
    yield f"def {name}({all_params}):"
    if doc:
        yield f'    """{doc}"""'
    yield "    pass"

def generate_enum_stub(name: str, enum_obj: PlasmaConstants.Enum) -> Iterable[str]:
    yield f"class {name}:"
    # Output the string "(none)" as the docstring for all enums for now, to match the existing stubs.
    # (Plasma enums don't have docstrings.)
    yield '    """(none)"""'

    # Output enum constants sorted by their int value.
    for name, value in sorted(enum_obj.lookup.items(), key=lambda item: int(item[1])):
        yield f"    {name} = {int(value)}"

def generate_class_stub(name: str, cls: type) -> Iterable[str]:
    if tuple(cls.__bases__) == (object,):
        class_parens = ""
    else:
        base_names = []
        for base in cls.__bases__:
            base_names.append(format_qualified_name(base, cls.__module__))

        class_parens = "(" + ", ".join(base_names) + ")"

    init_params, doc = parse_params_from_doc(cls.__doc__)

    yield f"class {name}{class_parens}:"
    if doc:
        yield f'    """{doc}"""'

    first = True
    # This currently also includes attributes inherited from superclasses, to match the existing stubs.
    # We should change this to only include attributes defined in the class itself.
    # This would make stubs for subclasses a lot more readable.
    for name, value in iter_attributes(cls):
        if name == "__init__":
            # Don't put a blank line between the class docstring and __init__, to match the existing stubs.
            if not first:
                yield ""
            first = False

            # Special case for __init__: use the parameters from the class docstring.
            # Output the string "None" as the docstring for all __init__ methods for now, to match the existing stubs.
            yield from add_indents("    ", generate_function_stub("method", name, init_params, "None"))
            continue
        elif name.startswith("__"):
            # Ignore all other special attributes.
            continue

        first = False
        yield ""

        if isinstance(value, type):
            yield from add_indents("    ", generate_class_stub(name, value))
        elif callable(value):
            kind: FunctionKind
            if isinstance(value, types.BuiltinMethodType):
                # C-defined non-instance methods have class builtin_function_or_method (aka types.BuiltinMethodType).
                # This class is used for both static methods and class methods.
                # It seems that we can tell them apart by looking at __self__,
                # though this seems like an implementation detail and potentially brittle.

                # If this breaks at some point,
                # I guess we could just treat all non-instance methods as static methods
                # (from a caller's perspective, static methods and class methods are almost indistinguishable anyway).

                # A more reliable way might be to look at the class __dict__,
                # where class methods show up as classmethod_descriptor (aka types.ClassMethodDescriptorType) objects
                # and static methods as staticmethod objects.

                if getattr(value, "__self__", None) is None:
                    kind = "staticmethod"
                else:
                    kind = "classmethod"
            else:
                # C-defined instance methods have two possible classes:
                # either wrapper_descriptor (aka types.WrapperDescriptorType) for "special" methods (e. g. __init__),
                # or method_descriptor (aka types.MethodDescriptorType) for "regular" methods.
                # We don't need to care about these implementation details though
                # and simply assume that all other callables are instance methods.
                kind = "method"

            params, doc = parse_params_from_doc(value.__doc__)
            yield from add_indents("    ", generate_function_stub(kind, name, params, doc))
        elif isinstance(value, types.GetSetDescriptorType):
            # C-defined properties have class getset_descriptor (aka types.GetSetDescriptorType).
            # (Note: There is also member_descriptor aka types.MemberDescriptorType,
            # but we don't need to care about that, because Plasma doesn't use PyMemberDef.)
            # We currently can't determine whether a C-defined property is read-only or read-write,
            # because the Python glue implements read-only properties by defining a setter that always throws an error.
            # (Perhaps we should set the setter to nullptr in that case? Perhaps Python can tell the difference then?)
            # Right now it doesn't matter anyway,
            # because the type annotation syntax doesn't differentiate between read-only and read-write attributes.
            doc = value.__doc__
            yield f"    {name}: Any"
            if doc:
                yield f'    """{doc}"""'
        else:
            yield f"    {name}: {format_qualified_name(type(value), cls.__module__)} = ... # = {value!r}"

def generate_module_stub(module: types.ModuleType) -> Iterable[str]:
    yield "# -*- coding: utf-8 -*-"

    # License header (copied from the stub generator script, *not* from the module being dumped):
    yield f'"""{__doc__}"""'

    yield ""
    yield "# NOTE: This stub file was generated automatically from Plasma's Python interface."
    yield "# Do not edit this file manually."
    yield "# To change any of the docstrings or function signatures,"
    yield "# edit the corresponding C++ glue code in pfPython."
    yield "# If the Python interface has changed, regenerate these stubs"
    yield "# by running the following call in the in-game Python console:"
    yield '# >>> __import__("generate_stubs").run()'
    yield ""

    # Now the actual module docstring (if one exists).
    # Temporarily disabled, because the docstring is currently occupied by the license header,
    # so a string literal placed here would be considered a regular statement and not a docstring.
    # This is also important for the __future__ import,
    # which loses its effect if preceded by any statement other than a docstring.
    if False and getattr(module, "__doc__", None) is not None:
        yield f'"""{module.__doc__}"""'
        yield ""

    # Hardcoded imports for type annotations:
    yield "from __future__ import annotations"
    yield "from typing import *"

    for name, value in iter_attributes(module):
        if name.startswith("__"):
            continue

        yield ""
        if isinstance(value, PlasmaConstants.Enum):
            yield from generate_enum_stub(name, value)
        elif isinstance(value, type):
            yield from generate_class_stub(name, value)
        elif callable(value):
            params, doc = parse_params_from_doc(value.__doc__)
            yield from generate_function_stub("function", name, params, doc)
        else:
            yield f"{name}: {format_qualified_name(type(value), module.__name__)} = ... # = {value!r}"

def run(dest_dir: str = "plasma_stubs_generated") -> None:
    try:
        os.mkdir(dest_dir)
    except FileExistsError:
        pass

    # Note: Keep this list in sync with the IInitBuiltinModule calls
    # in PythonInterface::initPython in pfPython/cyPythonInterface.cpp!
    for module in all_plasma_modules:
        # Generate all the lines first and only write the file afterwards
        # to avoid leaving behind a half-written file if generate_module_stub throws an exception.
        lines = list(generate_module_stub(module))
        stub_file = os.path.join(dest_dir, module.__name__ + ".py")
        with open(stub_file, "w", encoding="utf-8", newline="\n") as f:
            for line in lines:
                f.write(line + "\n")
