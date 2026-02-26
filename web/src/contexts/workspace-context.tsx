"use client";

import {
  createContext,
  useCallback,
  useContext,
  useState,
  type ReactNode,
} from "react";
import type { LearningModule } from "@/data/types";
import type { VizData } from "@/types/viz";
import { useEditorState } from "@/hooks/use-editor-state";
import { getDefaultVizInput } from "@/lib/pyodide/viz-extractor";

interface WorkspaceContextValue {
  code: string;
  setCode: (code: string) => void;
  resetCode: () => void;
  vizData: VizData | null;
  vizLoading: boolean;
  vizInput: string;
  setVizInput: (input: string) => void;
  runVisualize: () => Promise<void>;
}

const WorkspaceContext = createContext<WorkspaceContextValue | null>(null);

export function useWorkspace() {
  const ctx = useContext(WorkspaceContext);
  if (!ctx) throw new Error("useWorkspace must be used within WorkspaceProvider");
  return ctx;
}

interface WorkspaceProviderProps {
  module: LearningModule;
  children: ReactNode;
}

export function WorkspaceProvider({ module, children }: WorkspaceProviderProps) {
  const { code, setCode, reset } = useEditorState(module.id, module.skeleton);
  const [vizData, setVizData] = useState<VizData | null>(null);
  const [vizLoading, setVizLoading] = useState(false);
  const [vizInput, setVizInput] = useState(() => getDefaultVizInput(module.slug));

  const runVisualize = useCallback(async () => {
    if (!module.vizType || module.vizType === "none") return;
    setVizLoading(true);
    try {
      const { extractVizData } = await import("@/lib/pyodide/viz-extractor");
      const data = await extractVizData(module.slug, vizInput);
      setVizData(data);
    } finally {
      setVizLoading(false);
    }
  }, [module.slug, module.vizType, vizInput]);

  return (
    <WorkspaceContext.Provider
      value={{
        code,
        setCode,
        resetCode: reset,
        vizData,
        vizLoading,
        vizInput,
        setVizInput,
        runVisualize,
      }}
    >
      {children}
    </WorkspaceContext.Provider>
  );
}
