export interface VizToken {
  type: string;
  value: string;
  line: number;
  column: number;
}

export interface VizASTNode {
  type: string;
  value?: string | number | boolean;
  name?: string;
  children?: VizASTNode[];
  line: number;
  column: number;
}

export interface VizEnvFrame {
  id: string;
  label: string;
  bindings: Record<string, string>;
  parentId: string | null;
}

export type VizType = "none" | "tokens" | "ast" | "environment" | "evaluator" | "closure";

export interface VizData {
  moduleSlug: string;
  input: string;
  tokens?: VizToken[];
  ast?: VizASTNode[];
  environments?: VizEnvFrame[];
  evalResult?: string;
  error?: string;
}
