"use client";

import { useState } from "react";
import { Bot, ChevronUp, ChevronDown } from "lucide-react";
import { useTranslations } from "next-intl";
import { Button } from "@/components/ui/button";

export function AiPanel() {
  const [expanded, setExpanded] = useState(false);
  const t = useTranslations("common");

  return (
    <div className="border-t bg-background">
      <button
        onClick={() => setExpanded(!expanded)}
        className="flex w-full items-center justify-between px-4 py-2 text-sm hover:bg-accent transition-colors"
      >
        <div className="flex items-center gap-2">
          <Bot className="h-4 w-4" />
          <span className="font-medium">{t("aiAssistant")}</span>
        </div>
        {expanded ? (
          <ChevronDown className="h-4 w-4" />
        ) : (
          <ChevronUp className="h-4 w-4" />
        )}
      </button>
      {expanded && (
        <div className="flex h-48 flex-col items-center justify-center border-t p-6">
          <Bot className="mb-3 h-8 w-8 text-muted-foreground/50" />
          <p className="text-sm text-muted-foreground">
            {t("aiPlaceholder")}
          </p>
        </div>
      )}
    </div>
  );
}
