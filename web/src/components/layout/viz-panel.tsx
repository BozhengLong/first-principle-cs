"use client";

import type { LearningModule } from "@/data/types";
import { VizTabPanel } from "@/components/viz/viz-tab-panel";

interface VizPanelProps {
  project: string;
  module: LearningModule;
}

export function VizPanel({ module }: VizPanelProps) {
  return <VizTabPanel module={module} />;
}
