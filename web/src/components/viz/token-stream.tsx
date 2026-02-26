"use client";

import type { VizToken } from "@/types/viz";

const TOKEN_COLORS: Record<string, string> = {
  NUMBER: "bg-blue-100 text-blue-800 dark:bg-blue-900/40 dark:text-blue-300",
  SYMBOL: "bg-green-100 text-green-800 dark:bg-green-900/40 dark:text-green-300",
  BOOLEAN: "bg-purple-100 text-purple-800 dark:bg-purple-900/40 dark:text-purple-300",
  LPAREN: "bg-slate-100 text-slate-700 dark:bg-slate-800 dark:text-slate-300",
  RPAREN: "bg-slate-100 text-slate-700 dark:bg-slate-800 dark:text-slate-300",
};

interface TokenStreamProps {
  tokens: VizToken[];
  input: string;
}

export function TokenStream({ tokens, input }: TokenStreamProps) {
  return (
    <div className="space-y-4">
      <div className="rounded-md border bg-muted/30 p-3">
        <p className="mb-1 text-[10px] font-medium uppercase tracking-wider text-muted-foreground">
          Source
        </p>
        <code className="text-sm font-mono">{input}</code>
      </div>
      <div className="flex flex-wrap gap-1.5">
        {tokens.map((token, i) => (
          <span
            key={i}
            className={`inline-flex items-center gap-1 rounded-md px-2 py-1 text-xs font-mono ${TOKEN_COLORS[token.type] ?? "bg-gray-100 text-gray-700 dark:bg-gray-800 dark:text-gray-300"}`}
          >
            <span className="text-[10px] opacity-60">{token.type}</span>
            <span className="font-semibold">{token.value}</span>
          </span>
        ))}
      </div>
    </div>
  );
}
