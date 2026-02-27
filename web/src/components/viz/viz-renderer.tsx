"use client";

import { useMemo } from "react";
import { useTranslations } from "next-intl";
import type { VizData } from "@/types/viz";
import type { VizType } from "@/types/viz";
import { TokenStream } from "./token-stream";
import { ASTTree } from "./ast-tree";
import { EnvChain } from "./env-chain";
import { ClosureViz } from "./closure-viz";
import { StepControls } from "./step-controls";
import { useWorkspace } from "@/contexts/workspace-context";

interface VizRendererProps {
  vizType: VizType;
  data: VizData;
}

export function VizRenderer({ vizType, data }: VizRendererProps) {
  const t = useTranslations("viz");
  const { currentStep } = useWorkspace();

  // Compute current step data
  const stepData = useMemo(() => {
    if (!data.trace || data.trace.length === 0) {
      return {
        hasTrace: false,
        currentNodeId: undefined,
        executedNodeIds: new Set<string>(),
        currentMessage: undefined,
        currentEnv: data.environments,
        highlightedBinding: undefined,
        lookupPath: undefined,
        newFrameId: undefined,
      };
    }

    const trace = data.trace;
    const step = trace[Math.min(currentStep, trace.length - 1)];
    const executedNodeIds = new Set(
      trace.slice(0, currentStep + 1)
        .map((s) => s.nodeId)
        .filter((id): id is string => id !== undefined)
    );

    return {
      hasTrace: true,
      currentNodeId: step.nodeId,
      executedNodeIds,
      currentMessage: step.message,
      currentEnv: step.env ? [step.env] : data.environments,
      highlightedBinding: undefined, // TODO: extract from step
      lookupPath: undefined, // TODO: extract from step
      newFrameId: step.type === "call" ? step.env?.id : undefined,
    };
  }, [data, currentStep]);

  if (data.error) {
    return (
      <div className="rounded-md border border-red-200 bg-red-50 p-3 dark:border-red-900 dark:bg-red-950/30">
        <p className="text-xs font-medium text-red-600 dark:text-red-400">
          {t("error")}
        </p>
        <pre className="mt-1 text-xs font-mono text-red-800 dark:text-red-300 whitespace-pre-wrap">
          {data.error}
        </pre>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Step controls - only show if we have trace data */}
      {stepData.hasTrace && data.trace && (
        <StepControls
          totalSteps={data.trace.length}
          currentMessage={stepData.currentMessage}
        />
      )}

      {data.tokens && (
        <section>
          <h3 className="mb-2 text-xs font-semibold uppercase tracking-wider text-muted-foreground">
            Tokens
          </h3>
          <TokenStream tokens={data.tokens} input={data.input} />
        </section>
      )}

      {data.ast && (vizType === "ast" || vizType === "evaluator" || vizType === "closure") && (
        <section>
          <h3 className="mb-2 text-xs font-semibold uppercase tracking-wider text-muted-foreground">
            AST
          </h3>
          <ASTTree
            ast={data.ast}
            currentNodeId={stepData.currentNodeId}
            executedNodeIds={stepData.executedNodeIds}
          />
        </section>
      )}

      {data.environments && (vizType === "environment" || vizType === "evaluator" || vizType === "closure") && (
        <section>
          <h3 className="mb-2 text-xs font-semibold uppercase tracking-wider text-muted-foreground">
            Environment
          </h3>
          <EnvChain
            environments={stepData.currentEnv ?? data.environments}
            evalResult={data.evalResult}
            highlightedBinding={stepData.highlightedBinding}
            lookupPath={stepData.lookupPath}
            newFrameId={stepData.newFrameId}
          />
        </section>
      )}

      {vizType === "closure" && data.closures && (
        <section>
          <h3 className="mb-2 text-xs font-semibold uppercase tracking-wider text-muted-foreground">
            Closures
          </h3>
          <ClosureViz closures={data.closures} />
        </section>
      )}
    </div>
  );
}
