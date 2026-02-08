"use client";

import { BarChart3 } from "lucide-react";
import { useTranslations } from "next-intl";

export function VizPanel() {
  const t = useTranslations("common");

  return (
    <div className="flex h-full flex-col items-center justify-center rounded-lg border border-dashed bg-muted/30 p-6">
      <BarChart3 className="mb-3 h-10 w-10 text-muted-foreground/50" />
      <p className="text-sm text-muted-foreground">{t("vizPlaceholder")}</p>
    </div>
  );
}
