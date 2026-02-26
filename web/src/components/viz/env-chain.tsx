"use client";

import { useState } from "react";
import { ChevronDown, ChevronRight } from "lucide-react";
import type { VizEnvFrame } from "@/types/viz";

interface EnvChainProps {
  environments: VizEnvFrame[];
  evalResult?: string;
}

export function EnvChain({ environments, evalResult }: EnvChainProps) {
  return (
    <div className="space-y-3">
      {evalResult !== undefined && (
        <div className="rounded-md border border-green-200 bg-green-50 p-3 dark:border-green-900 dark:bg-green-950/30">
          <p className="text-[10px] font-medium uppercase tracking-wider text-green-600 dark:text-green-400">
            Result
          </p>
          <code className="text-sm font-mono text-green-800 dark:text-green-300">
            {evalResult}
          </code>
        </div>
      )}
      {environments.map((frame, i) => (
        <EnvFrameCard key={frame.id} frame={frame} isFirst={i === 0} />
      ))}
      {environments.length > 1 && (
        <svg className="mx-auto" width="24" height="0" />
      )}
    </div>
  );
}

function EnvFrameCard({ frame, isFirst }: { frame: VizEnvFrame; isFirst: boolean }) {
  const isGlobal = frame.label === "global";
  const builtinEntries = Object.entries(frame.bindings).filter(
    ([, v]) => v === "<builtin>"
  );
  const userEntries = Object.entries(frame.bindings).filter(
    ([, v]) => v !== "<builtin>"
  );
  const [showBuiltins, setShowBuiltins] = useState(false);

  return (
    <div className="relative">
      {!isFirst && (
        <div className="flex justify-center -mt-1 mb-1">
          <svg width="24" height="20">
            <line x1="12" y1="0" x2="12" y2="14" className="stroke-muted-foreground/50" strokeWidth="1.5" />
            <polygon points="6,14 12,20 18,14" className="fill-muted-foreground/50" />
          </svg>
        </div>
      )}
      <div className="rounded-lg border bg-card p-3">
        <p className="mb-2 text-xs font-semibold text-muted-foreground uppercase tracking-wider">
          {frame.label}
        </p>
        {userEntries.length > 0 && (
          <table className="w-full text-xs">
            <tbody>
              {userEntries.map(([k, v]) => (
                <tr key={k} className="border-b last:border-0">
                  <td className="py-1 pr-3 font-mono font-semibold text-blue-600 dark:text-blue-400">
                    {k}
                  </td>
                  <td className="py-1 font-mono text-muted-foreground">{v}</td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
        {isGlobal && builtinEntries.length > 0 && (
          <button
            onClick={() => setShowBuiltins(!showBuiltins)}
            className="mt-2 flex items-center gap-1 text-[10px] text-muted-foreground hover:text-foreground"
          >
            {showBuiltins ? (
              <ChevronDown className="h-3 w-3" />
            ) : (
              <ChevronRight className="h-3 w-3" />
            )}
            {builtinEntries.length} builtins
          </button>
        )}
        {showBuiltins && (
          <div className="mt-1 flex flex-wrap gap-1">
            {builtinEntries.map(([k]) => (
              <span
                key={k}
                className="rounded bg-muted px-1.5 py-0.5 text-[10px] font-mono text-muted-foreground"
              >
                {k}
              </span>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
