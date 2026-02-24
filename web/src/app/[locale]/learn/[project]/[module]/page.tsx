import { setRequestLocale } from "next-intl/server";
import { notFound } from "next/navigation";
import { projects } from "@/lib/projects";
import { getModule, getModules } from "@/data/tiny-interpreter";
import { WorkspaceLayout } from "@/components/layout/workspace-layout";

const validProjectIds = projects.map((p) => p.id);

export function generateStaticParams() {
  const params: { project: string; module: string }[] = [];
  for (const projectId of validProjectIds) {
    const modules = getModules(projectId);
    for (const mod of modules) {
      params.push({ project: projectId, module: mod.slug });
    }
  }
  return params;
}

export default async function LearnModulePage({
  params,
}: {
  params: Promise<{ locale: string; project: string; module: string }>;
}) {
  const { locale, project, module: moduleSlug } = await params;

  if (!validProjectIds.includes(project)) {
    notFound();
  }

  const mod = getModule(project, moduleSlug);
  if (!mod) {
    notFound();
  }

  setRequestLocale(locale);

  return (
    <div className="h-full">
      <WorkspaceLayout project={project} module={mod} />
    </div>
  );
}
