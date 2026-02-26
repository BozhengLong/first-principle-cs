"use client";

import { useTranslations } from "next-intl";
import type { VizData } from "@/types/viz";
import type { VizType } from "@/types/viz";
import { TokenStream } from "./token-stream";
import { ASTTree } from "./ast-tree";
import { EnvChain } from "./env-chain";

interface VizRendererProps {
  vizType: VizType;
  data: VizData;
}

export function VizRenderer({ vizType, data }: VizRendererProps) {
  const t = useTranslations("viz");

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
          <ASTTree ast={data.ast} />
        </section>
      )}

      {data.environments && (vizType === "environment" || vizType === "evaluator" || vizType === "closure") && (
        <section>
          <h3 className="mb-2 text-xs font-semibold uppercase tracking-wider text-muted-foreground">
            Environment
          </h3>
          <EnvChain environments={data.environments} evalResult={data.evalResult} />
        </section>
      )}
    </div>
  );
}
