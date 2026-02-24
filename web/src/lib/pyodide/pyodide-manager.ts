import {
  refInit,
  refLexer,
  refParser,
  refEnvironment,
  refSrcInit,
} from "@/data/tiny-interpreter/reference-implementations";

type PyodideStatus = "idle" | "loading" | "installing" | "ready" | "error";

let pyodideInstance: PyodideInterface | null = null;
let initPromise: Promise<void> | null = null;
let currentStatus: PyodideStatus = "idle";
let statusListeners: Set<(status: PyodideStatus) => void> = new Set();

function setStatus(status: PyodideStatus) {
  currentStatus = status;
  statusListeners.forEach((fn) => fn(status));
}

export function getStatus(): PyodideStatus {
  return currentStatus;
}

export function onStatusChange(fn: (status: PyodideStatus) => void) {
  statusListeners.add(fn);
  return () => { statusListeners.delete(fn); };
}

export async function initialize(): Promise<void> {
  if (pyodideInstance) return;
  if (initPromise) return initPromise;

  initPromise = (async () => {
    try {
      setStatus("loading");

      // Load Pyodide script from CDN
      if (typeof window !== "undefined" && !window.loadPyodide) {
        await new Promise<void>((resolve, reject) => {
          const script = document.createElement("script");
          script.src =
            "https://cdn.jsdelivr.net/pyodide/v0.27.0/full/pyodide.js";
          script.onload = () => resolve();
          script.onerror = () => reject(new Error("Failed to load Pyodide"));
          document.head.appendChild(script);
        });
      }

      pyodideInstance = await window.loadPyodide({
        indexURL: "https://cdn.jsdelivr.net/pyodide/v0.27.0/full/",
      });

      setStatus("installing");
      await pyodideInstance!.loadPackage("micropip");
      await pyodideInstance!.runPythonAsync(`
import micropip
await micropip.install("pytest")
`);

      setupReferenceImplementations(pyodideInstance!);

      setStatus("ready");
    } catch (e) {
      setStatus("error");
      initPromise = null;
      throw e;
    }
  })();

  return initPromise;
}

function setupReferenceImplementations(py: PyodideInterface) {
  // Create /home/pyodide/src/tiny_interpreter/ package in Pyodide's virtual FS
  // so `from src.tiny_interpreter.X import ...` resolves correctly.
  try { py.FS.mkdir("/home/pyodide/src"); } catch { /* exists */ }
  try { py.FS.mkdir("/home/pyodide/src/tiny_interpreter"); } catch { /* exists */ }

  py.FS.writeFile("/home/pyodide/src/__init__.py", refSrcInit);
  py.FS.writeFile("/home/pyodide/src/tiny_interpreter/__init__.py", refInit);
  py.FS.writeFile("/home/pyodide/src/tiny_interpreter/lexer.py", refLexer);
  py.FS.writeFile("/home/pyodide/src/tiny_interpreter/parser.py", refParser);
  py.FS.writeFile("/home/pyodide/src/tiny_interpreter/environment.py", refEnvironment);
}

export async function runTests(
  skeletonCode: string,
  testCode: string
): Promise<string> {
  if (!pyodideInstance) {
    throw new Error("Pyodide not initialized");
  }

  const py = pyodideInstance;

  // Write files to virtual FS
  py.FS.writeFile("/home/pyodide/skeleton.py", skeletonCode);
  py.FS.writeFile("/home/pyodide/test_skeleton.py", testCode);

  // Clear cached modules so re-runs pick up changes
  await py.runPythonAsync(`
import sys
for mod_name in list(sys.modules.keys()):
    if mod_name in ("skeleton", "test_skeleton") or mod_name.startswith("src."):
        del sys.modules[mod_name]
sys.path = [p for p in sys.path if "/home/pyodide" not in p]
sys.path.insert(0, "/home/pyodide")
`);

  // Run pytest and capture output
  const result = await py.runPythonAsync(`
import subprocess, io, sys

# Capture stdout/stderr
old_stdout = sys.stdout
old_stderr = sys.stderr
sys.stdout = io.StringIO()
sys.stderr = io.StringIO()

try:
    import pytest
    exit_code = pytest.main(["/home/pyodide/test_skeleton.py", "-v", "--tb=short", "--no-header"])
    output = sys.stdout.getvalue() + sys.stderr.getvalue()
except Exception as e:
    output = str(e)
finally:
    sys.stdout = old_stdout
    sys.stderr = old_stderr

output
`);

  return String(result);
}
