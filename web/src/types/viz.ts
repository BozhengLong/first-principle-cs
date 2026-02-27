export interface VizToken {
  type: string;
  value: string;
  line: number;
  column: number;
}

export interface VizASTNode {
  id?: string;
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

export interface ExecutionStep {
  stepId: number;
  type: "eval" | "define" | "call" | "return" | "closure_create";
  node?: VizASTNode;
  env?: VizEnvFrame;
  result?: string;
  message?: string;
  nodeId?: string;
}

export interface VizClosure {
  id: string;
  params: string[];
  body: VizASTNode[];
  capturedEnv: VizEnvFrame;
  createdAtStep: number;
}

export interface VizData {
  moduleSlug: string;
  input: string;
  tokens?: VizToken[];
  ast?: VizASTNode[];
  environments?: VizEnvFrame[];
  evalResult?: string;
  error?: string;
  trace?: ExecutionStep[];
  closures?: VizClosure[];
}
