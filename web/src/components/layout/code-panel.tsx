"use client";

import { useLocale } from "next-intl";
import type { LearningModule } from "@/data/types";
import { getAdjacentModules, getModules } from "@/data/tiny-interpreter";
import { CodeEditor } from "@/components/editor/code-editor";
import { StepHeader } from "@/components/editor/step-header";
import { TestRunner } from "@/components/editor/test-runner";
import { MarkdownContent } from "@/components/learn/markdown-content";
import { useWorkspace } from "@/contexts/workspace-context";

interface CodePanelProps {
  project: string;
  module: LearningModule;
}

export function CodePanel({ project, module }: CodePanelProps) {
  const locale = useLocale() as "zh" | "en";
  const { prev, next } = getAdjacentModules(project, module.slug);
  const totalModules = getModules(project).length;
  const { code, setCode, resetCode } = useWorkspace();

  // For modules without code, show README in full panel
  if (!module.hasCode) {
    return (
      <div className="flex h-full flex-col">
        <StepHeader
          project={project}
          module={module}
          prev={prev}
          next={next}
          totalModules={totalModules}
        />
        <div className="flex-1 overflow-auto p-4">
          <MarkdownContent content={module.readme[locale]} />
        </div>
      </div>
    );
  }

  return (
    <div className="flex h-full flex-col">
      <StepHeader
        project={project}
        module={module}
        prev={prev}
        next={next}
        totalModules={totalModules}
      />
      <div className="min-h-0 flex-1">
        <CodeEditor value={code} onChange={setCode} />
      </div>
      {module.testCode && (
        <TestRunner code={code} testCode={module.testCode} onReset={resetCode} />
      )}
    </div>
  );
}
