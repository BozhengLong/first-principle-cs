"use client";

import dynamic from "next/dynamic";
import { useTheme } from "next-themes";

const CodeMirrorEditor = dynamic(
  () =>
    Promise.all([
      import("@uiw/react-codemirror"),
      import("@codemirror/lang-python"),
      import("@codemirror/view"),
    ]).then(([codemirror, pythonLang, viewMod]) => {
      const CodeMirror = codemirror.default;
      const { python } = pythonLang;
      const { ViewPlugin, Decoration, EditorView } = viewMod;

      const todoLineMark = Decoration.line({ class: "cm-todo-line" });

      const todoHighlighter = ViewPlugin.fromClass(
        class {
          decorations: ReturnType<typeof Decoration.set>;
          constructor(view: ReturnType<typeof EditorView.prototype.state.doc.toString> extends string ? any : any) {
            this.decorations = this.buildDecorations(view);
          }
          update(update: any) {
            if (update.docChanged || update.viewportChanged) {
              this.decorations = this.buildDecorations(update.view);
            }
          }
          buildDecorations(view: any) {
            const builder: any[] = [];
            for (const { from, to } of view.visibleRanges) {
              const doc = view.state.doc;
              const startLine = doc.lineAt(from).number;
              const endLine = doc.lineAt(to).number;
              for (let i = startLine; i <= endLine; i++) {
                const line = doc.line(i);
                if (line.text.includes("# TODO")) {
                  builder.push(todoLineMark.range(line.from));
                }
              }
            }
            return Decoration.set(builder);
          }
        },
        { decorations: (v: any) => v.decorations }
      );

      const todoTheme = EditorView.baseTheme({
        "&light .cm-todo-line": { backgroundColor: "rgba(250, 204, 21, 0.15)" },
        "&dark .cm-todo-line": { backgroundColor: "rgba(250, 204, 21, 0.10)" },
      });

      return function CodeMirrorWrapper(props: {
        value: string;
        onChange: (val: string) => void;
        theme: "light" | "dark";
      }) {
        return (
          <CodeMirror
            value={props.value}
            onChange={props.onChange}
            extensions={[python(), todoHighlighter, todoTheme]}
            theme={props.theme}
            basicSetup={{
              lineNumbers: true,
              foldGutter: true,
              highlightActiveLine: true,
              autocompletion: false,
            }}
            className="h-full text-sm"
          />
        );
      };
    }),
  {
    ssr: false,
    loading: () => (
      <div className="flex h-full items-center justify-center rounded-lg border bg-muted/30">
        <div className="h-5 w-5 animate-spin rounded-full border-2 border-primary border-t-transparent" />
      </div>
    ),
  }
);

interface CodeEditorProps {
  value: string;
  onChange: (value: string) => void;
}

export function CodeEditor({ value, onChange }: CodeEditorProps) {
  const { resolvedTheme } = useTheme();
  const theme = resolvedTheme === "dark" ? "dark" : "light";

  return (
    <div className="h-full overflow-hidden rounded-lg border [&_.cm-editor]:!h-full [&_.cm-editor]:!outline-none [&_.cm-scroller]:!overflow-auto">
      <CodeMirrorEditor value={value} onChange={onChange} theme={theme} />
    </div>
  );
}
