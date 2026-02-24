"use client";

import dynamic from "next/dynamic";
import { useTheme } from "next-themes";

const CodeMirrorEditor = dynamic(
  () =>
    Promise.all([
      import("@uiw/react-codemirror"),
      import("@codemirror/lang-python"),
    ]).then(([codemirror, pythonLang]) => {
      const CodeMirror = codemirror.default;
      const { python } = pythonLang;

      return function CodeMirrorWrapper(props: {
        value: string;
        onChange: (val: string) => void;
        theme: "light" | "dark";
      }) {
        return (
          <CodeMirror
            value={props.value}
            onChange={props.onChange}
            extensions={[python()]}
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
