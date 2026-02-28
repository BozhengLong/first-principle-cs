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

# Global counter for node IDs
_node_id_counter = [0]

def ast_node_to_dict(node, assign_id=False):
    from src.tiny_interpreter.parser import Number, Boolean, Symbol, SExpression

    base = {"line": node.line, "column": node.column}

    if assign_id:
        base["id"] = f"node_{_node_id_counter[0]}"
        _node_id_counter[0] += 1
        # Store ID on node for later reference
        if not hasattr(node, '_viz_id'):
            node._viz_id = base["id"]

    if isinstance(node, Number):
        return {**base, "type": "Number", "value": node.value}
    if isinstance(node, Boolean):
        return {**base, "type": "Boolean", "value": node.value}
    if isinstance(node, Symbol):
        return {**base, "type": "Symbol", "name": node.name}
    if isinstance(node, SExpression):
        return {
            **base,
            "type": "SExpression",
            "children": [ast_node_to_dict(e, assign_id) for e in node.elements],
        }
    return {**base, "type": str(type(node).__name__)}

def extract_ast(source):
    from src.tiny_interpreter.parser import parse
    global _node_id_counter
    _node_id_counter[0] = 0
    nodes = parse(source)
    return [ast_node_to_dict(n, assign_id=True) for n in nodes]

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

# Tracing evaluator wrapper
class TracingEvaluator:
    def __init__(self, evaluator):
        self.evaluator = evaluator
        self.trace = []
        self.closures = []
        self.step_id = 0
        self.env_counter = [0]
        self.closure_counter = [0]

    def add_step(self, step_type, node=None, env=None, result=None, message=None):
        step = {
            "stepId": self.step_id,
            "type": step_type,
            "message": message
        }
        if node is not None and hasattr(node, '_viz_id'):
            step["nodeId"] = node._viz_id
            step["node"] = ast_node_to_dict(node, assign_id=False)
        if env is not None:
            step["env"] = extract_env_chain(env, [0])[0]
        if result is not None:
            step["result"] = repr(result)
        self.trace.append(step)
        self.step_id += 1

    def eval_with_trace(self, node, env):
        from src.tiny_interpreter.parser import Number, Boolean, Symbol, SExpression
        from src.tiny_interpreter.evaluator import Closure

        # Add eval step
        self.add_step("eval", node=node, env=env, message=f"Evaluating {type(node).__name__}")

        # Self-evaluating
        if isinstance(node, (Number, Boolean)):
            result = node.value
            self.add_step("return", node=node, result=result, message=f"Returns {result}")
            return result

        # Variable lookup
        if isinstance(node, Symbol):
            # Trace the lookup path through env chain (use same ID scheme as extract_env_chain)
            lookup_frame_ids = []
            current_env = env
            env_chain_counter = [0]
            while current_env is not None:
                fid = f"env_{env_chain_counter[0]}"
                env_chain_counter[0] += 1
                lookup_frame_ids.append(fid)
                if node.name in current_env.bindings:
                    break
                current_env = current_env.parent
            result = env.get(node.name)
            # Include full env chain so lookup path animation works
            full_env_chain = extract_env_chain(env, [0])
            step = {
                "stepId": self.step_id,
                "type": "return",
                "message": f"Lookup {node.name} = {repr(result)}",
                "result": repr(result),
                "lookupFrameIds": lookup_frame_ids,
                "envChain": full_env_chain,
            }
            if hasattr(node, '_viz_id'):
                step["nodeId"] = node._viz_id
                step["node"] = ast_node_to_dict(node, assign_id=False)
            if env is not None:
                step["env"] = extract_env_chain(env, [0])[0]
            self.trace.append(step)
            self.step_id += 1
            return result

        # S-expression
        if isinstance(node, SExpression):
            if len(node.elements) == 0:
                raise ValueError("Empty S-expression")

            first = node.elements[0]

            # Special forms
            if isinstance(first, Symbol):
                if first.name == "define":
                    if len(node.elements) != 3:
                        raise ValueError("define requires 2 arguments")
                    name_node = node.elements[1]
                    if not isinstance(name_node, Symbol):
                        raise ValueError("define name must be a symbol")
                    val = self.eval_with_trace(node.elements[2], env)
                    env.define(name_node.name, val)
                    step = {
                        "stepId": self.step_id,
                        "type": "define",
                        "message": f"Define {name_node.name} = {repr(val)}",
                        "result": repr(val),
                        "definedName": name_node.name,
                    }
                    if hasattr(node, '_viz_id'):
                        step["nodeId"] = node._viz_id
                        step["node"] = ast_node_to_dict(node, assign_id=False)
                    if env is not None:
                        step["env"] = extract_env_chain(env, [0])[0]
                    self.trace.append(step)
                    self.step_id += 1
                    return val

                if first.name == "lambda":
                    if len(node.elements) < 3:
                        raise ValueError("lambda requires at least 2 arguments")
                    params_node = node.elements[1]
                    if not isinstance(params_node, SExpression):
                        raise ValueError("lambda params must be a list")
                    params = [p.name for p in params_node.elements if isinstance(p, Symbol)]
                    body = node.elements[2:]
                    closure = Closure(params, body, env)
                    # Capture closure data for visualization
                    closure_id = f"closure_{self.closure_counter[0]}"
                    self.closure_counter[0] += 1
                    self.closures.append({
                        "id": closure_id,
                        "params": params,
                        "body": [ast_node_to_dict(b, assign_id=False) for b in body],
                        "capturedEnv": extract_env_chain(env, [0])[0],
                        "createdAtStep": self.step_id,
                    })
                    self.add_step("closure_create", node=node, env=env, result=closure,
                                message=f"Create closure with params {params}")
                    return closure

                if first.name == "if":
                    if len(node.elements) != 4:
                        raise ValueError("if requires 3 arguments")
                    cond = self.eval_with_trace(node.elements[1], env)
                    if cond:
                        result = self.eval_with_trace(node.elements[2], env)
                    else:
                        result = self.eval_with_trace(node.elements[3], env)
                    self.add_step("return", node=node, result=result, message=f"If returns {repr(result)}")
                    return result

            # Function application
            func = self.eval_with_trace(first, env)
            args = [self.eval_with_trace(arg, env) for arg in node.elements[1:]]

            self.add_step("call", node=node, message=f"Call {repr(func)} with {len(args)} args")

            if isinstance(func, Closure):
                from src.tiny_interpreter.environment import Environment as Env
                if len(args) != len(func.params):
                    raise ValueError(f"Expected {len(func.params)} args, got {len(args)}")
                new_env = Env(parent=func.env)
                for param, arg in zip(func.params, args):
                    new_env.define(param, arg)
                result = None
                for expr in func.body:
                    result = self.eval_with_trace(expr, new_env)
                self.add_step("return", result=result, message=f"Closure returns {repr(result)}")
                return result
            elif callable(func):
                result = func(*args)
                self.add_step("return", result=result, message=f"Builtin returns {repr(result)}")
                return result
            else:
                raise ValueError(f"Not a function: {func}")

        raise ValueError(f"Unknown node type: {type(node)}")

def extract_viz(source, level):
    result = {}
    try:
        if level >= 1:
            result["tokens"] = extract_tokens(source)
        if level >= 2:
            result["ast"] = extract_ast(source)
        if level >= 3:
            from src.tiny_interpreter.evaluator import Evaluator
            from src.tiny_interpreter.parser import parse

            # Parse and assign IDs
            global _node_id_counter
            _node_id_counter[0] = 0
            nodes = parse(source)
            for node in nodes:
                ast_node_to_dict(node, assign_id=True)

            # Run with tracing
            evaluator = Evaluator()
            tracer = TracingEvaluator(evaluator)

            val = None
            for node in nodes:
                val = tracer.eval_with_trace(node, evaluator.global_env)

            result["evalResult"] = repr(val)
            result["environments"] = extract_env_chain(evaluator.global_env, [0])
            result["trace"] = tracer.trace
            result["closures"] = tracer.closures
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
      trace: parsed.trace as any[] | undefined,
      closures: parsed.closures as any[] | undefined,
    };
  } catch (e) {
    return { moduleSlug, input, error: String(e) };
  }
}
