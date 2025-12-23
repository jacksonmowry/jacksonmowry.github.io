#!/usr/bin/env python3

import lldb
import os
import json
import re
import sys


def remove_ansi_escape_sequences(text):
    ansi_escape = re.compile(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])")
    return ansi_escape.sub("", text)


class Program:
    def __init__(self, source, steps):
        self.source = source
        self.steps = steps

    def toJSON(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)


class Step:
    def __init__(self, stack_frames, function_signatures, line, locals, args):
        self.stack_frames = [
            remove_ansi_escape_sequences(x.__repr__()) for x in stack_frames
        ]
        self.function_signatures = [
            remove_ansi_escape_sequences(x.__repr__()) for x in function_signatures
        ]
        self.line = line.__repr__()
        self.locals = [x.__repr__() for x in locals]
        self.args = [x.__repr__() for x in args]


# Set the path to the executable to debug
exe = "/tmp/a.out"
source = "/tmp/example.c"

source_text = ""
with open(source, "r") as file:
    source_text = file.read()

# Create a new debugger instance
debugger = lldb.SBDebugger.Create()

# When we step or continue, don't return from the function until the process
# stops. Otherwise we would have to handle the process events ourselves which, while doable is
# a little tricky.  We do this by setting the async mode to false.
debugger.SetAsync(False)

# Create a target from a file and arch
# print("Creating a target for '%s'" % exe)

target = debugger.CreateTargetWithFileAndArch(exe, lldb.LLDB_ARCH_DEFAULT)

steps = []
if target:
    # If the target is valid set a breakpoint at main
    main_bp = target.BreakpointCreateByName(
        "main", target.GetExecutable().GetFilename()
    )

    # Launch the process. Since we specified synchronous mode, we won't return
    # from this function until we hit the breakpoint at main
    process = target.LaunchSimple(None, None, os.getcwd())
    while True:
        if process.threads[0].frames[0].GetLineEntry().file.fullpath != source:
            process.threads[0].StepOut()
        if "main" not in [x.name for x in process.threads[0].frames]:
            break

        stack_frames = []
        function_signatures = []
        for frame in process.threads[0].frames:
            stack_frames.append(frame)
            function_signatures.append(frame.function.type)
            if frame.function.name == "main":
                break

        step = Step(
            stack_frames,
            function_signatures,
            process.threads[0].frames[0].GetLineEntry(),
            list(process.threads[0].frames[0].locals),
            list(process.threads[0].frames[0].args),
        )

        steps.append(step)

        # Attempt to StepInto at all times
        process.threads[0].StepInto()


program = Program(source_text, steps)
print(program.toJSON())
