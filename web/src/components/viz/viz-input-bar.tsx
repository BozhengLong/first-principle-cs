"use client";

import { Eye, Loader2 } from "lucide-react";
import { useTranslations } from "next-intl";
import { Button } from "@/components/ui/button";
import { useWorkspace } from "@/contexts/workspace-context";

export function VizInputBar() {
  const t = useTranslations("viz");
  const { vizInput, setVizInput, runVisualize, vizLoading } = useWorkspace();

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    runVisualize();
  };

  return (
    <form onSubmit={handleSubmit} className="flex items-center gap-2 border-b px-3 py-2">
      <input
        type="text"
        value={vizInput}
        onChange={(e) => setVizInput(e.target.value)}
        placeholder={t("inputPlaceholder")}
        className="flex-1 rounded-md border bg-background px-2.5 py-1.5 text-xs font-mono focus:outline-none focus:ring-1 focus:ring-ring"
      />
      <Button type="submit" size="sm" variant="secondary" disabled={vizLoading} className="gap-1.5">
        {vizLoading ? (
          <Loader2 className="h-3.5 w-3.5 animate-spin" />
        ) : (
          <Eye className="h-3.5 w-3.5" />
        )}
        {t("visualize")}
      </Button>
    </form>
  );
}
