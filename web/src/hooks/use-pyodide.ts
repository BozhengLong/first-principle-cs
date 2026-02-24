"use client";

import { useCallback, useEffect, useRef, useState } from "react";
import type { TestResult } from "@/lib/pyodide/test-parser";
import { parseTestOutput } from "@/lib/pyodide/test-parser";

type PyodideStatus = "idle" | "loading" | "installing" | "ready" | "error";

export function usePyodide() {
  const [status, setStatus] = useState<PyodideStatus>("idle");
  const [running, setRunning] = useState(false);
  const [results, setResults] = useState<TestResult[]>([]);
  const [rawOutput, setRawOutput] = useState("");
  const managerRef = useRef<typeof import("@/lib/pyodide/pyodide-manager") | null>(null);

  const init = useCallback(async () => {
    if (status !== "idle" && status !== "error") return;
    try {
      const manager = await import("@/lib/pyodide/pyodide-manager");
      managerRef.current = manager;
      const unsub = manager.onStatusChange(setStatus);
      await manager.initialize();
      return unsub;
    } catch {
      setStatus("error");
    }
  }, [status]);

  // Start loading on mount
  useEffect(() => {
    let unsub: (() => void) | undefined;
    init().then((u) => { unsub = u; });
    return () => { unsub?.(); };
  }, []); // eslint-disable-line react-hooks/exhaustive-deps

  const runTests = useCallback(
    async (code: string, testCode: string) => {
      if (!managerRef.current) return;
      setRunning(true);
      setResults([]);
      setRawOutput("");
      try {
        const output = await managerRef.current.runTests(code, testCode);
        setRawOutput(output);
        setResults(parseTestOutput(output));
      } catch (e) {
        setRawOutput(String(e));
      } finally {
        setRunning(false);
      }
    },
    []
  );

  return { status, running, results, rawOutput, runTests };
}
