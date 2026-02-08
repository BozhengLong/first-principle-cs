"use client";

import { useState } from "react";
import { Code2, BarChart3, Bot } from "lucide-react";
import { useTranslations } from "next-intl";
import { CodePanel } from "./code-panel";
import { VizPanel } from "./viz-panel";
import { AiPanel } from "./ai-panel";
import { cn } from "@/lib/utils";

type MobileTab = "code" | "viz" | "ai";

export function WorkspaceLayout() {
  const [activeTab, setActiveTab] = useState<MobileTab>("code");
  const t = useTranslations("common");

  const tabs: { key: MobileTab; label: string; icon: React.ReactNode }[] = [
    { key: "code", label: t("codeEditor"), icon: <Code2 className="h-4 w-4" /> },
    { key: "viz", label: t("visualization"), icon: <BarChart3 className="h-4 w-4" /> },
    { key: "ai", label: t("aiAssistant"), icon: <Bot className="h-4 w-4" /> },
  ];

  return (
    <div className="flex h-full flex-col">
      {/* Desktop: side-by-side panels + bottom AI */}
      <div className="hidden flex-1 flex-col md:flex">
        <div className="flex flex-1 min-h-0">
          <div className="flex-1 p-4">
            <CodePanel />
          </div>
          <div className="flex-1 border-l p-4">
            <VizPanel />
          </div>
        </div>
        <AiPanel />
      </div>

      {/* Mobile: tab switcher */}
      <div className="flex flex-1 flex-col md:hidden">
        <div className="flex border-b">
          {tabs.map((tab) => (
            <button
              key={tab.key}
              onClick={() => setActiveTab(tab.key)}
              className={cn(
                "flex flex-1 items-center justify-center gap-1.5 px-3 py-2.5 text-xs font-medium transition-colors",
                activeTab === tab.key
                  ? "border-b-2 border-primary text-primary"
                  : "text-muted-foreground hover:text-foreground"
              )}
            >
              {tab.icon}
              {tab.label}
            </button>
          ))}
        </div>
        <div className="flex-1 p-4">
          {activeTab === "code" && <CodePanel />}
          {activeTab === "viz" && <VizPanel />}
          {activeTab === "ai" && (
            <div className="flex h-full flex-col items-center justify-center rounded-lg border border-dashed bg-muted/30 p-6">
              <Bot className="mb-3 h-10 w-10 text-muted-foreground/50" />
              <p className="text-sm text-muted-foreground">
                {t("aiPlaceholder")}
              </p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
