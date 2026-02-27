"use client";

import { motion } from "framer-motion";
import type { VizClosure } from "@/types/viz";

interface ClosureVizProps {
  closures: VizClosure[];
}

export function ClosureViz({ closures }: ClosureVizProps) {
  if (closures.length === 0) {
    return (
      <div className="rounded-lg border bg-muted/30 p-6 text-center text-sm text-muted-foreground">
        No closures created yet. Try defining a lambda function!
      </div>
    );
  }

  return (
    <div className="space-y-4">
      {closures.map((closure, idx) => (
        <ClosureCard key={closure.id} closure={closure} index={idx} />
      ))}
    </div>
  );
}

function ClosureCard({ closure, index }: { closure: VizClosure; index: number }) {
  return (
    <motion.div
      initial={{ opacity: 0, x: -20 }}
      animate={{ opacity: 1, x: 0 }}
      transition={{ delay: index * 0.1 }}
      className="rounded-lg border bg-card p-4"
    >
      <div className="mb-3 flex items-center justify-between">
        <h4 className="text-sm font-semibold">Closure #{index + 1}</h4>
        <span className="text-xs text-muted-foreground">
          Created at step {closure.createdAtStep + 1}
        </span>
      </div>

      <div className="grid grid-cols-2 gap-4">
        {/* Left: Function structure */}
        <div className="space-y-3">
          <div className="rounded-md border bg-muted/30 p-3">
            <p className="mb-1 text-[10px] font-medium uppercase tracking-wider text-muted-foreground">
              Parameters
            </p>
            <div className="flex flex-wrap gap-1">
              {closure.params.map((param) => (
                <span
                  key={param}
                  className="rounded bg-blue-100 px-2 py-0.5 text-xs font-mono text-blue-700 dark:bg-blue-950 dark:text-blue-300"
                >
                  {param}
                </span>
              ))}
            </div>
          </div>

          <div className="rounded-md border bg-muted/30 p-3">
            <p className="mb-1 text-[10px] font-medium uppercase tracking-wider text-muted-foreground">
              Body
            </p>
            <div className="space-y-1">
              {closure.body.map((node, i) => (
                <div
                  key={i}
                  className="rounded bg-background px-2 py-1 text-xs font-mono"
                >
                  {nodeToString(node)}
                </div>
              ))}
            </div>
          </div>
        </div>

        {/* Right: Captured environment */}
        <div className="relative">
          <div className="absolute -left-2 top-1/2 -translate-y-1/2">
            <svg width="16" height="40">
              <defs>
                <marker
                  id="arrowhead"
                  markerWidth="10"
                  markerHeight="10"
                  refX="9"
                  refY="3"
                  orient="auto"
                >
                  <polygon
                    points="0 0, 10 3, 0 6"
                    className="fill-green-500"
                  />
                </marker>
              </defs>
              <line
                x1="0"
                y1="20"
                x2="16"
                y2="20"
                className="stroke-green-500"
                strokeWidth="2"
                markerEnd="url(#arrowhead)"
              />
            </svg>
          </div>

          <motion.div
            initial={{ scale: 0.95 }}
            animate={{ scale: 1 }}
            transition={{ delay: 0.2 }}
            className="rounded-md border-2 border-green-500/50 bg-green-50/50 p-3 dark:bg-green-950/20"
          >
            <p className="mb-2 text-[10px] font-medium uppercase tracking-wider text-green-600 dark:text-green-400">
              Captured Environment
            </p>
            {Object.keys(closure.capturedEnv.bindings).length > 0 ? (
              <table className="w-full text-xs">
                <tbody>
                  {Object.entries(closure.capturedEnv.bindings)
                    .filter(([, v]) => v !== "<builtin>")
                    .map(([k, v]) => (
                      <tr key={k} className="border-b last:border-0">
                        <td className="py-1 pr-2 font-mono font-semibold text-green-700 dark:text-green-400">
                          {k}
                        </td>
                        <td className="py-1 font-mono text-muted-foreground">
                          {v}
                        </td>
                      </tr>
                    ))}
                </tbody>
              </table>
            ) : (
              <p className="text-xs text-muted-foreground">
                (empty environment)
              </p>
            )}
          </motion.div>
        </div>
      </div>
    </motion.div>
  );
}

function nodeToString(node: any): string {
  if (node.type === "Number") return String(node.value);
  if (node.type === "Boolean") return String(node.value);
  if (node.type === "Symbol") return node.name;
  if (node.type === "SExpression") {
    const children = node.children?.map(nodeToString).join(" ") ?? "";
    return `(${children})`;
  }
  return node.type;
}
