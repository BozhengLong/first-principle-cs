"use client";

import { Play, RotateCcw, Loader2, CheckCircle2, XCircle, Lightbulb } from "lucide-react";
import { useTranslations, useLocale } from "next-intl";
import { Button } from "@/components/ui/button";
import { ScrollArea } from "@/components/ui/scroll-area";
import { usePyodide } from "@/hooks/use-pyodide";
import { useWorkspace } from "@/contexts/workspace-context";
import { cn } from "@/lib/utils";
import { matchDiagnostics } from "@/lib/diagnostics/match-diagnostics";

interface TestRunnerProps {
  code: string;
  testCode: string;
  onReset: () => void;
}

export function TestRunner({ code, testCode, onReset }: TestRunnerProps) {
  const t = useTranslations("learn");
  const locale = useLocale() as "zh" | "en";
  const { status, running, results, rawOutput, runTests } = usePyodide();
  const { module, runVisualize } = useWorkspace();

  const diagnosticMap = matchDiagnostics(results, module.diagnostics);

  const isReady = status === "ready";
  const passed = results.filter((r) => r.passed).length;
  const total = results.length;

  const handleRunTests = async () => {
    await runTests(code, testCode);
    // Auto-visualize after test run
    runVisualize();
  };

  return (
    <div className="flex flex-col border-t">
      <div className="flex items-center gap-2 px-3 py-2">
        <Button
          size="sm"
          onClick={handleRunTests}
          disabled={!isReady || running}
          className="gap-1.5"
        >
          {running ? (
            <Loader2 className="h-3.5 w-3.5 animate-spin" />
          ) : (
            <Play className="h-3.5 w-3.5" />
          )}
          {running ? t("running") : t("runTests")}
        </Button>

        <Button variant="ghost" size="sm" onClick={onReset} className="gap-1.5">
          <RotateCcw className="h-3.5 w-3.5" />
          {t("resetCode")}
        </Button>

        {!isReady && status !== "idle" && status !== "error" && (
          <span className="flex items-center gap-1.5 text-xs text-muted-foreground">
            <Loader2 className="h-3 w-3 animate-spin" />
            {t(`pyodide_${status}`)}
          </span>
        )}

        {status === "error" && (
          <span className="text-xs text-destructive">{t("pyodideError")}</span>
        )}

        {total > 0 && (
          <span
            className={cn(
              "ml-auto text-xs font-medium",
              passed === total ? "text-green-600 dark:text-green-400" : "text-orange-600 dark:text-orange-400"
            )}
          >
            {t("testsPassed", { passed, total })}
          </span>
        )}
      </div>

      {results.length > 0 && (
        <ScrollArea className="max-h-48 border-t">
          <div className="space-y-0.5 p-2">
            {/* _allFail banner */}
            {diagnosticMap.has(0) && results.every((r) => !r.passed) && (
              <div className="mb-2 flex items-center gap-2 rounded-md bg-amber-50 px-3 py-2 text-xs text-amber-800 dark:bg-amber-950/40 dark:text-amber-300">
                <Lightbulb className="h-4 w-4 shrink-0" />
                <span>{diagnosticMap.get(0)!.message[locale]}</span>
              </div>
            )}
            {results.map((r, i) => (
              <div
                key={i}
                className="flex items-start gap-2 rounded px-2 py-1 text-xs"
              >
                {r.passed ? (
                  <CheckCircle2 className="mt-0.5 h-3.5 w-3.5 shrink-0 text-green-600 dark:text-green-400" />
                ) : (
                  <XCircle className="mt-0.5 h-3.5 w-3.5 shrink-0 text-red-600 dark:text-red-400" />
                )}
                <div className="min-w-0">
                  <span className="text-muted-foreground">{r.className}::</span>
                  <span className={r.passed ? "" : "text-red-600 dark:text-red-400"}>
                    {r.name}
                  </span>
                  {r.message && (
                    <pre className="mt-1 whitespace-pre-wrap text-[10px] text-muted-foreground">
                      {r.message}
                    </pre>
                  )}
                  {!r.passed && diagnosticMap.has(i) && !results.every((x) => !x.passed) && (
                    <div className="mt-1 flex items-center gap-1.5 rounded bg-amber-50 px-2 py-1 text-[11px] text-amber-800 dark:bg-amber-950/40 dark:text-amber-300">
                      <Lightbulb className="h-3 w-3 shrink-0" />
                      <span>{diagnosticMap.get(i)!.message[locale]}</span>
                    </div>
                  )}
                </div>
              </div>
            ))}
          </div>
        </ScrollArea>
      )}

      {rawOutput && results.length === 0 && (
        <ScrollArea className="max-h-48 border-t">
          <pre className="p-3 text-xs text-muted-foreground whitespace-pre-wrap">
            {rawOutput}
          </pre>
        </ScrollArea>
      )}
    </div>
  );
}
