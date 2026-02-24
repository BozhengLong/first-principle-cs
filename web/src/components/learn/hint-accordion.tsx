"use client";

import { useState } from "react";
import { Lightbulb } from "lucide-react";
import { useTranslations, useLocale } from "next-intl";
import {
  Accordion,
  AccordionContent,
  AccordionItem,
  AccordionTrigger,
} from "@/components/ui/accordion";
import { MarkdownContent } from "./markdown-content";

interface HintAccordionProps {
  hints: { level: number; content: { zh: string; en: string } }[];
}

export function HintAccordion({ hints }: HintAccordionProps) {
  const t = useTranslations("learn");
  const locale = useLocale() as "zh" | "en";
  const [revealed, setRevealed] = useState<Set<number>>(new Set());

  const handleValueChange = (value: string[]) => {
    const newRevealed = new Set(revealed);
    for (const v of value) {
      const level = parseInt(v.replace("hint-", ""), 10);
      newRevealed.add(level);
    }
    setRevealed(newRevealed);
  };

  return (
    <div className="mt-6">
      <h3 className="mb-3 flex items-center gap-2 text-sm font-medium">
        <Lightbulb className="h-4 w-4" />
        {t("hints")}
      </h3>
      <Accordion type="multiple" onValueChange={handleValueChange}>
        {hints.map((hint) => {
          const isUnlocked =
            hint.level === 1 || revealed.has(hint.level - 1);

          return (
            <AccordionItem
              key={hint.level}
              value={`hint-${hint.level}`}
              disabled={!isUnlocked}
              className={!isUnlocked ? "opacity-50" : ""}
            >
              <AccordionTrigger className="text-sm">
                {t("hintLevel", { level: hint.level })}
                {!isUnlocked && (
                  <span className="ml-2 text-xs text-muted-foreground">
                    ({t("unlockPrevious")})
                  </span>
                )}
              </AccordionTrigger>
              <AccordionContent>
                <MarkdownContent content={hint.content[locale]} />
              </AccordionContent>
            </AccordionItem>
          );
        })}
      </Accordion>
    </div>
  );
}
