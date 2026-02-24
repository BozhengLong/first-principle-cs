"use client";

import { ChevronLeft, ChevronRight } from "lucide-react";
import { useTranslations } from "next-intl";
import { Link } from "@/i18n/navigation";
import { Button } from "@/components/ui/button";
import type { LearningModule } from "@/data/types";

interface StepHeaderProps {
  project: string;
  module: LearningModule;
  prev?: LearningModule;
  next?: LearningModule;
  totalModules: number;
}

export function StepHeader({
  project,
  module,
  prev,
  next,
  totalModules,
}: StepHeaderProps) {
  const t = useTranslations("modules");

  return (
    <div className="flex items-center justify-between border-b px-3 py-2">
      <Button
        variant="ghost"
        size="sm"
        asChild
        disabled={!prev}
        className={!prev ? "pointer-events-none opacity-40" : ""}
      >
        <Link href={`/learn/${project}/${prev?.slug ?? ""}`}>
          <ChevronLeft className="h-4 w-4" />
        </Link>
      </Button>

      <span className="text-xs font-medium text-muted-foreground">
        {t(`${project}.module${module.index}`)} ({module.index + 1}/
        {totalModules})
      </span>

      <Button
        variant="ghost"
        size="sm"
        asChild
        disabled={!next}
        className={!next ? "pointer-events-none opacity-40" : ""}
      >
        <Link href={`/learn/${project}/${next?.slug ?? ""}`}>
          <ChevronRight className="h-4 w-4" />
        </Link>
      </Button>
    </div>
  );
}
