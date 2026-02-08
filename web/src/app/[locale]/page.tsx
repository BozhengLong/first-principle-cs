import { setRequestLocale } from "next-intl/server";
import { useTranslations } from "next-intl";
import { Link } from "@/i18n/navigation";
import { Button } from "@/components/ui/button";
import { projectGroups } from "@/lib/projects";
import { ArrowRight } from "lucide-react";

export default async function HomePage({
  params,
}: {
  params: Promise<{ locale: string }>;
}) {
  const { locale } = await params;
  setRequestLocale(locale);

  return <HomeContent />;
}

function HomeContent() {
  const t = useTranslations();

  return (
    <div className="flex flex-col items-center px-6 py-16 md:py-24">
      <div className="mx-auto max-w-2xl text-center">
        <h1 className="text-3xl font-bold tracking-tight md:text-4xl">
          {t("home.title")}
        </h1>
        <p className="mt-4 text-lg text-muted-foreground">
          {t("home.subtitle")}
        </p>
        <div className="mt-8">
          <Button asChild size="lg">
            <Link href="/learn/tiny-interpreter">
              {t("home.startProject")}
              <ArrowRight className="ml-2 h-4 w-4" />
            </Link>
          </Button>
        </div>
      </div>

      <div className="mt-16 grid w-full max-w-3xl gap-6 md:grid-cols-2">
        {projectGroups.map((group) => (
          <div key={group.key} className="rounded-lg border p-4">
            <h2 className="mb-3 text-sm font-semibold uppercase tracking-wider text-muted-foreground">
              {t(`nav.${group.key}`)}
            </h2>
            <div className="space-y-2">
              {group.projects.map((project) => {
                const Icon = project.icon;
                return (
                  <Link
                    key={project.id}
                    href={`/learn/${project.id}`}
                    className="flex items-center gap-3 rounded-md p-2 transition-colors hover:bg-accent"
                  >
                    <Icon className="h-4 w-4 shrink-0 text-muted-foreground" />
                    <div>
                      <div className="text-sm font-medium">
                        {t(`projects.${project.id}.name`)}
                      </div>
                      <div className="text-xs text-muted-foreground">
                        {t(`projects.${project.id}.description`)}
                      </div>
                    </div>
                  </Link>
                );
              })}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}
