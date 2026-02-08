"use client";

import { Languages } from "lucide-react";
import { useLocale, useTranslations } from "next-intl";
import { usePathname, useRouter } from "@/i18n/navigation";
import { Button } from "@/components/ui/button";

export function LocaleToggle() {
  const locale = useLocale();
  const router = useRouter();
  const pathname = usePathname();
  const t = useTranslations("common");

  const toggleLocale = () => {
    const next = locale === "zh" ? "en" : "zh";
    router.replace(pathname, { locale: next });
  };

  return (
    <Button
      variant="ghost"
      size="icon"
      onClick={toggleLocale}
      title={t("switchLanguage")}
    >
      <Languages className="h-4 w-4" />
      <span className="sr-only">{t("switchLanguage")}</span>
    </Button>
  );
}
