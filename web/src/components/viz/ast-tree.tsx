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
  currentNodeId?: string;
  executedNodeIds?: Set<string>;
  onNodeClick?: (node: VizASTNode) => void;
}

export function ASTTree({ ast, currentNodeId, executedNodeIds, onNodeClick }: ASTTreeProps) {
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

  const getNodeOpacity = (nodeId?: string) => {
    if (!executedNodeIds || executedNodeIds.size === 0) return 1;
    if (!nodeId) return 0.4;
    return executedNodeIds.has(nodeId) ? 1 : 0.4;
  };

  const isCurrentNode = (nodeId?: string) => {
    return currentNodeId && nodeId === currentNodeId;
  };

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
        const isCurrent = isCurrentNode(ln.node.id);
        const opacity = getNodeOpacity(ln.node.id);
        const strokeWidth = isCurrent ? 3 : 1.5;

        return (
          <g
            key={i}
            style={{ cursor: onNodeClick ? "pointer" : "default" }}
            onClick={() => onNodeClick?.(ln.node)}
            className="transition-all duration-200 hover:scale-105"
          >
            <rect
              x={ln.x - NODE_W / 2}
              y={ln.y}
              width={NODE_W}
              height={NODE_H}
              rx={8}
              fill={fill}
              opacity={opacity * 0.15}
              stroke={fill}
              strokeWidth={strokeWidth}
              filter={isCurrent ? "url(#glow)" : undefined}
            />
            <text
              x={ln.x}
              y={ln.y + NODE_H / 2}
              textAnchor="middle"
              dominantBaseline="central"
              className="text-xs font-mono"
              fill={fill}
              opacity={opacity}
            >
              {label}
            </text>
          </g>
        );
      })}
      {/* Glow filter for current node */}
      <defs>
        <filter id="glow">
          <feGaussianBlur stdDeviation="2" result="coloredBlur" />
          <feMerge>
            <feMergeNode in="coloredBlur" />
            <feMergeNode in="SourceGraphic" />
          </feMerge>
        </filter>
      </defs>
    </svg>
  );
}
