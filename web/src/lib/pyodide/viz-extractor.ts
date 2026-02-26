import type { VizData, VizToken, VizASTNode, VizEnvFrame } from "@/types/viz";

const VIZ_SCRIPT = `
import json, sys

def extract_tokens(source):
    from src.tiny_interpreter.lexer import Lexer, TokenType
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    return [
        {"type": t.type.name, "value": repr(t.value), "line": t.line, "column": t.column}
        for t in tokens if t.type != TokenType.EOF
    ]

def ast_node_to_dict(node):
    from src.tiny_interpreter.parser import Number, Boolean, Symbol, SExpression
    if isinstance(node, Number):
        return {"type": "Number", "value": node.value, "line": node.line, "column": node.column}
    if isinstance(node, Boolean):
        return {"type": "Boolean", "value": node.value, "line": node.line, "column": node.column}
    if isinstance(node, Symbol):
        return {"type": "Symbol", "name": node.name, "line": node.line, "column": node.column}
    if isinstance(node, SExpression):
        return {
            "type": "SExpression",
            "children": [ast_node_to_dict(e) for e in node.elements],
            "line": node.line, "column": node.column,
        }
    return {"type": str(type(node).__name__), "line": 0, "column": 0}

def extract_ast(source):
    from src.tiny_interpreter.parser import parse
    nodes = parse(source)
    return [ast_node_to_dict(n) for n in nodes]

def extract_env_chain(env, counter=[0]):
    frames = []
    while env is not None:
        frame_id = f"env_{counter[0]}"
        counter[0] += 1
        bindings = {}
        for k, v in env.bindings.items():
            if callable(v):
                bindings[k] = "<builtin>"
            else:
                bindings[k] = repr(v)
        parent_id = None
        if env.parent is not None:
            parent_id = f"env_{counter[0]}"
        label = "global" if env.parent is None else f"frame"
        frames.append({"id": frame_id, "label": label, "bindings": bindings, "parentId": parent_id})
        env = env.parent
    return frames

def extract_viz(source, level):
    result = {}
    try:
        if level >= 1:
            result["tokens"] = extract_tokens(source)
        if level >= 2:
            result["ast"] = extract_ast(source)
        if level >= 3:
            from src.tiny_interpreter.evaluator import Evaluator
            evaluator = Evaluator()
            val = evaluator.run(source)
            result["evalResult"] = repr(val)
            result["environments"] = extract_env_chain(evaluator.global_env, [0])
    except Exception as e:
        result["error"] = str(e)
    return json.dumps(result)
`;

const MODULE_LEVELS: Record<string, number> = {
  lexer: 1,
  parser: 2,
  environment: 3,
  "evaluator-basic": 3,
  "evaluator-lambda": 3,
};

const DEFAULT_INPUTS: Record<string, string> = {
  lexer: "(+ 1 (* 2 3))",
  parser: "(+ 1 (* 2 3))",
  environment: "(define x 10)",
  "evaluator-basic": "(+ 1 (* 2 3))",
  "evaluator-lambda": "(define square (lambda (x) (* x x)))\n(square 5)",
};

export function getDefaultVizInput(moduleSlug: string): string {
  return DEFAULT_INPUTS[moduleSlug] ?? "(+ 1 2)";
}

export async function extractVizData(
  moduleSlug: string,
  input: string
): Promise<VizData> {
  const { getPyodide } = await import("@/lib/pyodide/pyodide-manager");
  const py = getPyodide();
  if (!py) {
    return { moduleSlug, input, error: "Pyodide not ready" };
  }

  const level = MODULE_LEVELS[moduleSlug] ?? 0;
  if (level === 0) {
    return { moduleSlug, input };
  }

  try {
    // Clear module cache for fresh run
    await py.runPythonAsync(`
import sys
for mod_name in list(sys.modules.keys()):
    if mod_name.startswith("src."):
        del sys.modules[mod_name]
`);

    await py.runPythonAsync(VIZ_SCRIPT);

    const raw = await py.runPythonAsync(
      `extract_viz(${JSON.stringify(input)}, ${level})`
    );
    const parsed = JSON.parse(String(raw));

    return {
      moduleSlug,
      input,
      tokens: parsed.tokens as VizToken[] | undefined,
      ast: parsed.ast as VizASTNode[] | undefined,
      environments: parsed.environments as VizEnvFrame[] | undefined,
      evalResult: parsed.evalResult as string | undefined,
      error: parsed.error as string | undefined,
    };
  } catch (e) {
    return { moduleSlug, input, error: String(e) };
  }
}
