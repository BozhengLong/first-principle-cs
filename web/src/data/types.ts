import type { VizType } from "@/types/viz";

export interface DiagnosticRule {
  pattern: string | string[];
  message: { zh: string; en: string };
}

export interface LearningModule {
  id: string;
  index: number;
  slug: string;
  hasCode: boolean;
  skeleton?: string;
  testCode?: string;
  readme: { zh: string; en: string };
  hints?: { level: number; content: { zh: string; en: string } }[];
  diagnostics?: DiagnosticRule[];
  vizType?: VizType;
}
