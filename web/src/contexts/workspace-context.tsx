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
  module: LearningModule;
  code: string;
  setCode: (code: string) => void;
  resetCode: () => void;
  vizData: VizData | null;
  vizLoading: boolean;
  vizInput: string;
  setVizInput: (input: string) => void;
  runVisualize: () => Promise<void>;
  currentStep: number;
  setCurrentStep: (step: number) => void;
  isPlaying: boolean;
  setIsPlaying: (playing: boolean) => void;
  playbackSpeed: number;
  setPlaybackSpeed: (speed: number) => void;
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
  const [currentStep, setCurrentStep] = useState(0);
  const [isPlaying, setIsPlaying] = useState(false);
  const [playbackSpeed, setPlaybackSpeed] = useState(1000);

  const runVisualize = useCallback(async () => {
    if (!module.vizType || module.vizType === "none") return;
    setVizLoading(true);
    setCurrentStep(0);
    setIsPlaying(false);
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
        module,
        code,
        setCode,
        resetCode: reset,
        vizData,
        vizLoading,
        vizInput,
        setVizInput,
        runVisualize,
        currentStep,
        setCurrentStep,
        isPlaying,
        setIsPlaying,
        playbackSpeed,
        setPlaybackSpeed,
      }}
    >
      {children}
    </WorkspaceContext.Provider>
  );
}
