import { setRequestLocale } from "next-intl/server";
import { notFound } from "next/navigation";
import { projects } from "@/lib/projects";
import { WorkspaceLayout } from "@/components/layout/workspace-layout";

const validProjectIds = projects.map((p) => p.id);

export function generateStaticParams() {
  return validProjectIds.map((project) => ({ project }));
}

export default async function LearnProjectPage({
  params,
}: {
  params: Promise<{ locale: string; project: string }>;
}) {
  const { locale, project } = await params;

  if (!validProjectIds.includes(project)) {
    notFound();
  }

  setRequestLocale(locale);

  return (
    <div className="h-full">
      <WorkspaceLayout />
    </div>
  );
}
