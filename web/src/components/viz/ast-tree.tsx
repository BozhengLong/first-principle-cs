"use client";

import { useMemo } from "react";
import type { VizASTNode } from "@/types/viz";

const NODE_W = 100;
const NODE_H = 36;
const H_GAP = 16;
const V_GAP = 48;

interface LayoutNode {
  node: VizASTNode;
  x: number;
  y: number;
  width: number;
  children: LayoutNode[];
}

function layoutTree(node: VizASTNode, depth: number): LayoutNode {
  const kids = (node.children ?? []).map((c) => layoutTree(c, depth + 1));
  const childrenWidth = kids.length > 0
    ? kids.reduce((s, k) => s + k.width, 0) + (kids.length - 1) * H_GAP
    : 0;
  const width = Math.max(NODE_W, childrenWidth);
  return { node, x: 0, y: depth * (NODE_H + V_GAP), width, children: kids };
}

function positionTree(ln: LayoutNode, left: number) {
  ln.x = left + ln.width / 2;
  let childLeft = left + (ln.width - (ln.children.length > 0
    ? ln.children.reduce((s, k) => s + k.width, 0) + (ln.children.length - 1) * H_GAP
    : 0)) / 2;
  for (const child of ln.children) {
    positionTree(child, childLeft);
    childLeft += child.width + H_GAP;
  }
}

function collectNodes(ln: LayoutNode): LayoutNode[] {
  return [ln, ...ln.children.flatMap(collectNodes)];
}

const TYPE_COLORS: Record<string, string> = {
  SExpression: "#6366f1",
  Number: "#3b82f6",
  Symbol: "#22c55e",
  Boolean: "#a855f7",
};

function nodeLabel(n: VizASTNode): string {
  if (n.type === "Number") return String(n.value);
  if (n.type === "Boolean") return String(n.value);
  if (n.type === "Symbol") return n.name ?? "";
  if (n.type === "SExpression") return "( )";
  return n.type;
}

interface ASTTreeProps {
  ast: VizASTNode[];
}

export function ASTTree({ ast }: ASTTreeProps) {
  const { allNodes, viewBox } = useMemo(() => {
    // Wrap multiple top-level nodes in a virtual root
    const root: VizASTNode = ast.length === 1
      ? ast[0]
      : { type: "SExpression", children: ast, line: 0, column: 0 };

    const tree = layoutTree(root, 0);
    positionTree(tree, 0);
    const all = collectNodes(tree);
    const maxX = Math.max(...all.map((n) => n.x)) + NODE_W / 2 + 8;
    const maxY = Math.max(...all.map((n) => n.y)) + NODE_H + 8;
    return { allNodes: all, viewBox: `0 0 ${maxX} ${maxY}` };
  }, [ast]);

  return (
    <svg viewBox={viewBox} className="w-full" style={{ maxHeight: 400 }}>
      {/* Edges */}
      {allNodes.map((ln) =>
        ln.children.map((child, ci) => (
          <line
            key={`${ln.x}-${ln.y}-${ci}`}
            x1={ln.x}
            y1={ln.y + NODE_H}
            x2={child.x}
            y2={child.y}
            className="stroke-muted-foreground/40"
            strokeWidth={1.5}
          />
        ))
      )}
      {/* Nodes */}
      {allNodes.map((ln, i) => {
        const fill = TYPE_COLORS[ln.node.type] ?? "#64748b";
        const label = nodeLabel(ln.node);
        return (
          <g key={i}>
            <rect
              x={ln.x - NODE_W / 2}
              y={ln.y}
              width={NODE_W}
              height={NODE_H}
              rx={8}
              fill={fill}
              opacity={0.15}
              stroke={fill}
              strokeWidth={1.5}
            />
            <text
              x={ln.x}
              y={ln.y + NODE_H / 2}
              textAnchor="middle"
              dominantBaseline="central"
              className="text-xs font-mono"
              fill={fill}
            >
              {label}
            </text>
          </g>
        );
      })}
    </svg>
  );
}
