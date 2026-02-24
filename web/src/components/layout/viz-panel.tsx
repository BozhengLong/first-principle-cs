"use client";

import { useLocale } from "next-intl";
import { ScrollArea } from "@/components/ui/scroll-area";
import { MarkdownContent } from "@/components/learn/markdown-content";
import { HintAccordion } from "@/components/learn/hint-accordion";
import type { LearningModule } from "@/data/types";

interface VizPanelProps {
  project: string;
  module: LearningModule;
}

export function VizPanel({ module }: VizPanelProps) {
  const locale = useLocale() as "zh" | "en";

  return (
    <ScrollArea className="h-full">
      <div className="p-4">
        <MarkdownContent content={module.readme[locale]} />
        {module.hints && module.hints.length > 0 && (
          <HintAccordion hints={module.hints} />
        )}
      </div>
    </ScrollArea>
  );
}
