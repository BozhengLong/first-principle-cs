"use client";

import { useState } from "react";
import { useLocale, useTranslations } from "next-intl";
import { Loader2 } from "lucide-react";
import { ScrollArea } from "@/components/ui/scroll-area";
import { MarkdownContent } from "@/components/learn/markdown-content";
import { HintAccordion } from "@/components/learn/hint-accordion";
import { VizInputBar } from "@/components/viz/viz-input-bar";
import { VizRenderer } from "@/components/viz/viz-renderer";
import { useWorkspace } from "@/contexts/workspace-context";
import { cn } from "@/lib/utils";
import type { LearningModule } from "@/data/types";

type TabKey = "guide" | "visualization" | "hints";

interface VizTabPanelProps {
  module: LearningModule;
}

export function VizTabPanel({ module }: VizTabPanelProps) {
  const locale = useLocale() as "zh" | "en";
  const t = useTranslations("viz");
  const { vizData, vizLoading } = useWorkspace();

  const hasViz = module.vizType && module.vizType !== "none";
  const hasHints = module.hints && module.hints.length > 0;

  const availableTabs: { key: TabKey; label: string }[] = [];
  availableTabs.push({ key: "guide", label: t("tabGuide") });
  if (hasViz) {
    availableTabs.push({ key: "visualization", label: t("tabVisualization") });
  }
  if (hasHints) {
    availableTabs.push({ key: "hints", label: t("tabHints") });
  }

  const defaultTab: TabKey = hasViz ? "visualization" : "guide";
  const [activeTab, setActiveTab] = useState<TabKey>(defaultTab);

  return (
    <div className="flex h-full flex-col">
      {/* Tab bar */}
      {availableTabs.length > 1 && (
        <div className="flex border-b">
          {availableTabs.map((tab) => (
            <button
              key={tab.key}
              onClick={() => setActiveTab(tab.key)}
              className={cn(
                "px-4 py-2 text-xs font-medium transition-colors",
                activeTab === tab.key
                  ? "border-b-2 border-primary text-primary"
                  : "text-muted-foreground hover:text-foreground"
              )}
            >
              {tab.label}
            </button>
          ))}
        </div>
      )}

      {/* Tab content */}
      <div className="flex-1 min-h-0">
        {activeTab === "guide" && (
          <ScrollArea className="h-full">
            <div className="p-4">
              <MarkdownContent content={module.readme[locale]} />
            </div>
          </ScrollArea>
        )}

        {activeTab === "visualization" && hasViz && (
          <div className="flex h-full flex-col">
            <VizInputBar />
            <ScrollArea className="flex-1">
              <div className="p-4">
                {vizLoading && (
                  <div className="flex items-center justify-center py-12">
                    <Loader2 className="h-5 w-5 animate-spin text-muted-foreground" />
                  </div>
                )}
                {!vizLoading && vizData && (
                  <VizRenderer vizType={module.vizType!} data={vizData} />
                )}
                {!vizLoading && !vizData && (
                  <p className="py-12 text-center text-sm text-muted-foreground">
                    {t("noData")}
                  </p>
                )}
              </div>
            </ScrollArea>
          </div>
        )}

        {activeTab === "hints" && hasHints && (
          <ScrollArea className="h-full">
            <div className="p-4">
              <HintAccordion hints={module.hints!} />
            </div>
          </ScrollArea>
        )}
      </div>
    </div>
  );
}
