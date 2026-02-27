"use client";

import { useState, useEffect } from "react";
import { ChevronDown, ChevronUp, Lightbulb } from "lucide-react";
import { useTranslations, useLocale } from "next-intl";
import { Button } from "@/components/ui/button";
import type { GuidanceQuestion } from "@/data/types";
import { cn } from "@/lib/utils";

interface GuidancePanelProps {
  guidance: GuidanceQuestion[];
  hints?: { level: number; content: { zh: string; en: string } }[];
  hasFailures: boolean;
}

export function GuidancePanel({
  guidance,
  hints,
  hasFailures,
}: GuidancePanelProps) {
  const t = useTranslations("learn.guidance");
  const locale = useLocale() as "zh" | "en";

  const [isExpanded, setIsExpanded] = useState(false);
  const [revealedCount, setRevealedCount] = useState(1);
  const [hasAutoExpanded, setHasAutoExpanded] = useState(false);

  // Auto-expand on first failure
  useEffect(() => {
    if (hasFailures && !hasAutoExpanded) {
      setIsExpanded(true);
      setHasAutoExpanded(true);
    }
  }, [hasFailures, hasAutoExpanded]);

  if (!guidance || guidance.length === 0) {
    return null;
  }

  const handleContinueThinking = () => {
    if (revealedCount < guidance.length) {
      setRevealedCount(revealedCount + 1);
    }
  };

  const handleViewHint = (level: number) => {
    // Scroll to hint accordion in viz panel
    const hintElement = document.getElementById(`hint-${level}`);
    if (hintElement) {
      hintElement.scrollIntoView({ behavior: "smooth", block: "center" });
    }
  };

  const allQuestionsRevealed = revealedCount >= guidance.length;

  return (
    <div className="border-t bg-muted/30">
      {/* Header */}
      <button
        onClick={() => setIsExpanded(!isExpanded)}
        className="flex w-full items-center justify-between px-3 py-2 text-sm font-medium hover:bg-muted/50 transition-colors"
      >
        <div className="flex items-center gap-2">
          <Lightbulb className="h-4 w-4 text-amber-600 dark:text-amber-400" />
          <span>{t("header")}</span>
        </div>
        {isExpanded ? (
          <ChevronUp className="h-4 w-4 text-muted-foreground" />
        ) : (
          <ChevronDown className="h-4 w-4 text-muted-foreground" />
        )}
      </button>

      {/* Content */}
      {isExpanded && (
        <div className="space-y-3 px-3 pb-3">
          {/* Questions */}
          <div className="space-y-2">
            {guidance.slice(0, revealedCount).map((q, i) => (
              <div
                key={i}
                className={cn(
                  "rounded-md border bg-background p-3 text-sm",
                  i === revealedCount - 1 && "border-amber-300 dark:border-amber-700"
                )}
              >
                <div className="flex items-start gap-2">
                  <span className="shrink-0 font-medium text-amber-600 dark:text-amber-400">
                    Q{i + 1}:
                  </span>
                  <span className="text-foreground">{q.question[locale]}</span>
                </div>
              </div>
            ))}
          </div>

          {/* Footer Actions */}
          <div className="flex flex-wrap items-center gap-2">
            {/* Continue Thinking Button */}
            {!allQuestionsRevealed && (
              <Button
                size="sm"
                variant="outline"
                onClick={handleContinueThinking}
                className="gap-1.5"
              >
                {t("continueThinking")}
              </Button>
            )}

            {/* All Questions Revealed Message */}
            {allQuestionsRevealed && (
              <span className="text-xs text-muted-foreground">
                {t("allQuestionsRevealed")}
              </span>
            )}

            {/* View Hint Buttons */}
            {hints && hints.length > 0 && (
              <>
                {hints.map((hint) => (
                  <Button
                    key={hint.level}
                    size="sm"
                    variant="ghost"
                    onClick={() => handleViewHint(hint.level)}
                    className="gap-1.5 text-xs"
                  >
                    {t("viewHint", { level: hint.level })}
                  </Button>
                ))}
              </>
            )}
          </div>
        </div>
      )}
    </div>
  );
}
